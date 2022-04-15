#ifndef UVW_TCP_INCLUDE_H
#define UVW_TCP_INCLUDE_H

#include <chrono>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <uv.h>
#include "config.h"
#include "enum.hpp"
#include "request.hpp"
#include "stream.h"
#include "util.h"

namespace uvw {

namespace details {

enum class uvw_tcp_flags : std::underlying_type_t<uv_tcp_flags> {
    IPV6ONLY = UV_TCP_IPV6ONLY,
    _UVW_ENUM = 0
};

}

/**
 * @brief The TCP handle.
 *
 * TCP handles are used to represent both TCP streams and servers.<br/>
 * By default, _ipv4_ is used as a template parameter. The handle already
 * supports _IPv6_ out-of-the-box by using `uvw::ipv6`.
 *
 * To create a `tcp_handle` through a `loop`, arguments follow:
 *
 * * An optional integer value that indicates the flags used to initialize
 * the socket.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/tcp.html#c.uv_tcp_init_ex)
 * for further details.
 */
class tcp_handle final: public stream_handle<tcp_handle, uv_tcp_t> {
public:
    using time = std::chrono::duration<unsigned int>;
    using tcp_flags = details::uvw_tcp_flags;
    using ipv4 = uvw::ipv4;
    using ipv6 = uvw::ipv6;

    explicit tcp_handle(loop::token token, std::shared_ptr<loop> ref, unsigned int f = {});

    /**
     * @brief Initializes the handle. No socket is created as of yet.
     * @return Underlying return value.
     */
    int init() final;

    /**
     * @brief Opens an existing file descriptor or SOCKET as a TCP handle.
     *
     * The passed file descriptor or SOCKET is not checked for its type, but
     * it’s required that it represents a valid stream socket.
     *
     * @param socket A valid socket handle (either a file descriptor or a
     * SOCKET).
     *
     * @return Underlying return value.
     */
    int open(os_socket_handle socket);

    /**
     * @brief Enables/Disables Nagle’s algorithm.
     * @param value True to enable it, false otherwise.
     * @return True in case of success, false otherwise.
     */
    bool no_delay(bool value = false);

    /**
     * @brief Enables/Disables TCP keep-alive.
     * @param enable True to enable it, false otherwise.
     * @param val Initial delay in seconds (use
     * `std::chrono::duration<unsigned int>`).
     * @return True in case of success, false otherwise.
     */
    bool keep_alive(bool enable = false, time val = time{0});

    /**
     * @brief Enables/Disables simultaneous asynchronous accept requests.
     *
     * Enables/Disables simultaneous asynchronous accept requests that are
     * queued by the operating system when listening for new TCP
     * connections.<br/>
     * This setting is used to tune a TCP server for the desired performance.
     * Having simultaneous accepts can significantly improve the rate of
     * accepting connections (which is why it is enabled by default) but may
     * lead to uneven load distribution in multi-process setups.
     *
     * @param enable True to enable it, false otherwise.
     * @return True in case of success, false otherwise.
     */
    bool simultaneous_accepts(bool enable = true);

    /**
     * @brief Binds the handle to an address and port.
     *
     * A successful call to this function does not guarantee that the call to
     * `listen()` or `connect()` will work properly.
     *
     * Available flags are:
     *
     * * `tcp_handle::tcp_flags::IPV6ONLY`: it disables dual-stack support and
     * only IPv6 is used.
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param opts Optional additional flags.
     * @return Underlying return value.
     */
    int bind(const sockaddr &addr, tcp_flags opts = tcp_flags::_UVW_ENUM);

    /**
     * @brief Binds the handle to an address and port.
     *
     * A successful call to this function does not guarantee that the call to
     * `listen()` or `connect()` will work properly.
     *
     * Available flags are:
     *
     * * `tcp_handle::tcp_flags::IPV6ONLY`: it disables dual-stack support and
     * only IPv6 is used.
     *
     * @param ip The address to which to bind.
     * @param port The port to which to bind.
     * @param opts Optional additional flags.
     * @return Underlying return value.
     */
    int bind(const std::string &ip, unsigned int port, tcp_flags opts = tcp_flags::_UVW_ENUM);

    /**
     * @brief Binds the handle to an address and port.
     *
     * A successful call to this function does not guarantee that the call to
     * `listen()` or `connect()` will work properly.
     *
     * Available flags are:
     *
     * * `tcp_handle::tcp_flags::IPV6ONLY`: it disables dual-stack support and
     * only IPv6 is used.
     *
     * @param addr A valid instance of socket_address.
     * @param opts Optional additional flags.
     * @return Underlying return value.
     */
    int bind(socket_address addr, tcp_flags opts = tcp_flags::_UVW_ENUM);

    /**
     * @brief Gets the current address to which the handle is bound.
     * @return A valid instance of socket_address, an empty one in case of
     * errors.
     */
    socket_address sock() const noexcept;

    /**
     * @brief Gets the address of the peer connected to the handle.
     * @return A valid instance of socket_address, an empty one in case of
     * errors.
     */
    socket_address peer() const noexcept;

    /**
     * @brief Establishes an IPv4 or IPv6 TCP connection.
     *
     * On Windows if the addr is initialized to point to an unspecified address
     * (`0.0.0.0` or `::`) it will be changed to point to localhost. This is
     * done to match the behavior of Linux systems.
     *
     * A connect event is emitted when the connection has been established.
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @return Underlying return value.
     */
    int connect(const sockaddr &addr);

    /**
     * @brief Establishes an IPv4 or IPv6 TCP connection.
     *
     * A connect event is emitted when the connection has been established.
     *
     * @param ip The address to which to bind.
     * @param port The port to which to bind.
     * @return Underlying return value.
     */
    int connect(const std::string &ip, unsigned int port);

    /**
     * @brief Establishes an IPv4 or IPv6 TCP connection.
     *
     * A connect event is emitted when the connection has been established.
     *
     * @param addr A valid instance of socket_address.
     * @return Underlying return value.
     */
    int connect(socket_address addr);

    /**
     * @brief Resets a TCP connection by sending a RST packet.
     *
     * This is accomplished by setting the `SO_LINGER` socket option with a
     * linger interval of zero and then calling `close`.<br/>
     * Due to some platform inconsistencies, mixing of `shutdown` and
     * `close_reset` calls is not allowed.
     *
     * A close event is emitted when the connection has been reset.
     *
     * @return Underlying return value.
     */
    int close_reset();

private:
    enum {
        DEFAULT,
        FLAGS
    } tag;

    unsigned int flags;
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "tcp.cpp"
#endif

#endif // UVW_TCP_INCLUDE_H
