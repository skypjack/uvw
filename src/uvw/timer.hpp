#pragma once


#include <utility>
#include <memory>
#include <chrono>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


class Timer final: public Handle<Timer> {
    static void startCallback(uv_timer_t *handle) {
        Timer &timer = *(static_cast<Timer*>(handle->data));
        timer.publish(TimerEvent{});
    }

    explicit Timer(std::shared_ptr<Loop> ref)
        : Handle{ResourceType<uv_timer_t>{}, std::move(ref)}
    { }

public:
    using Time = std::chrono::milliseconds;

    template<typename... Args>
    static std::shared_ptr<Timer> create(Args&&... args) {
        return std::shared_ptr<Timer>{new Timer{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_timer_t>(&uv_timer_init); }

    void start(Time timeout, Time repeat) { invoke(uv_timer_start, get<uv_timer_t>(), &startCallback, timeout.count(), repeat.count()); }
    void stop() { invoke(&uv_timer_stop, get<uv_timer_t>()); }
    void again() { invoke(&uv_timer_again, get<uv_timer_t>()); }

    void repeat(Time repeat) { uv_timer_set_repeat(get<uv_timer_t>(), repeat.count()); }
    Time repeat() { return Time{uv_timer_get_repeat(get<uv_timer_t>())}; }
};


}
