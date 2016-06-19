#pragma once


#include <uv.h>
#include "resource.hpp"
#include "error.hpp"


namespace uvw {


class Check final: public Resource<Check> {
    static void proto(uv_check_t* h) {
        static_cast<Check*>(h->data)->callback(UVWError{});
    }

public:
    using Callback = std::function<void(UVWError)>;

    explicit Check(uv_loop_t *loop): Resource{&handle} {
        uv_check_init(loop, &handle);
    }

    void start(Callback cb) noexcept {
        callback = cb;
        auto err = uv_check_start(&handle, &proto);

        if(err) {
            callback(UVWError{err});
        }
    }

    bool stop() noexcept { return (uv_check_stop(&handle) == 0); }

private:
    uv_check_t handle;
    Callback callback;
};


}
