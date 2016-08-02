#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


/**
 * @brief CheckEvent event.
 *
 * It will be emitted by the CheckHandle according with its functionalities.
 */
struct CheckEvent: Event<CheckEvent> { };


/**
 * @brief The CheckHandle handle.
 *
 * Check handles will emit a CheckEvent event once per loop iteration, right
 * after polling for I/O.
 */
class CheckHandle final: public Handle<CheckHandle, uv_check_t> {
    static void startCallback(uv_check_t *handle) {
        CheckHandle &check = *(static_cast<CheckHandle*>(handle->data));
        check.publish(CheckEvent{});
    }

    using Handle::Handle;

public:
    /**
     * @brief Creates a new check handle.
     * @param args A pointer to the loop from which the handle generated.
     * @return A pointer to the newly created handle.
     */
    template<typename... Args>
    static std::shared_ptr<CheckHandle> create(Args&&... args) {
        return std::shared_ptr<CheckHandle>{new CheckHandle{std::forward<Args>(args)...}};
    }

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return initialize<uv_check_t>(&uv_check_init);
    }

    /**
     * @brief Starts the handle.
     *
     * A CheckEvent event will be emitted once per loop iteration, right after
     * polling for I/O.
     */
    void start() {
        invoke(&uv_check_start, get<uv_check_t>(), &startCallback);
    }

    /**
     * @brief Stops the handle.
     */
    void stop() {
        invoke(&uv_check_stop, get<uv_check_t>());
    }
};


}
