#pragma once


#include <type_traits>
#include <functional>
#include <memory>
#include <uv.h>
#include "loop.hpp"


namespace uvw {


template<typename>
struct HandleType { };


template<typename>
class Resource;


namespace details {


template<typename T>
void* get(T *handle) { return handle->data; }


template<typename T, typename F>
struct UVCallback;


template<typename T,typename H,  typename... Args>
struct UVCallback<T, void(H, Args...)> {
    template<void(*F)(T &, H, Args...)>
    static auto get(T &);

private:
    template<void(*F)(T &, H, Args...)>
    static void proto(H, Args...);
};


}


template<typename T>
class Resource: public std::enable_shared_from_this<T> {
    template<typename, typename>
    friend struct details::UVCallback;

    static void closeCallback(T &ref, uv_handle_t*) {
        Resource<T> &res = ref;
        res.closeCb(UVWError{});
        res.closeCb = nullptr;
    }

protected:
    template<typename F>
    using Callback = details::UVCallback<T, F>;

    template<typename U>
    explicit Resource(HandleType<U>, std::shared_ptr<Loop> r)
        : pLoop{std::move(r)}, handle{std::make_shared<U>()}
    { }

    template<typename U>
    U* get() const noexcept { return reinterpret_cast<U*>(handle.get()); }

    uv_loop_t* parent() const noexcept { return pLoop->loop.get(); }

public:
    explicit Resource(const Resource &) = delete;
    explicit Resource(Resource &&) = delete;

    ~Resource() { static_assert(std::is_base_of<Resource<T>, T>::value, "!"); }

    void operator=(const Resource &) = delete;
    void operator=(Resource &&) = delete;

    std::shared_ptr<Loop> loop() const noexcept { return pLoop; }

    bool active() const noexcept { return !(uv_is_active(get<uv_handle_t>()) == 0); }
    bool closing() const noexcept { return !(uv_is_closing(get<uv_handle_t>()) == 0); }

    void reference() noexcept { uv_ref(get<uv_handle_t>()); }
    void unreference() noexcept { uv_ref(get<uv_handle_t>()); }
    bool referenced() const noexcept { return !(uv_has_ref(get<uv_handle_t>()) == 0); }

    void close(std::function<void(UVWError)> cb) noexcept {
        auto func = Callback<void(uv_handle_t*)>::template get<&Resource<T>::closeCallback>(*static_cast<T*>(this));
        closeCb = std::move(cb);
        uv_close(get<uv_handle_t>(), func);
    }

private:
    std::function<void(UVWError)> closeCb;
    std::shared_ptr<Loop> pLoop;
    std::shared_ptr<void> handle;
    std::shared_ptr<void> leak;
};


namespace details {


template<typename T, typename H, typename... Args>
template<void(*F)(T &, H, Args...)>
auto UVCallback<T, void(H, Args...)>::get(T &ref) {
    Resource<T> &res = ref;
    res.leak = res.shared_from_this();
    res.template get<uv_handle_t>()->data = static_cast<void*>(&ref);
    return &UVCallback<T, void(H, Args...)>::proto<F>;
}

template<typename T, typename H, typename... Args>
template<void(*F)(T &, H, Args...)>
void UVCallback<T, void(H, Args...)>::proto(H handle, Args... args) {
    T &ref = *(static_cast<T*>(details::get(handle)));
    auto ptr = ref.leak;
    ref.leak.reset();
    F(ref, handle, std::forward<Args>(args)...);
    (void)ptr;
}


}


}
