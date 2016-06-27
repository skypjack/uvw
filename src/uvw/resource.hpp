#pragma once


#include <type_traits>
#include <functional>
#include <utility>
#include <memory>
#include <uv.h>
#include "loop.hpp"
#include "util.hpp"


namespace uvw {


template<typename>
struct HandleType { };


template<typename>
class Resource;


namespace details {


template<typename T> void* get(T *handle) { return handle->data; }
void* get(uv_connect_t *conn) { return conn->handle->data; }


template<typename T, typename F>
struct UVCallbackFactory;


template<typename T,typename H,  typename... Args>
struct UVCallbackFactory<T, void(H, Args...)> {
    template<void(*F)(T &, std::function<void(UVWError, T &)> &, H, Args...)>
    static auto once(T &, std::function<void(UVWError, T &)>);

    template<void(*F)(T &, std::function<void(UVWError, T &)> &, H, Args...)>
    static auto on(T &, std::function<void(UVWError, T &)>);

private:
    template<void(*F)(T &, std::function<void(UVWError, T &)> &, H, Args...)>
    static void protoOnce(H, Args...);

    template<void(*F)(T &, std::function<void(UVWError, T &)> &, H, Args...)>
    static void protoOn(H, Args...);
};


}


template<typename T>
class Resource: public std::enable_shared_from_this<T> {
    template<typename, typename>
    friend struct details::UVCallbackFactory;

    static void closeCallback(T &t, std::function<void(UVWError, T &)> &cb, uv_handle_t*) {
        cb(UVWError{}, t);
    }

protected:
    template<typename F>
    using CallbackFactory = details::UVCallbackFactory<T, F>;

    template<typename U>
    explicit Resource(HandleType<U>, std::shared_ptr<Loop> r)
        : uvHandle{std::make_shared<U>()}, pLoop{std::move(r)}
    { }

    void error(int error) {
        auto ptr = std::static_pointer_cast<T>(leak);
        auto cb = std::move(callback);
        leak.reset();
        cb(UVWError{error}, *ptr);
    }

    template<typename U>
    U* get() const noexcept { return reinterpret_cast<U*>(uvHandle.get()); }

    uv_loop_t* parent() const noexcept { return pLoop->loop.get(); }

public:
    explicit Resource(const Resource &) = delete;
    explicit Resource(Resource &&) = delete;

    ~Resource() { static_assert(std::is_base_of<Resource<T>, T>::value, "!"); }

    void operator=(const Resource &) = delete;
    void operator=(Resource &&) = delete;

    Handle<T> handle() noexcept { return pLoop->handle(Resource<T>::shared_from_this()); }
    Loop& loop() const noexcept { return *pLoop; }

    bool active() const noexcept { return !(uv_is_active(get<uv_handle_t>()) == 0); }
    bool closing() const noexcept { return !(uv_is_closing(get<uv_handle_t>()) == 0); }

    void reference() noexcept { uv_ref(get<uv_handle_t>()); }
    void unreference() noexcept { uv_ref(get<uv_handle_t>()); }
    bool referenced() const noexcept { return !(uv_has_ref(get<uv_handle_t>()) == 0); }

    void close(std::function<void(UVWError, T &)> cb) noexcept {
        using CBF = CallbackFactory<void(uv_handle_t*)>;
        auto func = CBF::template once<&Resource<T>::closeCallback>(*static_cast<T*>(this), std::move(cb));
        uv_close(get<uv_handle_t>(), func);
    }

private:
    std::shared_ptr<void> uvHandle;
    std::shared_ptr<Loop> pLoop;
    std::shared_ptr<void> leak;
    std::function<void(UVWError, T &)> callback;
};


namespace details {


template<typename T, typename H, typename... Args>
template<void(*F)(T &, std::function<void(UVWError, T &)> &, H, Args...)>
auto UVCallbackFactory<T, void(H, Args...)>::once(T &ref, std::function<void(UVWError, T &)> cb) {
    Resource<T> &res = ref;
    res.callback = std::move(cb);
    res.leak = res.shared_from_this();
    res.template get<uv_handle_t>()->data = &ref;
    return &UVCallbackFactory<T, void(H, Args...)>::protoOnce<F>;
}

template<typename T, typename H, typename... Args>
template<void(*F)(T &, std::function<void(UVWError, T &)> &, H, Args...)>
auto UVCallbackFactory<T, void(H, Args...)>::on(T &ref, std::function<void(UVWError, T &)> cb) {
    Resource<T> &res = ref;
    res.callback = std::move(cb);
    res.leak = res.shared_from_this();
    res.template get<uv_handle_t>()->data = &ref;
    return &UVCallbackFactory<T, void(H, Args...)>::protoOn<F>;
}

template<typename T, typename H, typename... Args>
template<void(*F)(T &, std::function<void(UVWError, T &)> &, H, Args...)>
void UVCallbackFactory<T, void(H, Args...)>::protoOnce(H handle, Args... args) {
    T &ref = *(static_cast<T*>(details::get(handle)));
    auto ptr = std::static_pointer_cast<T>(ref.leak);
    auto cb = std::move(ref.callback);
    ref.leak.reset();
    F(*ptr, cb, handle, std::forward<Args>(args)...);
}

template<typename T, typename H, typename... Args>
template<void(*F)(T &, std::function<void(UVWError, T &)> &, H, Args...)>
void UVCallbackFactory<T, void(H, Args...)>::protoOn(H handle, Args... args) {
    T &ref = *(static_cast<T*>(details::get(handle)));
    F(ref, ref.callback, handle, std::forward<Args>(args)...);
}


}


}
