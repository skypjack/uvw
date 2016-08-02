#pragma once


#include <type_traits>
#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


namespace details {


enum class UVPollEvent: std::underlying_type_t<uv_poll_event> {
    READABLE = UV_READABLE,
    WRITABLE = UV_WRITABLE,
    DISCONNECT = UV_DISCONNECT
};


}



/**
 * @brief PollEvent event.
 *
 * It will be emitted by the PollHandle according with its functionalities.
 */
struct PollEvent: Event<PollEvent> {
    explicit PollEvent(Flags<details::UVPollEvent> f) noexcept
        : flgs{std::move(f)}
    { }

    /**
     * @brief Gets the detected events.
     *
     * Available flags are:
     *
     * * PollHandle::Event::READABLE
     * * PollHandle::Event::WRITABLE
     * * PollHandle::Event::DISCONNECT
     *
     * @return Detected events all in one.
     */
    Flags<details::UVPollEvent> flags() const noexcept { return flgs; }

private:
    Flags<details::UVPollEvent> flgs;
};


/**
 * @brief The PollHandle handle.
 *
 * Poll handles are used to watch file descriptors for readability, writability
 * and disconnection.
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

    using Handle::Handle;

public:
    using Event = details::UVPollEvent;

    /**
     * @brief Creates a new poll handle.
     * @param args A pointer to the loop from which the handle generated.
     * @return A pointer to the newly created handle.
     */
    template<typename... Args>
    static std::shared_ptr<PollHandle> create(Args&&... args) {
        return std::shared_ptr<PollHandle>{new PollHandle{std::forward<Args>(args)...}};
    }

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init(int fd) {
        return initialize<uv_poll_t>(&uv_poll_init, fd);
    }

    /**
     * @brief Initializes the handle using a socket descriptor.
     *
     * On Unix this is identical to `init(int)`. On windows it takes a SOCKET
     * handle.
     *
     * @param socket A valid socket descriptor.
     * @return True in case of success, false otherwise.
     */
    bool init(OSSocketHandle socket) {
        return initialize<uv_poll_t>(&uv_poll_init_socket, socket);
    }

    /**
     * @brief Starts polling the file descriptor.
     *
     * Available flags are:
     *
     * * PollHandle::Event::READABLE
     * * PollHandle::Event::WRITABLE
     * * PollHandle::Event::DISCONNECT
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
        invoke(&uv_poll_start, get<uv_poll_t>(), flags, &startCallback);
    }

    /**
     * @brief Starts polling the file descriptor.
     *
     * Available flags are:
     *
     * * PollHandle::Event::READABLE
     * * PollHandle::Event::WRITABLE
     * * PollHandle::Event::DISCONNECT
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
        invoke(&uv_poll_stop, get<uv_poll_t>());
    }
};


}
