#pragma once


#include <type_traits>
#include <functional>
#include <memory>
#include <uv.h>
#include "loop.hpp"


namespace uvw {


template<typename T>
class Resource {
protected:
    template<typename U>
    explicit Resource(U *u): handle{reinterpret_cast<uv_handle_t*>(u)} {
        handle->data = this;
    }

    static void proto(uv_handle_t* h) {
        static_cast<Resource*>(h->data)->callback(UVWError{});
    }

public:
    using Callback = std::function<void(UVWError)>;

    virtual ~Resource() { static_assert(std::is_base_of<Resource<T>, T>::value, "!"); }

    Resource(const Resource &) = delete;
    Resource(Resource &&) = delete;

    void operator=(const Resource &) = delete;
    void operator=(Resource &&) = delete;

    bool active() const noexcept { return !(uv_is_active(handle) == 0); }
    bool closing() const noexcept { return !(uv_is_closing(handle) == 0); }

    void close(Callback cb) noexcept {
        callback = cb;
        uv_close(handle, &proto);
    }

    void reference() noexcept { uv_ref(handle); }
    void unreference() noexcept { uv_ref(handle); }
    bool referenced() const noexcept { return !(uv_has_ref(handle) == 0); }

private:
    uv_handle_t *handle;
    Callback callback;
};


}
