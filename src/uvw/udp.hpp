#pragma once


#include <type_traits>
#include <algorithm>
#include <iterator>
#include <utility>
#include <cstddef>
#include <memory>
#include <string>
#include <uv.h>
#include "request.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


/**
 * @brief SendEvent event.
 *
 * It will be emitted by UDPHandle according with its functionalities.
 */
struct SendEvent {};


/**
 * @brief UDPDataEvent event.
 *
 * It will be emitted by UDPHandle according with its functionalities.
 */
struct UDPDataEvent {
    explicit UDPDataEvent(Addr sndr, std::unique_ptr<const char[]> buf, std::size_t len, bool part) noexcept
        : data{std::move(buf)}, length{len}, sender{std::move(sndr)}, partial{part}
    {}

    std::unique_ptr<const char[]> data; /*!< A bunch of data read on the stream. */
    std::size_t length;  /*!< The amount of data read on the stream. */
    Addr sender; /*!< A valid instance of Addr. */
    bool partial; /*!< True if the message was truncated, false otherwise. */
};


namespace details {


enum class UVUdpFlags: std::underlying_type_t<uv_udp_flags> {
    IPV6ONLY = UV_UDP_IPV6ONLY,
    REUSEADDR = UV_UDP_REUSEADDR
};


enum class UVMembership: std::underlying_type_t<uv_membership> {
    LEAVE_GROUP = UV_LEAVE_GROUP,
    JOIN_GROUP = UV_JOIN_GROUP
};


class SendReq final: public Request<SendReq, uv_udp_send_t> {
public:
    using Deleter = void(*)(char *);

    SendReq(ConstructorAccess ca, std::shared_ptr<Loop> loop, std::unique_ptr<char[], Deleter> dt, unsigned int len)
        : Request<SendReq, uv_udp_send_t>{ca, std::move(loop)},
          data{std::move(dt)},
          buf{uv_buf_init(data.get(), len)}
    {}

    void send(uv_udp_t *handle, const struct sockaddr* addr) {
        invoke(&uv_udp_send, get(), handle, &buf, 1, addr, &defaultCallback<SendEvent>);
    }

private:
    std::unique_ptr<char[], Deleter> data;
    uv_buf_t buf;
};


}


/**
 * @brief The UDPHandle handle.
 *
 * UDP handles encapsulate UDP communication for both clients and servers.<br/>
 * By default, _IPv4_ is used as a template parameter. The handle already
 * supports _IPv6_ out-of-the-box by using `uvw::IPv6`.
 *
 * To create an `UDPHandle` through a `Loop`, arguments follow:
 *
 * * An optional integer value that indicates optional flags used to initialize
 * the socket.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/udp.html#c.uv_udp_init_ex)
 * for further details.
 */
class UDPHandle final: public Handle<UDPHandle, uv_udp_t> {
    template<typename I>
    static void recvCallback(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const sockaddr *addr, unsigned flags) {
        const typename details::IpTraits<I>::Type *aptr = reinterpret_cast<const typename details::IpTraits<I>::Type *>(addr);

        UDPHandle &udp = *(static_cast<UDPHandle*>(handle->data));
        // data will be destroyed no matter of what the value of nread is
        std::unique_ptr<const char[]> data{buf->base};

        if(nread > 0) {
            // data available (can be truncated)
            udp.publish(UDPDataEvent{details::address<I>(aptr), std::move(data), static_cast<std::size_t>(nread), !(0 == (flags & UV_UDP_PARTIAL))});
        } else if(nread == 0 && addr == nullptr) {
            // no more data to be read, doing nothing is fine
        } else if(nread == 0 && addr != nullptr) {
            // empty udp packet
            udp.publish(UDPDataEvent{details::address<I>(aptr), std::move(data), static_cast<std::size_t>(nread), false});
        } else {
            // transmission error
            udp.publish(ErrorEvent(nread));
        }
    }

public:
    using Membership = details::UVMembership;
    using Bind = details::UVUdpFlags;
    using IPv4 = uvw::IPv4;
    using IPv6 = uvw::IPv6;

    using Handle::Handle;

    explicit UDPHandle(ConstructorAccess ca, std::shared_ptr<Loop> ref, unsigned int f)
        : Handle{ca, std::move(ref)}, tag{FLAGS}, flags{f}
    {}

    /**
     * @brief Initializes the handle. The actual socket is created lazily.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return (tag == FLAGS)
                ? initialize(&uv_udp_init_ex, flags)
                : initialize(&uv_udp_init);
    }

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
     * @param socket A valid socket handle (either a file descriptor or a SOCKET).
     */
    void open(OSSocketHandle socket) {
        invoke(&uv_udp_open, get(), socket);
    }

    /**
     * @brief Binds the UDP handle to an IP address and port.
     *
     * Available flags are:
     *
     * * `UDPHandle::Bind::IPV6ONLY`
     * * `UDPHandle::Bind::REUSEADDR`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/udp.html#c.uv_udp_flags)
     * for further details.
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param opts Optional additional flags.
     */
    void bind(const sockaddr &addr, Flags<Bind> opts = Flags<Bind>{}) {
        invoke(&uv_udp_bind, get(), &addr, opts);
    }

    /**
     * @brief Binds the UDP handle to an IP address and port.
     *
     * Available flags are:
     *
     * * `UDPHandle::Bind::IPV6ONLY`
     * * `UDPHandle::Bind::REUSEADDR`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/udp.html#c.uv_udp_flags)
     * for further details.
     *
     * @param ip The IP address to which to bind.
     * @param port The port to which to bind.
     * @param opts Optional additional flags.
     */
    template<typename I = IPv4>
    void bind(std::string ip, unsigned int port, Flags<Bind> opts = Flags<Bind>{}) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::addrFunc(ip.data(), port, &addr);
        bind(reinterpret_cast<const sockaddr &>(addr), std::move(opts));
    }

    /**
     * @brief Binds the UDP handle to an IP address and port.
     *
     * Available flags are:
     *
     * * `UDPHandle::Bind::IPV6ONLY`
     * * `UDPHandle::Bind::REUSEADDR`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/udp.html#c.uv_udp_flags)
     * for further details.
     *
     * @param addr A valid instance of Addr.
     * @param opts Optional additional flags.
     */
    template<typename I = IPv4>
    void bind(Addr addr, Flags<Bind> opts = Flags<Bind>{}) {
        bind<I>(std::move(addr.ip), addr.port, std::move(opts));
    }

    /**
     * @brief Get the local IP and port of the UDP handle.
     * @return A valid instance of Addr, an empty one in case of errors.
     */
    template<typename I = IPv4>
    Addr sock() const noexcept {
        return details::address<I>(&uv_udp_getsockname, get());
    }

    /**
     * @brief Sets membership for a multicast address.
     *
     * Available values for `membership` are:
     *
     * * `UDPHandle::Membership::LEAVE_GROUP`
     * * `UDPHandle::Membership::JOIN_GROUP`
     *
     * @param multicast Multicast address to set membership for.
     * @param iface Interface address.
     * @param membership Action to be performed.
     * @return True in case of success, false otherwise.
     */
    template<typename I = IPv4>
    bool multicastMembership(std::string multicast, std::string iface, Membership membership) {
        return (0 == uv_udp_set_membership(get(), multicast.data(), iface.data(), static_cast<uv_membership>(membership)));
    }

    /**
     * @brief Sets IP multicast loop flag.
     *
     * This makes multicast packets loop back to local sockets.
     *
     * @param enable True to enable multicast loop, false otherwise.
     * @return True in case of success, false otherwise.
     */
    bool multicastLoop(bool enable = true) {
        return (0 == uv_udp_set_multicast_loop(get(), enable));
    }

    /**
     * @brief Sets the multicast ttl.
     * @param val A value in the range `[1, 255]`.
     * @return True in case of success, false otherwise.
     */
    bool multicastTtl(int val) {
        return (0 == uv_udp_set_multicast_ttl(get(), val > 255 ? 255 : val));
    }

    /**
     * @brief Sets the multicast interface to send or receive data on.
     * @param iface Interface address.
     * @return True in case of success, false otherwise.
     */
    template<typename I = IPv4>
    bool multicastInterface(std::string iface) {
        return (0 == uv_udp_set_multicast_interface(get(), iface.data()));
    }

    /**
     * @brief Sets broadcast on or off.
     * @param enable True to set broadcast on, false otherwise.
     * @return True in case of success, false otherwise.
     */
    bool broadcast(bool enable = false) {
        return (0 == uv_udp_set_broadcast(get(), enable));
    }

    /**
     * @brief Sets the time to live.
     * @param val A value in the range `[1, 255]`.
     * @return True in case of success, false otherwise.
     */
    bool ttl(int val) {
        return (0 == uv_udp_set_ttl(get(), val > 255 ? 255 : val));
    }

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
     * A SendEvent event will be emitted when the data have been sent.<br/>
     * An ErrorEvent event will be emitted in case of errors.
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     */
    void send(const sockaddr &addr, std::unique_ptr<char[]> data, unsigned int len) {
        auto req = loop().resource<details::SendReq>(
                    std::unique_ptr<char[], details::SendReq::Deleter>{
                        data.release(), [](char *ptr) { delete[] ptr; }
                    }, len);

        auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
            ptr->publish(event);
        };

        req->once<ErrorEvent>(listener);
        req->once<SendEvent>(listener);
        req->send(get(), &addr);
    }

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
     * A SendEvent event will be emitted when the data have been sent.<br/>
     * An ErrorEvent event will be emitted in case of errors.
     *
     * @param ip The address to which to send data.
     * @param port The port to which to send data.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     */
    template<typename I = IPv4>
    void send(std::string ip, unsigned int port, std::unique_ptr<char[]> data, unsigned int len) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::addrFunc(ip.data(), port, &addr);
        send(reinterpret_cast<const sockaddr &>(addr), std::move(data), len);
    }

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
     * A SendEvent event will be emitted when the data have been sent.<br/>
     * An ErrorEvent event will be emitted in case of errors.
     *
     * @param addr A valid instance of Addr.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     */
    template<typename I = IPv4>
    void send(Addr addr, std::unique_ptr<char[]> data, unsigned int len) {
        send<I>(std::move(addr.ip), addr.port, std::move(data), len);
    }

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
     * A SendEvent event will be emitted when the data have been sent.<br/>
     * An ErrorEvent event will be emitted in case of errors.
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     */
    void send(const sockaddr &addr, char *data, unsigned int len) {
        auto req = loop().resource<details::SendReq>(
                    std::unique_ptr<char[], details::SendReq::Deleter>{
                        data, [](char *) {}
                    }, len);

        auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
            ptr->publish(event);
        };

        req->once<ErrorEvent>(listener);
        req->once<SendEvent>(listener);
        req->send(get(), &addr);
    }

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
     * A SendEvent event will be emitted when the data have been sent.<br/>
     * An ErrorEvent event will be emitted in case of errors.
     *
     * @param ip The address to which to send data.
     * @param port The port to which to send data.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     */
    template<typename I = IPv4>
    void send(std::string ip, unsigned int port, char *data, unsigned int len) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::addrFunc(ip.data(), port, &addr);
        send(reinterpret_cast<const sockaddr &>(addr), data, len);
    }

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
     * A SendEvent event will be emitted when the data have been sent.<br/>
     * An ErrorEvent event will be emitted in case of errors.
     *
     * @param addr A valid instance of Addr.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     */
    template<typename I = IPv4>
    void send(Addr addr, char *data, unsigned int len) {
        send<I>(std::move(addr.ip), addr.port, data, len);
    }

    /**
     * @brief Sends data over the UDP socket.
     *
     * Same as `send()`, but it won’t queue a send request if it can’t be
     * completed immediately.
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Number of bytes written.
     */
    template<typename I = IPv4>
    int trySend(const sockaddr &addr, std::unique_ptr<char[]> data, unsigned int len) {
        uv_buf_t bufs[] = { uv_buf_init(data.get(), len) };
        auto bw = uv_udp_try_send(get(), bufs, 1, &addr);

        if(bw < 0) {
            publish(ErrorEvent{bw});
            bw = 0;
        }

        return bw;
    }

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
     * @return Number of bytes written.
     */
    template<typename I = IPv4>
    int trySend(std::string ip, unsigned int port, std::unique_ptr<char[]> data, unsigned int len) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::addrFunc(ip.data(), port, &addr);
        return trySend(reinterpret_cast<const sockaddr &>(addr), std::move(data), len);
    }

    /**
     * @brief Sends data over the UDP socket.
     *
     * Same as `send()`, but it won’t queue a send request if it can’t be
     * completed immediately.
     *
     * @param addr A valid instance of Addr.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Number of bytes written.
     */
    template<typename I = IPv4>
    int trySend(Addr addr, std::unique_ptr<char[]> data, unsigned int len) {
        return trySend<I>(std::move(addr.ip), addr.port, std::move(data), len);
    }

    /**
     * @brief Sends data over the UDP socket.
     *
     * Same as `send()`, but it won’t queue a send request if it can’t be
     * completed immediately.
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Number of bytes written.
     */
    template<typename I = IPv4>
    int trySend(const sockaddr &addr, char *data, unsigned int len) {
        uv_buf_t bufs[] = { uv_buf_init(data, len) };
        auto bw = uv_udp_try_send(get(), bufs, 1, &addr);

        if(bw < 0) {
            publish(ErrorEvent{bw});
            bw = 0;
        }

        return bw;
    }

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
     * @return Number of bytes written.
     */
    template<typename I = IPv4>
    int trySend(std::string ip, unsigned int port, char *data, unsigned int len) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::addrFunc(ip.data(), port, &addr);
        return trySend(reinterpret_cast<const sockaddr &>(addr), data, len);
    }

    /**
     * @brief Sends data over the UDP socket.
     *
     * Same as `send()`, but it won’t queue a send request if it can’t be
     * completed immediately.
     *
     * @param addr A valid instance of Addr.
     * @param data The data to be sent.
     * @param len The lenght of the submitted data.
     * @return Number of bytes written.
     */
    template<typename I = IPv4>
    int trySend(Addr addr, char *data, unsigned int len) {
        return trySend<I>(std::move(addr.ip), addr.port, data, len);
    }

    /**
     * @brief Prepares for receiving data.
     *
     * Note that if the socket has not previously been bound with `bind()`, it
     * is bound to `0.0.0.0` (the _all interfaces_ IPv4 address) and a random
     * port number.
     *
     * An UDPDataEvent event will be emitted when the handle receives data.<br/>
     * An ErrorEvent event will be emitted in case of errors.
     */
    template<typename I = IPv4>
    void recv() {
        invoke(&uv_udp_recv_start, get(), &allocCallback, &recvCallback<I>);
    }

    /**
     * @brief Stops listening for incoming datagrams.
     */
    void stop() {
        invoke(&uv_udp_recv_stop, get());
    }

    /**
     * @brief Gets the number of bytes queued for sending.
     *
     * It strictly shows how much information is currently queued.
     *
     * @return Number of bytes queued for sending.
     */
    size_t sendQueueSize() const noexcept {
        return uv_udp_get_send_queue_size(get());
    }

    /**
     * @brief Number of send requests currently in the queue awaiting to be processed.
     * @return Number of send requests currently in the queue.
     */
    size_t sendQueueCount() const noexcept {
        return uv_udp_get_send_queue_count(get());
    }

private:
    enum { DEFAULT, FLAGS } tag{DEFAULT};
    unsigned int flags{};
};


}
