#pragma once


#include <type_traits>
#include <functional>
#include <memory>
#include <uv.h>
#include "loop.hpp"


namespace uvw {


template<typename>
struct HandleType { };


class BaseResource {
protected:
    template<typename T>
    explicit BaseResource(HandleType<T>, std::shared_ptr<Loop> r)
        : pLoop{std::move(r)}, handle{std::make_shared<T>()}
    { }

    template<typename T>
    T* get() const noexcept { return reinterpret_cast<T*>(handle.get()); }

    uv_loop_t* parent() const noexcept { return pLoop->loop.get(); }

public:
    explicit BaseResource(const BaseResource &) = delete;
    explicit BaseResource(BaseResource &&) = delete;

    void operator=(const BaseResource &) = delete;
    void operator=(BaseResource &&) = delete;

    std::shared_ptr<Loop> loop() const noexcept { return pLoop; }

    bool active() const noexcept { return !(uv_is_active(get<uv_handle_t>()) == 0); }
    bool closing() const noexcept { return !(uv_is_closing(get<uv_handle_t>()) == 0); }

    void reference() noexcept { uv_ref(get<uv_handle_t>()); }
    void unreference() noexcept { uv_ref(get<uv_handle_t>()); }
    bool referenced() const noexcept { return !(uv_has_ref(get<uv_handle_t>()) == 0); }

private:
    std::shared_ptr<Loop> pLoop;
    std::shared_ptr<void> handle;
};


template<typename T>
class Resource: public std::enable_shared_from_this<T>, public BaseResource {
    static void proto(uv_handle_t* h) {
        static_cast<Resource<T>*>(h->data)->callback(UVWError{});
    }

protected:
    using BaseResource::BaseResource;

public:
    using Callback = std::function<void(UVWError)>;

    void close(Callback cb) noexcept {
        callback = std::move(cb);
        get<uv_handle_t>()->data = this;
        uv_close(get<uv_handle_t>(), &proto);
    }

private:
    Callback callback;
};


}
