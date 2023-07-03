#ifndef UVW_PIPE_INCLUDE_H
#define UVW_PIPE_INCLUDE_H

#include <memory>
#include <string>
#include <type_traits>
#include <uv.h>
#include "config.h"
#include "enum.hpp"
#include "loop.h"
#include "request.hpp"
#include "stream.h"
#include "util.h"

namespace uvw {

namespace details {

enum class uvw_chmod_flags : std::underlying_type_t<uv_poll_event> {
    READABLE = UV_READABLE,
    WRITABLE = UV_WRITABLE,
    _UVW_ENUM = 0
};

}

/**
 * @brief The pipe handle.
 *
 * Pipe handles provide an abstraction over local domain sockets on Unix and
 * named pipes on Windows.
 *
 * To create a `pipe_handle` through a `loop`, arguments follow:
 *
 * * An optional boolean value that indicates if this pipe will be used for
 * handle passing between processes.
 */
class pipe_handle final: public stream_handle<pipe_handle, uv_pipe_t> {
public:
    using chmod_flags = details::uvw_chmod_flags;

    explicit pipe_handle(loop::token token, std::shared_ptr<loop> ref, bool pass = false);

    /**
     * @brief Initializes the handle.
     * @return Underlying return value.
     */
    int init() final;

    /**
     * @brief Opens an existing file descriptor or HANDLE as a pipe.
     *
     * The passed file descriptor or HANDLE is not checked for its type, but
     * it’s required that it represents a valid pipe.
     *
     * @param file A valid file handle (either a file descriptor or a HANDLE).
     * @return Underlying return value.
     */
    int open(file_handle file);

    /**
     * @brief bind Binds the pipe to a file path (Unix) or a name (Windows).
     *
     * Paths on Unix get truncated typically between 92 and 108 bytes.
     *
     * @param name A valid file path.
     * @param no_truncate Force an error rather than allow truncating a path.
     * @return Underlying return value.
     */
    int bind(const std::string &name, const bool no_truncate = false);

    /**
     * @brief Connects to the Unix domain socket or the named pipe.
     *
     * Paths on Unix get truncated typically between 92 and 108 bytes.<br/>
     * A connect event is emitted when the connection has been
     * established.
     *
     * @param name A valid domain socket or named pipe.
     * @param no_truncate Force an error rather than allow truncating a path.
     * @return Underlying return value.
     */
    int connect(const std::string &name, const bool no_truncate = false);

    /**
     * @brief Gets the name of the Unix domain socket or the named pipe.
     * @return The name of the Unix domain socket or the named pipe, an empty
     * string in case of errors.
     */
    std::string sock() const noexcept;

    /**
     * @brief Gets the name of the Unix domain socket or the named pipe to which
     * the handle is connected.
     * @return The name of the Unix domain socket or the named pipe to which
     * the handle is connected, an empty string in case of errors.
     */
    std::string peer() const noexcept;

    /**
     * @brief Sets the number of pending pipe this instance can handle.
     *
     * This method can be used to set the number of pending pipe this instance
     * handles when the pipe server is waiting for connections.<br/>
     * Note that this setting applies to Windows only.
     *
     * @param count The number of accepted pending pipe.
     */
    void pending(int count) noexcept;

    /**
     * @brief Gets the number of pending pipe this instance can handle.
     * @return The number of pending pipe this instance can handle.
     */
    int pending() noexcept;

    /**
     * @brief Used to receive handles over IPC pipes.
     *
     * Steps to be done:
     *
     * * Call `pending()`, if it’s greater than zero then proceed.
     * * Initialize a handle of the given type, as returned by `receive()`.
     * * Call `accept(pipe, handle)`.
     *
     * @return The type of the pending handle. Possible values are:
     *
     * * `handle_type::PIPE`
     * * `handle_type::TCP`
     * * `handle_type::UDP`
     * * `handle_type::UNKNOWN`
     */
    handle_type receive() noexcept;

    /**
     * @brief Alters pipe permissions.
     *
     * It allows the pipe to be accessed from processes run by different users.
     *
     * Available flags are:
     *
     * * `pipe_handle::chmod_flags::READABLE`
     * * `pipe_handle::chmod_flags::WRITABLE`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/pipe.html#c.uv_pipe_chmod)
     * for further details.
     *
     * @param flags A valid set of flags.
     * @return Underlying return value.
     */
    int chmod(chmod_flags flags) noexcept;

private:
    bool ipc;
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "pipe.cpp"
#endif

#endif // UVW_PIPE_INCLUDE_H
