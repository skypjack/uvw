#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "handle.hpp"
#include "loop.hpp"


namespace uvw {


/**
 * @brief IdleEvent event.
 *
 * It will be emitted by IdleHandle according with its functionalities.
 */
struct IdleEvent {};


/**
 * @brief The IdleHandle handle.
 *
 * Idle handles will emit a IdleEvent event once per loop iteration, right
 * before the PrepareHandle handles.
 *
 * The notable difference with prepare handles is that when there are active
 * idle handles, the loop will perform a zero timeout poll instead of blocking
 * for I/O.
 *
 * @note
 * Despite the name, idle handles will emit events on every loop iteration, not
 * when the loop is actually _idle_.
 *
 * To create an `IdleHandle` through a `Loop`, no arguments are required.
 */
class IdleHandle final: public Handle<IdleHandle, uv_idle_t> {
    static void startCallback(uv_idle_t *handle) {
        IdleHandle &idle = *(static_cast<IdleHandle*>(handle->data));
        idle.publish(IdleEvent{});
    }

public:
    using Handle::Handle;

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return initialize(&uv_idle_init);
    }

    /**
     * @brief Starts the handle.
     *
     * A IdleEvent event will be emitted once per loop iteration, right before
     * polling the PrepareHandle handles.
     */
    void start() {
        invoke(&uv_idle_start, get(), &startCallback);
    }

    /**
     * @brief Stops the handle.
     */
    void stop() {
        invoke(&uv_idle_stop, get());
    }
};


}
