#include "timer.h"

namespace uvw {

    void TimerHandle::startCallback(uv_timer_t *handle) {
        TimerHandle &timer = *(static_cast<TimerHandle*>(handle->data));
        timer.publish(TimerEvent{});
    }

    bool TimerHandle::init() {
        return initialize(&uv_timer_init);
    }

    void TimerHandle::start(TimerHandle::Time timeout, TimerHandle::Time repeat) {
        invoke(&uv_timer_start, get(), &startCallback, timeout.count(), repeat.count());
    }

    void TimerHandle::stop() {
        invoke(&uv_timer_stop, get());
    }

    void TimerHandle::again() {
        invoke(&uv_timer_again, get());
    }

    void TimerHandle::repeat(TimerHandle::Time repeat) {
        uv_timer_set_repeat(get(), repeat.count());
    }

    TimerHandle::Time TimerHandle::repeat() {
        return Time{uv_timer_get_repeat(get())};
    }
}
