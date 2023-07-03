#ifndef UVW_UDP_INCLUDE_H
#define UVW_UDP_INCLUDE_H

#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <uv.h>
#include "config.h"
#include "enum.hpp"
#include "handle.hpp"
#include "request.hpp"
#include "util.h"

namespace uvw {

/*! @brief Send event. */
struct send_event {};

/*! @brief UDP data event. */
struct udp_data_event {
    explicit udp_data_event(socket_address sndr, std::unique_ptr<char[]> buf, std::size_t len, bool part) noexcept;

    std::unique_ptr<char[]> data; /*!< A bunch of data read on the stream. */
    std::size_t length;           /*!< The amount of data read on the stream. */
    socket_address sender;        /*!< A valid instance of socket_address. */
    bool partial;                 /*!< True if the message was truncated, false otherwise. */
};

namespace details {

enum class uvw_udp_flags : std::underlying_type_t<uv_udp_flags> {
    IPV6ONLY = UV_UDP_IPV6ONLY,
    UDP_PARTIAL = UV_UDP_PARTIAL,
    REUSEADDR = UV_UDP_REUSEADDR,
    UDP_MMSG_CHUNK = UV_UDP_MMSG_CHUNK,
    UDP_MMSG_FREE = UV_UDP_MMSG_FREE,
    UDP_LINUX_RECVERR = UV_UDP_LINUX_RECVERR,
    UDP_RECVMMSG = UV_UDP_RECVMMSG,
    _UVW_ENUM = 0
};

enum class uvw_membership : std::underlying_type_t<uv_membership> {
    LEAVE_GROUP = UV_LEAVE_GROUP,
    JOIN_GROUP = UV_JOIN_GROUP
};

class send_req final: public request<send_req, uv_udp_send_t, send_event> {
    static void udp_send_callback(uv_udp_send_t *req, int status);

public:
    using deleter = void (*)(char *);

    send_req(loop::token token, std::shared_ptr<loop> parent, std::unique_ptr<char[], deleter> dt, unsigned int len);

    int send(uv_udp_t *hndl, const struct sockaddr *addr);

private:
    std::unique_ptr<char[], deleter> data;
    uv_buf_t buf;
};

} // namespace details

/**
 * @brief The UDP handle.
 *
 * UDP handles encapsulate UDP communication for both clients and servers.<br/>
 * By default, _ipv4_ is used as a template parameter. The handle already
 * supports _IPv6_ out-of-the-box by using `uvw::ipv6`.
 *
 * To create an `udp_handle` through a `loop`, arguments follow:
 *
 * * An optional integer value that indicates optional flags used to initialize
 * the socket.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/udp.html#c.uv_udp_init_ex)
 * for further details.
 */
class udp_handle final: public handle<udp_handle, uv_udp_t, send_event, udp_data_event> {
    static void recv_callback(uv_udp_t *hndl, ssize_t nread, const uv_buf_t *buf, const sockaddr *addr, unsigned flags);

public:
    using membership = details::uvw_membership;
    using udp_flags = details::uvw_udp_flags;
    using ipv4 = uvw::ipv4;
    using ipv6 = uvw::ipv6;

    explicit udp_handle(loop::token token, std::shared_ptr<loop> ref, unsigned int f = {});

    /**
     * @brief Initializes the handle. The actual socket is created lazily.
     * @return Underlying return value.
     */
    int init() final;

    /**
     * @brief Opens an existing file descriptor or SOCKET as a UDP handle.
     *
     * The passed file descriptor or SOCKET is not checked for its type, but
     * it’s required that it represents a valid datagram socket.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/udp.html#c.uv_udp_open)
     * for further details.
     *
     * @param socket A valid socket handle (either a file descriptor or a
     * SOCKET).
     *
     * @return Underlying return value.
     */
    int open(os_socket_handle socket);

    /**
     * @brief Associates the handle to a remote address and port (either IPv4 or
     * IPv6).
     *
     * Every message sent by this handle is automatically sent to the given
     * destination.<br/>
     * Trying to call this function on an already connected handle isn't
     * allowed.
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @return Underlying return value.
     */
    int connect(const sockaddr &addr);

    /**
     * @brief Associates the handle to a remote address and port (either IPv4 or
     * IPv6).
     *
     * Every message sent by this handle is automatically sent to the given
     * destination.<br/>
     * Trying to call this function on an already connected handle isn't
     * allowed.
     *
     * @param ip The address to which to bind.
     * @param port The port to which to bind.
     * @return Underlying return value.
     */
    int connect(const std::string &ip, unsigned int port);

    /**
     * @brief Associates the handle to a remote address and port (either IPv4 or
     * IPv6).
     *
     * Every message sent by this handle is automatically sent to the given
     * destination.<br/>
     * Trying to call this function on an already connected handle isn't
     * allowed.
     *
     * @param addr A valid instance of socket_address.
     * @return Underlying return value.
     */
    int connect(socket_address addr);

    /**
     * @brief Disconnects the handle.
     *
     * Trying to disconnect a handle that is not connected isn't allowed.
     *
     * @return Underlying return value.
     */
    int disconnect();

    /**
     * @brief Gets the remote address to which the handle is connected, if any.
     * @return A valid instance of socket_address, an empty one in case of
     * errors.
     */
    socket_address peer() const noexcept;

    /**
     * @brief Binds the UDP handle to an IP address and port.
     *
     * Available flags are:
     *
     * * `udp_handle::udp_flags::IPV6ONLY`
     * * `udp_handle::udp_flags::UDP_PARTIAL`
     * * `udp_handle::udp_flags::REUSEADDR`
     * * `udp_handle::udp_flags::UDP_MMSG_CHUNK`
     * * `udp_handle::udp_flags::UDP_MMSG_FREE`
     * * `udp_handle::udp_flags::UDP_LINUX_RECVERR`
     * * `udp_handle::udp_flags::UDP_RECVMMSG`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/udp.html#c.uv_udp_flags)
     * for further details.
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param opts Optional additional flags.
     * @return Underlying return value.
     */
    int bind(const sockaddr &addr, udp_flags opts = udp_flags::_UVW_ENUM);

    /**
     * @brief Binds the UDP handle to an IP address and port.
     *
     * Available flags are:
     *
     * * `udp_handle::udp_flags::IPV6ONLY`
     * * `udp_handle::udp_flags::UDP_PARTIAL`
     * * `udp_handle::udp_flags::REUSEADDR`
     * * `udp_handle::udp_flags::UDP_MMSG_CHUNK`
     * * `udp_handle::udp_flags::UDP_MMSG_FREE`
     * * `udp_handle::udp_flags::UDP_LINUX_RECVERR`
     * * `udp_handle::udp_flags::UDP_RECVMMSG`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/udp.html#c.uv_udp_flags)
     * for further details.
     *
     * @param ip The IP address to which to bind.
     * @param port The port to which to bind.
     * @param opts Optional additional flags.
     * @return Underlying return value.
     */
    int bind(const std::string &ip, unsigned int port, udp_flags opts = udp_flags::_UVW_ENUM);

    /**
     * @brief Binds the UDP handle to an IP address and port.
     *
     * Available flags are:
     *
     * * `udp_handle::udp_flags::IPV6ONLY`
     * * `udp_handle::udp_flags::UDP_PARTIAL`
     * * `udp_handle::udp_flags::REUSEADDR`
     * * `udp_handle::udp_flags::UDP_MMSG_CHUNK`
     * * `udp_handle::udp_flags::UDP_MMSG_FREE`
     * * `udp_handle::udp_flags::UDP_LINUX_RECVERR`
     * * `udp_handle::udp_flags::UDP_RECVMMSG`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/udp.html#c.uv_udp_flags)
     * for further details.
     *
     * @param addr A valid instance of socket_address.
     * @param opts Optional additional flags.
     * @return Underlying return value.
     */
    int bind(socket_address addr, udp_flags opts = udp_flags::_UVW_ENUM);

    /**
     * @brief Get the local IP and port of the UDP handle.
     * @return A valid instance of socket_address, an empty one in case of
     * errors.
     */
    socket_address sock() const noexcept;

    /**
     * @brief Sets membership for a multicast address.
     *
     * Available values for `ms` are:
     *
     * * `udp_handle::membership::LEAVE_GROUP`
     * * `udp_handle::membership::JOIN_GROUP`
     *
     * @param multicast Multicast address to set membership for.
     * @param iface Interface address.
     * @param ms Action to be performed.
     * @return True in case of success, false otherwise.
     */
    bool multicast_membership(const std::string &multicast, const std::string &iface, membership ms);

    /**
     * @brief Sets IP multicast loop flag.
     *
     * This makes multicast packets loop back to local sockets.
     *
     * @param enable True to enable multicast loop, false otherwise.
     * @return True in case of success, false otherwise.
     */
    bool multicast_loop(bool enable = true);

    /**
     * @brief Sets the multicast ttl.
     * @param val A value in the range `[1, 255]`.
     * @return True in case of success, false otherwise.
     */
    bool multicast_ttl(int val);

    /**
     * @brief Sets the multicast interface to send or receive data on.
     * @param iface Interface address.
     * @return True in case of success, false otherwise.
     */
    bool multicast_interface(const std::string &iface);

    /**
     * @brief Sets broadcast on or off.
     * @param enable True to set broadcast on, false otherwise.
     * @return True in case of success, false otherwise.
     */
    bool broadcast(bool enable = false);

    /**
     * @brief Sets the time to live.
     * @param val A value in the range `[1, 255]`.
     * @return True in case of success, false otherwise.
     */
    bool ttl(int val);

    /**
     * @brief Sends data over the UDP socket.
     *
     * Note that if the socket has not previously been bound with `bind()`, it
     * will be bound to `0.0.0.0` (the _all interfaces_ IPv4 address) and a
     * random port number.
     *
     * The handle takes the ownership of the data and it is in charge of delete
     * them.
     *
     * A send event will be emitted when the data have been sent.
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Underlying return value.
     */
    int send(const sockaddr &addr, std::unique_ptr<char[]> data, unsigned int len);

    /**
     * @brief Sends data over the UDP socket.
     *
     * Note that if the socket has not previously been bound with `bind()`, it
     * will be bound to `0.0.0.0` (the _all interfaces_ IPv4 address) and a
     * random port number.
     *
     * The handle takes the ownership of the data and it is in charge of delete
     * them.
     *
     * A send event will be emitted when the data have been sent.
     *
     * @param ip The address to which to send data.
     * @param port The port to which to send data.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Underlying return value.
     */
    int send(const std::string &ip, unsigned int port, std::unique_ptr<char[]> data, unsigned int len);

    /**
     * @brief Sends data over the UDP socket.
     *
     * Note that if the socket has not previously been bound with `bind()`, it
     * will be bound to `0.0.0.0` (the _all interfaces_ IPv4 address) and a
     * random port number.
     *
     * The handle takes the ownership of the data and it is in charge of delete
     * them.
     *
     * A send event will be emitted when the data have been sent.
     *
     * @param addr A valid instance of socket_address.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Underlying return value.
     */
    int send(socket_address addr, std::unique_ptr<char[]> data, unsigned int len);

    /**
     * @brief Sends data over the UDP socket.
     *
     * Note that if the socket has not previously been bound with `bind()`, it
     * will be bound to `0.0.0.0` (the _all interfaces_ IPv4 address) and a
     * random port number.
     *
     * The handle doesn't take the ownership of the data. Be sure that their
     * lifetime overcome the one of the request.
     *
     * A send event will be emitted when the data have been sent.
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Underlying return value.
     */
    int send(const sockaddr &addr, char *data, unsigned int len);

    /**
     * @brief Sends data over the UDP socket.
     *
     * Note that if the socket has not previously been bound with `bind()`, it
     * will be bound to `0.0.0.0` (the _all interfaces_ IPv4 address) and a
     * random port number.
     *
     * The handle doesn't take the ownership of the data. Be sure that their
     * lifetime overcome the one of the request.
     *
     * A send event will be emitted when the data have been sent.
     *
     * @param ip The address to which to send data.
     * @param port The port to which to send data.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Underlying return value.
     */
    int send(const std::string &ip, unsigned int port, char *data, unsigned int len);

    /**
     * @brief Sends data over the UDP socket.
     *
     * Note that if the socket has not previously been bound with `bind()`, it
     * will be bound to `0.0.0.0` (the _all interfaces_ IPv4 address) and a
     * random port number.
     *
     * The handle doesn't take the ownership of the data. Be sure that their
     * lifetime overcome the one of the request.
     *
     * A send event will be emitted when the data have been sent.
     *
     * @param addr A valid instance of socket_address.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Underlying return value.
     */
    int send(socket_address addr, char *data, unsigned int len);

    /**
     * @brief Sends data over the UDP socket.
     *
     * Same as `send()`, but it won’t queue a send request if it can’t be
     * completed immediately.
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Underlying return value.
     */
    int try_send(const sockaddr &addr, std::unique_ptr<char[]> data, unsigned int len);

    /**
     * @brief Sends data over the UDP socket.
     *
     * Same as `send()`, but it won’t queue a send request if it can’t be
     * completed immediately.
     *
     * @param ip The address to which to send data.
     * @param port The port to which to send data.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Underlying return value.
     */
    int try_send(const std::string &ip, unsigned int port, std::unique_ptr<char[]> data, unsigned int len);

    /**
     * @brief Sends data over the UDP socket.
     *
     * Same as `send()`, but it won’t queue a send request if it can’t be
     * completed immediately.
     *
     * @param addr A valid instance of socket_address.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Underlying return value.
     */
    int try_send(socket_address addr, std::unique_ptr<char[]> data, unsigned int len);

    /**
     * @brief Sends data over the UDP socket.
     *
     * Same as `send()`, but it won’t queue a send request if it can’t be
     * completed immediately.
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Underlying return value.
     */
    int try_send(const sockaddr &addr, char *data, unsigned int len);

    /**
     * @brief Sends data over the UDP socket.
     *
     * Same as `send()`, but it won’t queue a send request if it can’t be
     * completed immediately.
     *
     * @param ip The address to which to send data.
     * @param port The port to which to send data.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Underlying return value.
     */
    int try_send(const std::string &ip, unsigned int port, char *data, unsigned int len);

    /**
     * @brief Sends data over the UDP socket.
     *
     * Same as `send()`, but it won’t queue a send request if it can’t be
     * completed immediately.
     *
     * @param addr A valid instance of socket_address.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Underlying return value.
     */
    int try_send(socket_address addr, char *data, unsigned int len);

    /**
     * @brief Prepares for receiving data.
     *
     * Note that if the socket has not previously been bound with `bind()`, it
     * is bound to `0.0.0.0` (the _all interfaces_ IPv4 address) and a random
     * port number.
     *
     * An UDP data event will be emitted when the handle receives data.
     *
     * @return Underlying return value.
     */
    int recv();

    /**
     * @brief Prepares for receiving data.
     * @sa recv
     * @tparam Alloc Custom allocation function.
     * @return Underlying return value.
     */
    template<auto Alloc>
    int recv() {
        return uv_udp_recv_start(raw(), &details::common_alloc_callback<udp_handle, Alloc>, &recv_callback);
    }

    /**
     * @brief Stops listening for incoming datagrams.
     * @return Underlying return value.
     */
    int stop();

    /**
     * @brief Gets the number of bytes queued for sending.
     *
     * It strictly shows how much information is currently queued.
     *
     * @return Number of bytes queued for sending.
     */
    size_t send_queue_size() const noexcept;

    /**
     * @brief Number of send requests currently in the queue awaiting to be
     * processed.
     * @return Number of send requests currently in the queue.
     */
    size_t send_queue_count() const noexcept;

private:
    enum {
        DEFAULT,
        FLAGS
    } tag{DEFAULT};

    unsigned int flags{};
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "udp.cpp"
#endif

#endif // UVW_UDP_INCLUDE_H
