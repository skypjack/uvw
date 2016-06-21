#pragma once


#include <cstdint>
#include <chrono>
#include <ratio>
#include <uv.h>
#include "resource.hpp"
#include "error.hpp"


namespace uvw {


class Timer final: public Resource {
    static void proto(uv_timer_t* h) {
        static_cast<Timer*>(h->data)->callback(UVWError{});
    }

public:
    using Time = std::chrono::duration<uint64_t, std::milli>;
    using Callback = std::function<void(UVWError)>;

    explicit Timer(std::shared_ptr<Loop> ref)
        : Resource{HandleType<uv_timer_t>{}, std::move(ref)}
    {
        initialized = (uv_timer_init(parent(), get<uv_timer_t>()) == 0);
    }

    void start(const Time &timeout, const Time &rep, Callback cb) noexcept {
        callback = cb;
        get<uv_timer_t>()->data = this;
        auto err = uv_timer_start(get<uv_timer_t>(), &proto, timeout.count(), rep.count());

        if(err) {
            callback(UVWError{err});
        }
    }

    UVWError stop() noexcept { return UVWError{uv_timer_stop(get<uv_timer_t>())}; }
    UVWError again() noexcept { return UVWError{uv_timer_again(get<uv_timer_t>())}; }
    void repeat(const Time &rep) noexcept { uv_timer_set_repeat(get<uv_timer_t>(), rep.count()); }
    Time repeat() const noexcept { return Time{uv_timer_get_repeat(get<uv_timer_t>())}; }

    explicit operator bool() { return initialized; }

private:
    Callback callback;
    bool initialized;
};


}
