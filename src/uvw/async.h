#ifndef UVW_ASYNC_INCLUDE_H
#define UVW_ASYNC_INCLUDE_H

#include <uv.h>
#include "handle.hpp"
#include "loop.h"

namespace uvw {

/*! @brief Async event. */
struct async_event {};

/**
 * @brief The async handle.
 *
 * Async handles allow the user to _wakeup_ the event loop and get an event
 * emitted from another thread.
 *
 * To create an `async_handle` through a `loop`, no arguments are required.
 */
class async_handle final: public handle<async_handle, uv_async_t, async_event> {
    static void send_callback(uv_async_t *hndl);

public:
    using handle::handle;

    /**
     * @brief Initializes the handle.
     *
     * Unlike other handle initialization functions, it immediately starts the
     * handle.
     *
     * @return Underlying return value.
     */
    int init() final;

    /**
     * @brief Wakeups the event loop and emits the async event.
     *
     * It’s safe to call this function from any thread.<br/>
     * An async event is emitted on the loop thread.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/async.html#c.uv_async_send)
     * for further details.
     *
     * @return Underlying return value.
     */
    int send();
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "async.cpp"
#endif

#endif // UVW_ASYNC_INCLUDE_H
