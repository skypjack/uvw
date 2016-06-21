#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "resource.hpp"
#include "error.hpp"


namespace uvw {


class Check final: public Resource {
    static void proto(uv_check_t* h) {
        static_cast<Check*>(h->data)->callback(UVWError{});
    }

public:
    using Callback = std::function<void(UVWError)>;

    explicit Check(std::shared_ptr<Loop> ref)
        : Resource{HandleType<uv_check_t>{}, std::move(ref)}
    {
        initialized = (uv_check_init(parent(), get<uv_check_t>()) == 0);
    }

    void start(Callback cb) noexcept {
        callback = cb;
        get<uv_check_t>()->data = this;
        auto err = uv_check_start(get<uv_check_t>(), &proto);

        if(err) {
            callback(UVWError{err});
        }
    }

    UVWError stop() noexcept { return UVWError{uv_check_stop(get<uv_check_t>())}; }

    explicit operator bool() { return initialized; }

private:
    Callback callback;
    bool initialized;
};


}
