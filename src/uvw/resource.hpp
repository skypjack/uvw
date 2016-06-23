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


template<typename>
struct UVCallback;


template<typename... Args>
struct UVCallback<Args...> {
    template<typename T, T*(*F)(Args...)>
    static auto get(T *res);

private:
    template<typename T, T*(*F)(Args...)>
    static void proto(Args... args);
};


template<typename T>
class Resource: public std::enable_shared_from_this<T> {
    template<typename>
    friend struct UVCallback;

    static Resource<T>* closeCallback(uv_handle_t* h) {
        auto ptr = static_cast<Resource<T>*>(h->data);
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
        using UVCB = UVCallback<uv_handle_t*>;
        auto func = UVCB::get<Resource<T>, &closeCallback>(this);
        closeCb = std::move(cb);
        uv_close(get<uv_handle_t>(), func);
    }

private:
    std::function<void(UVWError)> closeCb;
    std::shared_ptr<Loop> pLoop;
    std::shared_ptr<void> handle;
    std::shared_ptr<void> ref;
};


template<typename... Args>
template<typename T, T*(*F)(Args...)>
void UVCallback<Args...>::proto(Args... args) {
    T *res = F(std::forward<Args>(args)...);
    res->ref = nullptr;
}

template<typename... Args>
template<typename T, T*(*F)(Args...)>
auto UVCallback<Args...>::get(T *res) {
    res->template get<uv_handle_t>()->data = res;
    res->ref = res->shared_from_this();
    return &proto<T, F>;
}


}
