#ifndef UVW_HANDLE_INCLUDE_H
#define UVW_HANDLE_INCLUDE_H

#include <cstddef>
#include <memory>
#include <utility>
#include <uv.h>
#include "config.h"
#include "resource.hpp"
#include "util.h"

namespace uvw {

/*! @brief Close event. */
struct close_event {};

/**
 * @brief Handle base class.
 *
 * Base type for all `uvw` handle types.
 */
template<typename T, typename U, typename... E>
class handle: public resource<T, U, close_event, E...> {
protected:
    static void close_callback(uv_handle_t *hndl) {
        handle<T, U, E...> &ref = *(static_cast<T *>(hndl->data));
        [[maybe_unused]] auto ptr = ref.shared_from_this();
        ref.self_reset();
        ref.publish(close_event{});
    }

    uv_handle_t *as_uv_handle() {
        return reinterpret_cast<uv_handle_t *>(this->raw());
    }

    const uv_handle_t *as_uv_handle() const {
        return reinterpret_cast<const uv_handle_t *>(this->raw());
    }

public:
    using resource<T, U, close_event, E...>::resource;

    /**
     * @brief Gets the category of the handle.
     *
     * A base handle offers no functionality to promote it to the actual handle
     * type. By means of this function, an opaque value that identifies the
     * category of the handle is made available to the users.
     *
     * @return The actual category of the handle.
     */
    handle_category category() const noexcept {
        return handle_category{as_uv_handle()->type};
    }

    /**
     * @brief Gets the type of the handle.
     *
     * A base handle offers no functionality to promote it to the actual handle
     * type. By means of this function, the type of the underlying handle as
     * specified by handle_type is made available to the users.
     *
     * @return The actual type of the handle.
     */
    handle_type type() const noexcept {
        return utilities::guess_handle(category());
    }

    /**
     * @brief Checks if the handle is active.
     *
     * What _active_ means depends on the type of handle:
     *
     * * An async_handle handle is always active and cannot be deactivated,
     * except by closing it with uv_close().
     * * A pipe, tcp, udp, etc. handle - basically any handle that deals with
     * I/O - is active when it is doing something that involves I/O, like
     * reading, writing, connecting, accepting new connections, etc.
     * * A check, idle, timer, etc. handle is active when it has been started
     * with a call to `start()`.
     *
     * Rule of thumb: if a handle of type `foo_handle` has a `start()` member
     * method, then it’s active from the moment that method is called. Likewise,
     * `stop()` deactivates the handle again.
     *
     * @return True if the handle is active, false otherwise.
     */
    bool active() const noexcept {
        return !!uv_is_active(as_uv_handle());
    }

    /**
     * @brief Checks if a handle is closing or closed.
     *
     * This function should only be used between the initialization of the
     * handle and the arrival of the close callback.
     *
     * @return True if the handle is closing or closed, false otherwise.
     */
    bool closing() const noexcept {
        return !!uv_is_closing(as_uv_handle());
    }

    /**
     * @brief Request handle to be closed.
     *
     * This **must** be called on each handle before memory is released.<br/>
     * In-progress requests are cancelled and this can result in errors.
     *
     * The handle will emit a close event when finished.
     */
    void close() noexcept {
        if(!closing()) {
            uv_close(as_uv_handle(), &handle<T, U, E...>::close_callback);
        }
    }

    /**
     * @brief Reference the given handle.
     *
     * References are idempotent, that is, if a handle is already referenced
     * calling this function again will have no effect.
     */
    void reference() noexcept {
        uv_ref(as_uv_handle());
    }

    /**
     * @brief Unreference the given handle.
     *
     * References are idempotent, that is, if a handle is not referenced calling
     * this function again will have no effect.
     */
    void unreference() noexcept {
        uv_unref(as_uv_handle());
    }

    /**
     * @brief Checks if the given handle referenced.
     * @return True if the handle referenced, false otherwise.
     */
    bool referenced() const noexcept {
        return !!uv_has_ref(as_uv_handle());
    }

    /**
     * @brief Returns the size of the underlying handle type.
     * @return The size of the underlying handle type.
     */
    std::size_t size() const noexcept {
        return uv_handle_size(as_uv_handle()->type);
    }

    /**
     * @brief Gets the size of the send buffer used for the socket.
     *
     * Gets the size of the send buffer that the operating system uses for the
     * socket.<br/>
     * This function works for tcp, pipeand udp handles on Unix and for tcp and
     * udp handles on Windows.<br/>
     * Note that Linux will return double the size of the original set value.
     *
     * @return The size of the send buffer, the underlying return value in case
     * of errors.
     */
    int send_buffer_size() {
        int value = 0;
        auto err = uv_send_buffer_size(as_uv_handle(), &value);
        return err ? err : value;
    }

    /**
     * @brief Sets the size of the send buffer used for the socket.
     *
     * Sets the size of the send buffer that the operating system uses for the
     * socket.<br/>
     * This function works for tcp, pipe and udp handles on Unix and for tcp and
     * udp handles on Windows.<br/>
     * Note that Linux will set double the size.
     *
     * @return Underlying return value.
     */
    int send_buffer_size(int value) {
        return uv_send_buffer_size(as_uv_handle(), &value);
    }

    /**
     * @brief Gets the size of the receive buffer used for the socket.
     *
     * Gets the size of the receive buffer that the operating system uses for
     * the socket.<br/>
     * This function works for tcp, pipe and udp handles on Unix and for tcp and
     * udp handles on Windows.<br/>
     * Note that Linux will return double the size of the original set value.
     *
     * @return The size of the receive buffer, the underlying return value in
     * case of errors.
     */
    int recv_buffer_size() {
        int value = 0;
        auto err = uv_recv_buffer_size(as_uv_handle(), &value);
        return err ? err : value;
    }

    /**
     * @brief Sets the size of the receive buffer used for the socket.
     *
     * Sets the size of the receive buffer that the operating system uses for
     * the socket.<br/>
     * This function works for tcp, pipe and udp handles on Unix and for tcp and
     * udp handles on Windows.<br/>
     * Note that Linux will set double the size.
     *
     * @return Underlying return value.
     */
    int recv_buffer_size(int value) {
        return uv_recv_buffer_size(as_uv_handle(), &value);
    }

    /**
     * @brief Gets the platform dependent file descriptor equivalent.
     *
     * Supported handles:
     *
     * * tcp_handle
     * * pipe_handle
     * * tty_handle
     * * udp_handle
     * * poll_handle
     *
     * If invoked on a different handle, one that doesn’t have an attached file
     * descriptor yet or one which was closed, an invalid value is returned.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/handle.html#c.uv_fileno)
     * for further details.
     *
     * @return The file descriptor attached to the hande or a negative value in
     * case of errors.
     */
    os_file_descriptor fd() const {
        uv_os_fd_t fd;
        uv_fileno(as_uv_handle(), &fd);
        return fd;
    }
};

} // namespace uvw

#endif // UVW_HANDLE_INCLUDE_H
