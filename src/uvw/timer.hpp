#pragma once


#include <utility>
#include <memory>
#include <chrono>
#include <uv.h>
#include "handle.hpp"
#include "loop.hpp"


namespace uvw {


/**
 * @brief TimerEvent event.
 *
 * It will be emitted by TimerHandle according with its functionalities.
 */
struct TimerEvent {};


/**
 * @brief The TimerHandle handle.
 *
 * Timer handles are used to schedule events to be emitted in the future.
 *
 * To create a `TimerHandle` through a `Loop`, no arguments are required.
 */
class TimerHandle final: public Handle<TimerHandle, uv_timer_t> {
    static void startCallback(uv_timer_t *handle) {
        TimerHandle &timer = *(static_cast<TimerHandle*>(handle->data));
        timer.publish(TimerEvent{});
    }

public:
    using Time = std::chrono::duration<uint64_t, std::milli>;

    using Handle::Handle;

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return initialize(&uv_timer_init);
    }

    /**
     * @brief Starts the timer.
     *
     * If timeout is zero, a TimerEvent event is emitted on the next event loop
     * iteration. If repeat is non-zero, a TimerEvent event is emitted first
     * after timeout milliseconds and then repeatedly after repeat milliseconds.
     *
     * @param timeout Milliseconds before to emit an event (use
     * `std::chrono::duration<uint64_t, std::milli>`).
     * @param repeat Milliseconds between successive events (use
     * `std::chrono::duration<uint64_t, std::milli>`).
     */
    void start(Time timeout, Time repeat) {
        invoke(&uv_timer_start, get(), &startCallback, timeout.count(), repeat.count());
    }

    /**
     * @brief Stops the handle.
     */
    void stop() {
        invoke(&uv_timer_stop, get());
    }

    /**
     * @brief Stops the timer and restarts it if it was repeating.
     *
     * Stop the timer, and if it is repeating restart it using the repeat value
     * as the timeout.<br/>
     * If the timer has never been started before it emits an ErrorEvent event.
     */
    void again() {
        invoke(&uv_timer_again, get());
    }

    /**
     * @brief Sets the repeat interval value.
     *
     * The timer will be scheduled to run on the given interval and will follow
     * normal timer semantics in the case of a time-slice overrun.<br/>
     * For example, if a 50ms repeating timer first runs for 17ms, it will be
     * scheduled to run again 33ms later. If other tasks consume more than the
     * 33ms following the first timer event, then another event will be emitted
     * as soon as possible.
     *
     *  If the repeat value is set from a listener bound to an event, it does
     * not immediately take effect. If the timer was non-repeating before, it
     * will have been stopped. If it was repeating, then the old repeat value
     * will have been used to schedule the next timeout.
     *
     * @param repeat Repeat interval in milliseconds (use
     * `std::chrono::duration<uint64_t, std::milli>`).
     */
    void repeat(Time repeat) {
        uv_timer_set_repeat(get(), repeat.count());
    }

    /**
     * @brief Gets the timer repeat value.
     * @return Timer repeat value in milliseconds (as a
     * `std::chrono::duration<uint64_t, std::milli>`).
     */
    Time repeat() {
        return Time{uv_timer_get_repeat(get())};
    }
};


}
