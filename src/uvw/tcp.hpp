#pragma once


#include <type_traits>
#include <utility>
#include <memory>
#include <string>
#include <chrono>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "stream.hpp"
#include "util.hpp"


namespace uvw {


namespace details {


enum class UVTcpFlags: std::underlying_type_t<uv_tcp_flags> {
    IPV6ONLY = UV_TCP_IPV6ONLY
};


}


/**
 * @brief The TcpHandle handle.
 *
 * TCP handles are used to represent both TCP streams and servers.<br/>
 * By default, _IPv4_ is used as a template parameter. The handle already
 * supports _IPv6_ out-of-the-box by using `uvw::IPv6`.
 *
 * To create a `TcpHandle` through a `Loop`, arguments follow:
 *
 * * An optional integer value that indicates the flags used to initialize
 * the socket.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/tcp.html#c.uv_tcp_init_ex)
 * for further details.
 */
class TcpHandle final: public StreamHandle<TcpHandle, uv_tcp_t> {
public:
    using Time = std::chrono::seconds;
    using Bind = details::UVTcpFlags;
    using IPv4 = uvw::IPv4;
    using IPv6 = uvw::IPv6;

    using StreamHandle::StreamHandle;

    explicit TcpHandle(ConstructorAccess ca, std::shared_ptr<Loop> ref, unsigned int f)
        : StreamHandle{std::move(ca), std::move(ref)}, tag{FLAGS}, flags{f}
    {}

    /**
     * @brief Initializes the handle. No socket is created as of yet.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return (tag == FLAGS)
                ? initialize(&uv_tcp_init_ex, flags)
                : initialize(&uv_tcp_init);
    }

    /**
     * @brief Opens an existing file descriptor or SOCKET as a TCP handle.
     *
     * The passed file descriptor or SOCKET is not checked for its type, but
     * it’s required that it represents a valid stream socket.
     *
     * @param sock A valid socket handle (either a file descriptor or a SOCKET).
     */
    void open(OSSocketHandle _sock) {
        invoke(&uv_tcp_open, get(), _sock);
    }

    /**
     * @brief Enables/Disables Nagle’s algorithm.
     * @param value True to enable it, false otherwise.
     * @return True in case of success, false otherwise.
     */
    bool noDelay(bool value = false) {
        return (0 == uv_tcp_nodelay(get(), value));
    }

    /**
     * @brief Enables/Disables TCP keep-alive.
     * @param enable True to enable it, false otherwise.
     * @param time Initial delay in seconds (use `std::chrono::seconds`).
     * @return True in case of success, false otherwise.
     */
    bool keepAlive(bool enable = false, Time time = Time{0}) {
        return (0 == uv_tcp_keepalive(get(), enable, time.count()));
    }

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
    bool simultaneousAccepts(bool enable = true) {
        return (0 == uv_tcp_simultaneous_accepts(get(), enable));
    }

    /**
     * @brief Binds the handle to an address and port.
     *
     * A successful call to this function does not guarantee that the call to
     * `listen()` or `connect()` will work properly.<br/>
     * `ErrorEvent` events can be emitted because of either this function or the
     * ones mentioned above.
     *
     * Available flags are:
     *
     * * `TcpHandle::Bind::IPV6ONLY`: it disables dual-stack support and only
     * IPv6 is used.
     *
     * @param ip The address to which to bind.
     * @param port The port to which to bind.
     * @param flags Optional additional flags.
     */
    template<typename I = IPv4>
    void bind(std::string ip, unsigned int port, Flags<Bind> _flags = Flags<Bind>{}) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::addrFunc(ip.data(), port, &addr);
        invoke(&uv_tcp_bind, get(), reinterpret_cast<const sockaddr *>(&addr), _flags);
    }

    /**
     * @brief Binds the handle to an address and port.
     *
     * A successful call to this function does not guarantee that the call to
     * `listen()` or `connect()` will work properly.<br/>
     * `ErrorEvent` events can be emitted because of either this function or the
     * ones mentioned above.
     *
     * Available flags are:
     *
     * * `TcpHandle::Bind::IPV6ONLY`: it disables dual-stack support and only
     * IPv6 is used.
     *
     * @param addr A valid instance of Addr.
     * @param flags Optional additional flags.
     */
    template<typename I = IPv4>
    void bind(Addr addr, Flags<Bind> _flags = Flags<Bind>{}) {
        bind<I>(addr.ip, addr.port, _flags);
    }

    /**
     * @brief Gets the current address to which the handle is bound.
     * @return A valid instance of Addr, an empty one in case of errors.
     */
    template<typename I = IPv4>
    Addr sock() const noexcept {
        return details::address<I>(&uv_tcp_getsockname, get());
    }

    /**
     * @brief Gets the address of the peer connected to the handle.
     * @return A valid instance of Addr, an empty one in case of errors.
     */
    template<typename I = IPv4>
    Addr peer() const noexcept {
        return details::address<I>(&uv_tcp_getpeername, get());
    }

    /**
     * @brief Establishes an IPv4 or IPv6 TCP connection.
     *
     * A ConnectEvent event is emitted when the connection has been
     * established.<br/>
     * An ErrorEvent event is emitted in case of errors during the connection.
     *
     * @param ip The address to which to bind.
     * @param port The port to which to bind.
     */
    template<typename I = IPv4>
    void connect(std::string ip, unsigned int port) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::addrFunc(ip.data(), port, &addr);

        auto listener = [ptr = shared_from_this()](const auto &event, details::ConnectReq &) {
            ptr->publish(event);
        };

        auto _connect = loop().resource<details::ConnectReq>();
        _connect->once<ErrorEvent>(listener);
        _connect->once<ConnectEvent>(listener);
        _connect->connect(&uv_tcp_connect, get(), reinterpret_cast<const sockaddr *>(&addr));
    }

    /**
     * @brief Establishes an IPv4 or IPv6 TCP connection.
     *
     * A ConnectEvent event is emitted when the connection has been
     * established.<br/>
     * An ErrorEvent event is emitted in case of errors during the connection.
     *
     * @param addr A valid instance of Addr.
     */
    template<typename I = IPv4>
    void connect(Addr addr) {
        connect<I>(addr.ip, addr.port);
    }

private:
    enum { DEFAULT, FLAGS } tag{DEFAULT};
    unsigned int flags{};
};


}
