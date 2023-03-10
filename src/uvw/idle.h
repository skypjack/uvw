#ifndef UVW_IDLE_INCLUDE_H
#define UVW_IDLE_INCLUDE_H

#include <uv.h>
#include "handle.hpp"
#include "loop.h"

namespace uvw {

/*! @brief Idle event. */
struct idle_event {};

/**
 * @brief The idle handle.
 *
 * Idle handles will emit a idle event once per loop iteration, right before the
 * prepare handles.
 *
 * The notable difference with prepare handles is that when there are active
 * idle handles, the loop will perform a zero timeout poll instead of blocking
 * for I/O.
 *
 * @note
 * Despite the name, idle handles will emit events on every loop iteration, not
 * when the loop is actually _idle_.
 *
 * To create an `idle_handle` through a `loop`, no arguments are required.
 */
class idle_handle final: public handle<idle_handle, uv_idle_t, idle_event> {
    static void start_callback(uv_idle_t *hndl);

public:
    using handle::handle;

    /**
     * @brief Initializes the handle.
     * @return Underlying return value.
     */
    int init() final;

    /**
     * @brief Starts the handle.
     *
     * An idle event will be emitted once per loop iteration, right before
     * polling the prepare handles.
     *
     * @return Underlying return value.
     */
    int start();

    /**
     * @brief Stops the handle.
     *
     * @return Underlying return value.
     */
    int stop();
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "idle.cpp"
#endif

#endif // UVW_IDLE_INCLUDE_H
