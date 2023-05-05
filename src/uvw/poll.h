#ifndef UVW_POLL_INCLUDE_H
#define UVW_POLL_INCLUDE_H

#include <memory>
#include <type_traits>
#include <uv.h>
#include "config.h"
#include "enum.hpp"
#include "handle.hpp"
#include "util.h"

namespace uvw {

namespace details {

enum class uvw_poll_event : std::underlying_type_t<uv_poll_event> {
    READABLE = UV_READABLE,
    WRITABLE = UV_WRITABLE,
    DISCONNECT = UV_DISCONNECT,
    PRIORITIZED = UV_PRIORITIZED,
    _UVW_ENUM = 0
};

}

/*! @brief Poll event. */
struct poll_event {
    explicit poll_event(details::uvw_poll_event events) noexcept;

    /**
     * @brief Detected events all in one.
     *
     * Available flags are:
     *
     * * `poll_handle::event::READABLE`
     * * `poll_handle::event::WRITABLE`
     * * `poll_handle::event::DISCONNECT`
     * * `poll_handle::event::PRIORITIZED`
     */
    details::uvw_poll_event flags;
};

/**
 * @brief The poll handle.
 *
 * Poll handles are used to watch file descriptors for readability, writability
 * and disconnection.
 *
 * To create a `poll_handle` through a `loop`, arguments follow:
 *
 * * A descriptor that can be:
 *     * either an `int` file descriptor
 *     * or a `os_socket_handle` socket descriptor
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/poll.html)
 * for further details.
 */
class poll_handle final: public handle<poll_handle, uv_poll_t, poll_event> {
    static void start_callback(uv_poll_t *hndl, int status, int events);

public:
    using poll_event_flags = details::uvw_poll_event;

    explicit poll_handle(loop::token token, std::shared_ptr<loop> ref, int desc);
    explicit poll_handle(loop::token token, std::shared_ptr<loop> ref, os_socket_handle sock);

    /**
     * @brief Initializes the handle.
     * @return Underlying return value.
     */
    int init() final;

    /**
     * @brief Starts polling the file descriptor.
     *
     * Available flags are:
     *
     * * `poll_handle::event::READABLE`
     * * `poll_handle::event::WRITABLE`
     * * `poll_handle::event::DISCONNECT`
     * * `poll_handle::event::PRIORITIZED`
     *
     * As soon as an event is detected, a poll event is emitted by the
     * handle.
     *
     * Calling more than once this method will update the flags to which the
     * caller is interested.
     *
     * @param flags The events to which the caller is interested.
     * @return Underlying return value.
     */
    int start(poll_event_flags flags);

    /**
     * @brief Stops polling the file descriptor.
     * @return Underlying return value.
     */
    int stop();

private:
    enum {
        FD,
        SOCKET
    } tag;

    union {
        int file_desc;
        os_socket_handle socket;
    };
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "poll.cpp"
#endif

#endif // UVW_POLL_INCLUDE_H
