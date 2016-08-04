#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


/**
 * @brief IdleEvent event.
 *
 * It will be emitted by IdleHandle according with its functionalities.
 */
struct IdleEvent: Event<IdleEvent> { };


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
 * **Note**: despite the name, idle handles will emit events on every loop
 * iteration, not when the loop is actually _idle_.
 */
class IdleHandle final: public Handle<IdleHandle, uv_idle_t> {
    static void startCallback(uv_idle_t *handle) {
        IdleHandle &idle = *(static_cast<IdleHandle*>(handle->data));
        idle.publish(IdleEvent{});
    }

    using Handle::Handle;

public:
    /**
     * @brief Creates a new check handle.
     * @param args A pointer to the loop from which the handle generated.
     * @return A pointer to the newly created handle.
     */
    template<typename... Args>
    static std::shared_ptr<IdleHandle> create(Args&&... args) {
        return std::shared_ptr<IdleHandle>{new IdleHandle{std::forward<Args>(args)...}};
    }

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return initialize<uv_idle_t>(&uv_idle_init);
    }

    /**
     * @brief Starts the handle.
     *
     * A IdleEvent event will be emitted once per loop iteration, right before
     * polling the PrepareHandle handles.
     */
    void start() {
        invoke(&uv_idle_start, get<uv_idle_t>(), &startCallback);
    }

    /**
     * @brief Stops the handle.
     */
    void stop() {
        invoke(&uv_idle_stop, get<uv_idle_t>());
    }
};


}
