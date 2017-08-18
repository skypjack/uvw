#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "handle.hpp"
#include "loop.hpp"


namespace uvw {


/**
 * @brief PrepareEvent event.
 *
 * It will be emitted by PrepareHandle according with its functionalities.
 */
struct PrepareEvent {};


/**
 * @brief The PrepareHandle handle.
 *
 * Prepare handles will emit a PrepareEvent event once per loop iteration, right
 * before polling for I/O.
 *
 * To create a `PrepareHandle` through a `Loop`, no arguments are required.
 */
class PrepareHandle final: public Handle<PrepareHandle, uv_prepare_t> {
    static void startCallback(uv_prepare_t *handle) {
        PrepareHandle &prepare = *(static_cast<PrepareHandle*>(handle->data));
        prepare.publish(PrepareEvent{});
    }

public:
    using Handle::Handle;

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return initialize(&uv_prepare_init);
    }

    /**
     * @brief Starts the handle.
     *
     * A PrepareEvent event will be emitted once per loop iteration, right
     * before polling for I/O.
     *
     * The handle will start emitting PrepareEvent when needed.
     */
    void start() {
        invoke(&uv_prepare_start, get(), &startCallback);
    }

    /**
     * @brief Stops the handle.
     */
    void stop() {
        invoke(&uv_prepare_stop, get());
    }
};


}
