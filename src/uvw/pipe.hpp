#pragma once


#include <type_traits>
#include <utility>
#include <memory>
#include <string>
#include <uv.h>
#include "request.hpp"
#include "stream.hpp"
#include "util.hpp"
#include "loop.hpp"


namespace uvw {


/**
 * @brief The PipeHandle handle.
 *
 * Pipe handles provide an abstraction over local domain sockets on Unix and
 * named pipes on Windows.
 *
 * To create a `PipeHandle` through a `Loop`, arguments follow:
 *
 * * An optional boolean value that indicates if this pipe will be used for
 * handle passing between processes.
 */
class PipeHandle final: public StreamHandle<PipeHandle, uv_pipe_t> {
public:
    explicit PipeHandle(ConstructorAccess ca, std::shared_ptr<Loop> ref, bool pass = false)
        : StreamHandle{ca, std::move(ref)}, ipc{pass}
    {}

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return initialize(&uv_pipe_init, ipc);
    }

    /**
     * @brief Opens an existing file descriptor or HANDLE as a pipe.
     *
     * The passed file descriptor or HANDLE is not checked for its type, but
     * it’s required that it represents a valid pipe.
     *
     * @param file A valid file handle (either a file descriptor or a HANDLE).
     */
    void open(FileHandle file) {
        invoke(&uv_pipe_open, get(), file);
    }

    /**
     * @brief bind Binds the pipe to a file path (Unix) or a name (Windows).
     *
     * Paths on Unix get truncated typically between 92 and 108 bytes.
     *
     * @param name A valid file path.
     */
    void bind(std::string name) {
        invoke(&uv_pipe_bind, get(), name.data());
    }

    /**
     * @brief Connects to the Unix domain socket or the named pipe.
     *
     * Paths on Unix get truncated typically between 92 and 108 bytes.
     *
     * @param name A valid domain socket or named pipe.
     */
    void connect(std::string name) {
        auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
            ptr->publish(event);
        };

        auto connect = loop().resource<details::ConnectReq>();
        connect->once<ErrorEvent>(listener);
        connect->once<ConnectEvent>(listener);
        connect->connect(&uv_pipe_connect, get(), name.data());
    }

    /**
     * @brief Gets the name of the Unix domain socket or the named pipe.
     * @return The name of the Unix domain socket or the named pipe, an empty
     * string in case of errors.
     */
    std::string sock() const noexcept {
        return details::tryRead(&uv_pipe_getsockname, get());
    }

    /**
     * @brief Gets the name of the Unix domain socket or the named pipe to which
     * the handle is connected.
     * @return The name of the Unix domain socket or the named pipe to which
     * the handle is connected, an empty string in case of errors.
     */
    std::string peer() const noexcept {
        return details::tryRead(&uv_pipe_getpeername, get());
    }

    /**
     * @brief Sets the number of pending pipe this instance can handle.
     *
     * This method can be used to set the number of pending pipe this instance
     * handles when the pipe server is waiting for connections.<br/>
     * Note that this setting applies to Windows only.
     *
     * @param count The number of accepted pending pipe.
     */
    void pending(int count) noexcept {
        uv_pipe_pending_instances(get(), count);
    }

    /**
     * @brief Gets the number of pending pipe this instance can handle.
     * @return The number of pending pipe this instance can handle.
     */
    int pending() noexcept {
        return uv_pipe_pending_count(get());
    }

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
     * * `HandleType::PIPE`
     * * `HandleType::TCP`
     * * `HandleType::UDP`
     * * `HandleType::UNKNOWN`
     */
    HandleType receive() noexcept {
        auto type = uv_pipe_pending_type(get());
        return Utilities::guessHandle(type);
    }

private:
    bool ipc;
};


}
