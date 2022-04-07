#ifndef UVW_PREPARE_INCLUDE_H
#define UVW_PREPARE_INCLUDE_H

#include <uv.h>
#include "handle.hpp"
#include "loop.h"

namespace uvw {

/*! @brief Prepare event. */
struct prepare_event {};

/**
 * @brief The prepare handle.
 *
 * Prepare handles will emit a prepare event once per loop iteration, right
 * before polling for I/O.
 *
 * To create a `prepare_handle` through a `loop`, no arguments are required.
 */
class prepare_handle final: public handle<prepare_handle, uv_prepare_t, prepare_event> {
    static void start_callback(uv_prepare_t *hndl);

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
     * A prepare event will be emitted once per loop iteration, right before
     * polling for I/O.
     *
     * The handle will start emitting prepare events when needed.
     *
     * @return Underlying return value.
     */
    int start();

    /**
     * @brief Stops the handle.
     * @return Underlying return value.
     */
    int stop();
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "prepare.cpp"
#endif

#endif // UVW_PREPARE_INCLUDE_H
