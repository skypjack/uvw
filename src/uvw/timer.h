#ifndef UVW_TIMER_INCLUDE_H
#define UVW_TIMER_INCLUDE_H

#include <cstdint>
#include <chrono>
#include <uv.h>
#include "handle.hpp"
#include "loop.h"

namespace uvw {

/*! @brief Timer event. */
struct timer_event {};

/**
 * @brief The timer handle.
 *
 * Timer handles are used to schedule events to be emitted in the future.
 *
 * To create a `timer_handle` through a `loop`, no arguments are required.
 */
class timer_handle final: public handle<timer_handle, uv_timer_t, timer_event> {
    static void start_callback(uv_timer_t *hndl);

public:
    using time = std::chrono::duration<uint64_t, std::milli>;

    using handle::handle;

    /**
     * @brief Initializes the handle.
     * @return Underlying return value.
     */
    int init() final;

    /**
     * @brief Starts the timer.
     *
     * If timeout is zero, a timer event is emitted on the next event loop
     * iteration. If repeat is non-zero, a timer event is emitted first after
     * timeout milliseconds and then repeatedly after repeat milliseconds.
     *
     * @param timeout Milliseconds before to emit an event (use
     * `std::chrono::duration<uint64_t, std::milli>`).
     * @param repeat Milliseconds between successive events (use
     * `std::chrono::duration<uint64_t, std::milli>`).
     *
     * @return Underlying return value.
     */
    int start(time timeout, time repeat);

    /**
     * @brief Stops the handle.
     * @return Underlying return value.
     */
    int stop();

    /**
     * @brief Stops the timer and restarts it if it was repeating.
     *
     * Stop the timer, and if it is repeating restart it using the repeat value
     * as the timeout.
     *
     * @return Underlying return value.
     */
    int again();

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
    void repeat(time repeat);

    /**
     * @brief Gets the timer repeat value.
     * @return Timer repeat value in milliseconds (as a
     * `std::chrono::duration<uint64_t, std::milli>`).
     */
    time repeat();

    /**
     * @brief Gets the timer due value.
     *
     * The time is relative to `loop::now()`.
     *
     * @return The timer due value or 0 if it has expired.
     */
    time due_in();
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "timer.cpp"
#endif

#endif // UVW_TIMER_INCLUDE_H
