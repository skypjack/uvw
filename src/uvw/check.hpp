#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "handle.hpp"
#include "loop.hpp"


namespace uvw {


/**
 * @brief CheckEvent event.
 *
 * It will be emitted by CheckHandle according with its functionalities.
 */
struct CheckEvent {};


/**
 * @brief The CheckHandle handle.
 *
 * Check handles will emit a CheckEvent event once per loop iteration, right
 * after polling for I/O.
 *
 * To create a `CheckHandle` through a `Loop`, no arguments are required.
 */
class CheckHandle final: public Handle<CheckHandle, uv_check_t> {
    static void startCallback(uv_check_t *handle) {
        CheckHandle &check = *(static_cast<CheckHandle*>(handle->data));
        check.publish(CheckEvent{});
    }

public:
    using Handle::Handle;

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return initialize(&uv_check_init);
    }

    /**
     * @brief Starts the handle.
     *
     * A CheckEvent event will be emitted once per loop iteration, right after
     * polling for I/O.
     */
    void start() {
        invoke(&uv_check_start, get(), &startCallback);
    }

    /**
     * @brief Stops the handle.
     */
    void stop() {
        invoke(&uv_check_stop, get());
    }
};


}
