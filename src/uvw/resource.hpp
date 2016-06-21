#pragma once


#include <type_traits>
#include <functional>
#include <memory>
#include <uv.h>
#include "loop.hpp"


namespace uvw {


template<typename>
struct HandleType { };


class Resource {
    static void proto(uv_handle_t* h) {
        static_cast<Resource*>(h->data)->callback(UVWError{});
    }

protected:
    template<typename T>
    explicit Resource(HandleType<T>, std::shared_ptr<Loop> r)
        : ref{std::move(r)}, res{std::make_shared<T>()}
    { }

    template<typename T>
    T* get() const noexcept { return reinterpret_cast<T*>(res.get()); }

    uv_loop_t* parent() const noexcept { return ref->loop.get(); }

public:
    using Callback = std::function<void(UVWError)>;

    explicit Resource(const Resource &) = delete;
    explicit Resource(Resource &&) = delete;

    void operator=(const Resource &) = delete;
    void operator=(Resource &&) = delete;

    std::shared_ptr<Loop> loop() const noexcept { return ref; }

    bool active() const noexcept { return !(uv_is_active(get<uv_handle_t>()) == 0); }
    bool closing() const noexcept { return !(uv_is_closing(get<uv_handle_t>()) == 0); }

    void close(Callback cb) noexcept {
        callback = cb;
        get<uv_handle_t>()->data = this;
        uv_close(get<uv_handle_t>(), &proto);
    }

    void reference() noexcept { uv_ref(get<uv_handle_t>()); }
    void unreference() noexcept { uv_ref(get<uv_handle_t>()); }
    bool referenced() const noexcept { return !(uv_has_ref(get<uv_handle_t>()) == 0); }

private:
    std::shared_ptr<Loop> ref;
    std::shared_ptr<void> res;
    Callback callback;
};


}
