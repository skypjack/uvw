#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "loop.hpp"


namespace uvw {


/**
 * @brief PrepareEvent event.
 *
 * It will be emitted by PrepareHandle according with its functionalities.
 */
struct PrepareEvent: Event<PrepareEvent> { };


/**
 * @brief The PrepareHandle handle.
 *
 * Prepare handles will emit a PrepareEvent event once per loop iteration, right
 * before polling for I/O.
 */
class PrepareHandle final: public Handle<PrepareHandle, uv_prepare_t> {
    static void startCallback(uv_prepare_t *handle) {
        PrepareHandle &prepare = *(static_cast<PrepareHandle*>(handle->data));
        prepare.publish(PrepareEvent{});
    }

    using Handle::Handle;

public:
    /**
     * @brief Creates a new check handle.
     * @param loop A pointer to the loop from which the handle generated.
     * @return A pointer to the newly created handle.
     */
    static std::shared_ptr<PrepareHandle> create(std::shared_ptr<Loop> loop) {
        return std::shared_ptr<PrepareHandle>{new PrepareHandle{std::move(loop)}};
    }

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
