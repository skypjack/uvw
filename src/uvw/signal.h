#ifndef UVW_SIGNAL_INCLUDE_H
#define UVW_SIGNAL_INCLUDE_H

#include <uv.h>
#include "config.h"
#include "handle.hpp"
#include "loop.h"

namespace uvw {

/*! @brief Signal event. */
struct signal_event {
    explicit signal_event(int sig) noexcept;

    int signum; /*!< The signal being monitored by this handle. */
};

/**
 * @brief The signal handle.
 *
 * Signal handles implement Unix style signal handling on a per-event loop
 * bases.<br/>
 * Reception of some signals is emulated on Windows.
 *
 * To create a `signal_handle` through a `loop`, no arguments are required.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/signal.html)
 * for further details.
 */
class signal_handle final: public handle<signal_handle, uv_signal_t, signal_event> {
    static void start_callback(uv_signal_t *hndl, int signum);

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
     * The handle will start emitting signal events when needed.
     *
     * @param signum The signal to be monitored.
     * @return Underlying return value.
     */
    int start(int signum);

    /**
     * @brief Starts the handle.
     *
     * Same functionality as signal_handle::start but the signal handler is
     * reset the moment the signal is received.
     *
     * @param signum The signal to be monitored.
     * @return Underlying return value.
     */
    int one_shot(int signum);

    /**
     * @brief Stops the handle.
     * @return Underlying return value.
     */
    int stop();

    /**
     * @brief Gets the signal being monitored.
     * @return The signal being monitored.
     */
    int signal() const noexcept;
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "signal.cpp"
#endif

#endif // UVW_SIGNAL_INCLUDE_H
