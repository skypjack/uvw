#pragma once


#include <memory>
#include <uv.h>
#include "emitter.hpp"
#include "resource.hpp"
#include "self.hpp"
#include "loop.hpp"


namespace uvw {


namespace details {


template<typename T> void* get(T *handle) { return handle->data; }
void* get(uv_connect_t *conn) { return conn->handle->data; }


template<typename, typename>
struct UVCallbackFactory;

template<typename T, typename H,  typename... Args>
struct UVCallbackFactory<T, void(H, Args...)> {
    template<void(*F)(T &, H, Args...)>
    static auto create(T &) noexcept;

private:
    template<void(*F)(T &, H, Args...)>
    static void proto(H, Args...) noexcept;
};


}


template<typename T>
class Handle: public Emitter<T>, public Self<T>, public ResourceWrapper {
    template<typename, typename>
    friend struct details::UVCallbackFactory;

    static void closeCallback(T &ref, uv_handle_t *) {
        ref.publish(CloseEvent{});
    }

protected:
    template<typename F>
    using CallbackFactory = details::UVCallbackFactory<T, F>;

    template<typename U>
    explicit Handle(ResourceType<U> rt, std::shared_ptr<Loop> ref)
        : Emitter<T>{}, Self<T>{}, ResourceWrapper{std::move(rt)}, pLoop{std::move(ref)}
    { }

    uv_loop_t* parent() const noexcept { return pLoop->loop.get(); }

public:
    virtual ~Handle() {
        static_assert(std::is_base_of<Handle<T>, T>::value, "!");
    }

    Loop& loop() const noexcept { return *pLoop; }

    bool active() const noexcept { return !(uv_is_active(get<uv_handle_t>()) == 0); }
    bool closing() const noexcept { return !(uv_is_closing(get<uv_handle_t>()) == 0); }

    void reference() noexcept { uv_ref(get<uv_handle_t>()); }
    void unreference() noexcept { uv_unref(get<uv_handle_t>()); }
    bool referenced() const noexcept { return !(uv_has_ref(get<uv_handle_t>()) == 0); }

    void close() noexcept {
        auto handle = get<uv_handle_t>();

        if(!uv_is_closing(handle)) {
            using CBF = CallbackFactory<void(uv_handle_t *)>;
            T &ref = *static_cast<T*>(this);
            auto func = CBF::template create<&Handle<T>::closeCallback>(ref);
            uv_close(handle, func);
        }
    }

private:
    std::shared_ptr<Loop> pLoop;
};


namespace details {


template<typename T, typename H,  typename... Args>
template<void(*F)(T &, H, Args...)>
auto UVCallbackFactory<T, void(H, Args...)>::create(T &ref) noexcept {
    Handle<T> &handle = ref;
    handle.leak();
    handle.template get<uv_handle_t>()->data = &ref;
    return &UVCallbackFactory<T, void(H, Args...)>::proto<F>;
}


template<typename T, typename H,  typename... Args>
template<void(*F)(T &, H, Args...)>
void UVCallbackFactory<T, void(H, Args...)>::proto(H handle, Args... args) noexcept {
    T &ref = *(static_cast<T*>(details::get(handle)));

    if(0 == uv_is_active(ref.template get<uv_handle_t>())) {
        auto ptr = ref.shared_from_this();
        ref.reset();
        F(*ptr, handle, args...);
    } else {
        F(ref, handle, args...);
    }
}


}


}
