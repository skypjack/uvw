#pragma once


#include <type_traits>
#include <utility>
#include <memory>
#include <uv.h>
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


namespace details {


enum class UVPollEvent: std::underlying_type_t<uv_poll_event> {
    READABLE = UV_READABLE,
    WRITABLE = UV_WRITABLE,
    DISCONNECT = UV_DISCONNECT,
    PRIORITIZED = UV_PRIORITIZED
};


}


/**
 * @brief PollEvent event.
 *
 * It will be emitted by PollHandle according with its functionalities.
 */
struct PollEvent {
    explicit PollEvent(Flags<details::UVPollEvent> events) noexcept
        : flags{std::move(events)}
    {}

    /**
     * @brief Detected events all in one.
     *
     * Available flags are:
     *
     * * `PollHandle::Event::READABLE`
     * * `PollHandle::Event::WRITABLE`
     * * `PollHandle::Event::DISCONNECT`
     * * `PollHandle::Event::PRIORITIZED`
     */
    Flags<details::UVPollEvent> flags;
};


/**
 * @brief The PollHandle handle.
 *
 * Poll handles are used to watch file descriptors for readability, writability
 * and disconnection.
 *
 * To create a `PollHandle` through a `Loop`, arguments follow:
 *
 * * A descriptor that can be:
 *     * either an `int` file descriptor
 *     * or a `OSSocketHandle` socket descriptor
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/poll.html)
 * for further details.
 */
class PollHandle final: public Handle<PollHandle, uv_poll_t> {
    static void startCallback(uv_poll_t *handle, int status, int events) {
        PollHandle &poll = *(static_cast<PollHandle*>(handle->data));
        if(status) { poll.publish(ErrorEvent{status}); }
        else { poll.publish(PollEvent{static_cast<std::underlying_type_t<Event>>(events)}); }
    }

public:
    using Event = details::UVPollEvent;

    explicit PollHandle(ConstructorAccess ca, std::shared_ptr<Loop> ref, int desc)
        : Handle{ca, std::move(ref)}, tag{FD}, fd{desc}
    {}

    explicit PollHandle(ConstructorAccess ca, std::shared_ptr<Loop> ref, OSSocketHandle sock)
        : Handle{ca, std::move(ref)}, tag{SOCKET}, socket{sock}
    {}

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return (tag == SOCKET)
                ? initialize(&uv_poll_init_socket, socket)
                : initialize(&uv_poll_init, fd);
    }

    /**
     * @brief Starts polling the file descriptor.
     *
     * Available flags are:
     *
     * * `PollHandle::Event::READABLE`
     * * `PollHandle::Event::WRITABLE`
     * * `PollHandle::Event::DISCONNECT`
     * * `PollHandle::Event::PRIORITIZED`
     *
     * As soon as an event is detected, a PollEvent is emitted by the
     * handle.<br>
     * It could happen that ErrorEvent events are emitted while running.
     *
     * Calling more than once this method will update the flags to which the
     * caller is interested.
     *
     * @param flags The events to which the caller is interested.
     */
    void start(Flags<Event> flags) {
        invoke(&uv_poll_start, get(), flags, &startCallback);
    }

    /**
     * @brief Starts polling the file descriptor.
     *
     * Available flags are:
     *
     * * `PollHandle::Event::READABLE`
     * * `PollHandle::Event::WRITABLE`
     * * `PollHandle::Event::DISCONNECT`
     * * `PollHandle::Event::PRIORITIZED`
     *
     * As soon as an event is detected, a PollEvent is emitted by the
     * handle.<br>
     * It could happen that ErrorEvent events are emitted while running.
     *
     * Calling more than once this method will update the flags to which the
     * caller is interested.
     *
     * @param event The event to which the caller is interested.
     */
    void start(Event event) {
        start(Flags<Event>{event});
    }

    /**
     * @brief Stops polling the file descriptor.
     */
    void stop() {
        invoke(&uv_poll_stop, get());
    }

private:
    enum { FD, SOCKET } tag;
    union {
        int fd;
        OSSocketHandle::Type socket;
    };
};


}
