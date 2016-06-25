#pragma once


#include <cstdint>
#include <utility>
#include <chrono>
#include <ratio>
#include <uv.h>
#include "resource.hpp"
#include "error.hpp"


namespace uvw {


class Timer final: public Resource<Timer> {
    static Timer* startCallback(uv_timer_t* h) {
        Timer *timer = static_cast<Timer*>(h->data);
        timer->startCb(UVWError{});
        return timer;
    }

    explicit Timer(std::shared_ptr<Loop> ref)
        : Resource{HandleType<uv_timer_t>{}, std::move(ref)}
    {
        initialized = (uv_timer_init(parent(), get<uv_timer_t>()) == 0);
    }

public:
    using Time = std::chrono::duration<uint64_t, std::milli>;

    template<typename... Args>
    static std::shared_ptr<Timer> create(Args&&... args) {
        return std::shared_ptr<Timer>{new Timer{std::forward<Args>(args)...}};
    }

    void start(const Time &timeout, const Time &rep, std::function<void(UVWError)> cb) noexcept {
        using UVCB = UVCallback<Timer, uv_timer_t*>;
        auto func = UVCB::get<&Timer::startCallback>(this);
        startCb = std::move(cb);
        auto err = uv_timer_start(get<uv_timer_t>(), func, timeout.count(), rep.count());

        if(err) {
            startCb(UVWError{err});
            reset();
        }
    }

    UVWError stop() noexcept { return UVWError{uv_timer_stop(get<uv_timer_t>())}; }
    UVWError again() noexcept { return UVWError{uv_timer_again(get<uv_timer_t>())}; }
    void repeat(const Time &rep) noexcept { uv_timer_set_repeat(get<uv_timer_t>(), rep.count()); }
    Time repeat() const noexcept { return Time{uv_timer_get_repeat(get<uv_timer_t>())}; }

    explicit operator bool() { return initialized; }

private:
    std::function<void(UVWError)> startCb;
    bool initialized;
};


}
