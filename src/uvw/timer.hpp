#pragma once


#include <cstdint>
#include <chrono>
#include <ratio>
#include <uv.h>
#include "resource.hpp"
#include "error.hpp"


namespace uvw {


class Timer final: public Resource<Timer> {
    static void proto(uv_timer_t* h) {
        static_cast<Timer*>(h->data)->callback(UVWError{});
    }

public:
    using Time = std::chrono::duration<uint64_t, std::milli>;
    using Callback = std::function<void(UVWError)>;

    explicit Timer(uv_loop_t *loop): Resource{&handle} {
        uv_timer_init(loop, &handle);
    }

    void start(const Time &timeout, const Time &rep, Callback cb) noexcept {
        callback = cb;
        auto err = uv_timer_start(&handle, &proto, timeout.count(), rep.count());

        if(err) {
            callback(UVWError{err});
        }
    }

    void stop() noexcept { uv_timer_stop(&handle); }
    void again() noexcept { uv_timer_again(&handle); }
    void repeat(const Time &rep) noexcept { uv_timer_set_repeat(&handle, rep.count()); }
    Time repeat() const noexcept { return Time{uv_timer_get_repeat(&handle)}; }

private:
    uv_timer_t handle;
    Callback callback;
};


}
