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


template<typename T, typename... Args>
struct UVCallback {
    template<T*(*F)(Args...)>
    static auto get(T *res);

private:
    template<T*(*F)(Args...)>
    static void proto(Args... args);
};


template<typename T>
class Resource: public std::enable_shared_from_this<T> {
    template<typename, typename...>
    friend struct UVCallback;

    static Resource<T>* closeCallback(uv_handle_t* h) {
        auto *ptr = static_cast<Resource<T>*>(h->data);
        ptr->closeCb(UVWError{});
        return ptr;
    }

protected:
    template<typename U>
    explicit Resource(HandleType<U>, std::shared_ptr<Loop> r)
        : pLoop{std::move(r)}, handle{std::make_shared<U>()}
    { }

    template<typename U>
    U* get() const noexcept { return reinterpret_cast<U*>(handle.get()); }

    uv_loop_t* parent() const noexcept { return pLoop->loop.get(); }
    void reset() noexcept { ref = nullptr; }

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
        using UVCB = UVCallback<Resource<T>, uv_handle_t*>;
        auto func = UVCB::template get<&Resource<T>::closeCallback>(this);
        closeCb = std::move(cb);
        uv_close(get<uv_handle_t>(), func);
    }

private:
    std::function<void(UVWError)> closeCb;
    std::shared_ptr<Loop> pLoop;
    std::shared_ptr<void> handle;
    std::shared_ptr<void> ref;
};


template<typename T, typename... Args>
template<T*(*F)(Args...)>
void UVCallback<T, Args...>::proto(Args... args) {
    T *res = F(std::forward<Args>(args)...);
    res->ref = nullptr;
}

template<typename T, typename... Args>
template<T*(*F)(Args...)>
auto UVCallback<T, Args...>::get(T *res) {
    res->template get<uv_handle_t>()->data = res;
    res->ref = res->shared_from_this();
    return &UVCallback<T, Args...>::proto<F>;
}


}
