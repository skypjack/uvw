#pragma once


#include <type_traits>
#include <utility>
#include <memory>
#include <string>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


/**
 * @brief SendEvent event.
 *
 * It will be emitted by UDPHandle according with its functionalities.
 */
struct SendEvent: Event<SendEvent> { };


/**
 * @brief UDPDataEvent event.
 *
 * It will be emitted by UDPHandle according with its functionalities.
 */
struct UDPDataEvent: Event<UDPDataEvent> {
    explicit UDPDataEvent(Addr addr, std::unique_ptr<const char[]> ptr, ssize_t l, bool trunc) noexcept
        : dt{std::move(ptr)}, len{l}, sndr(addr), part{trunc}
    { }

    /**
     * @brief Gets the data read on the stream.
     * @return A bunch of data read on the stream.
     */
    const char * data() const noexcept { return dt.get(); }

    /**
     * @brief Gets the amount of data read on the stream.
     * @return The amount of data read on the stream.
     */
    ssize_t length() const noexcept { return len; }

    /**
     * @brief Gets the address of the sender.
     * @return A valid instance of Addr.
     */
    Addr sender() const noexcept { return sndr; }

    /**
     * @brief Indicates if the message was truncated.
     * @return True if the message was truncated, false otherwise.
     */
    bool partial() const noexcept { return part; }

private:
    std::unique_ptr<const char[]> dt;
    const ssize_t len;
    Addr sndr;
    const bool part;
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


class Send final: public Request<Send, uv_udp_send_t> {
    using Request::Request;

public:
    template<typename... Args>
    static std::shared_ptr<Send> create(Args&&... args) {
        return std::shared_ptr<Send>{new Send{std::forward<Args>(args)...}};
    }

    void send(uv_udp_t *handle, const uv_buf_t bufs[], unsigned int nbufs, const struct sockaddr* addr) {
        invoke(&uv_udp_send, get<uv_udp_send_t>(), handle, bufs, nbufs, addr, &defaultCallback<uv_udp_send_t, SendEvent>);
    }
};


}


/**
 * @brief The UDPHandle handle.
 *
 * UDP handles encapsulate UDP communication for both clients and servers.<br/>
 * By default, _IPv4_ is used as a template parameter. The handle already
 * supports _IPv6_ out-of-the-box by using `uvw::IPv6`.
 */
class UDPHandle final: public Handle<UDPHandle, uv_udp_t> {
    template<typename I>
    static void recvCallback(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const sockaddr *addr, unsigned flags) {
        typename details::IpTraits<I>::Type *aptr = reinterpret_cast<const typename details::IpTraits<I>::Type *>(addr);

        UDPHandle &udp = *(static_cast<UDPHandle*>(handle->data));
        // data will be destroyed no matter of what the value of nread is
        std::unique_ptr<const char[]> data{buf->base};

        if(nread > 0) {
            // data available (can be truncated)
            udp.publish(UDPDataEvent{details::address<I>(aptr), std::move(data), nread, flags & UV_UDP_PARTIAL});
        } else if(nread == 0 && addr == nullptr) {
            // no more data to be read, doing nothing is fine
        } else if(nread == 0 && addr != nullptr) {
            // empty udp packet
            udp.publish(UDPDataEvent{details::address<I>(aptr), std::move(data), nread, false});
        } else {
            // transmission error
            udp.publish(ErrorEvent(nread));
        }
    }

    explicit UDPHandle(std::shared_ptr<Loop> ref)
        : Handle{std::move(ref)}, tag{DEFAULT}, flags{}
    { }

    explicit UDPHandle(std::shared_ptr<Loop> ref, unsigned int f)
        : Handle{std::move(ref)}, tag{FLAGS}, flags{f}
    { }

public:
    using Membership = details::UVMembership;
    using Bind = details::UVUdpFlags;
    using IPv4 = uvw::IPv4;
    using IPv6 = uvw::IPv6;

    /**
     * @brief Creates a new udp handle.
     * @param args
     *
     * * A pointer to the loop from which the handle generated.
     * * An optional integer value (_flags_) that indicates optional flags used
     * to initialize the socket.<br/>
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/udp.html#c.uv_udp_init_ex)
     * for further details.
     *
     * @return A pointer to the newly created handle.
     */
    template<typename... Args>
    static std::shared_ptr<UDPHandle> create(Args&&... args) {
        return std::shared_ptr<UDPHandle>{new UDPHandle{std::forward<Args>(args)...}};
    }

    /**
     * @brief Initializes the handle. The actual socket is created lazily.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return (tag == FLAGS)
                ? initialize<uv_udp_t>(&uv_udp_init_ex, flags)
                : initialize<uv_udp_t>(&uv_udp_init);
    }

    /**
     * @brief Opens an existing file descriptor or SOCKET as a UDP handle.
     *
     * The passed file descriptor or SOCKET is not checked for its type, but
     * it’s required that it represents a valid stream socket.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/udp.html#c.uv_udp_open)
     * for further details.
     *
     * @param sock A valid socket handle (either a file descriptor or a SOCKET).
     */
    void open(OSSocketHandle sock) {
        invoke(&uv_udp_open, get<uv_udp_t>(), sock);
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
     * @param flags Optional additional flags.
     */
    template<typename I = IPv4>
    void bind(std::string ip, unsigned int port, Flags<Bind> flags = Flags<Bind>{}) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::addrFunc(ip.data(), port, &addr);
        invoke(&uv_udp_bind, get<uv_udp_t>(), reinterpret_cast<const sockaddr *>(&addr), flags);
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
     * @param flags Optional additional flags.
     */
    template<typename I = IPv4>
    void bind(Addr addr, Flags<Bind> flags = Flags<Bind>{}) {
        bind<I>(addr.ip, addr.port, flags);
    }

    /**
     * @brief Get the local IP and port of the UDP handle.
     * @return A valid instance of Addr, an empty one in case of errors.
     */
    template<typename I = IPv4>
    Addr sock() const noexcept {
        return details::address<I>(&uv_udp_getsockname, get<uv_udp_t>());
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
     * @param interface Interface address.
     * @param membership Action to be performed.
     */
    template<typename I = IPv4>
    void multicastMembership(std::string multicast, std::string interface, Membership membership) {
        invoke(&uv_udp_set_membership, get<uv_udp_t>(), multicast.data(), interface.data(), static_cast<uv_membership>(membership));
    }

    /**
     * @brief Sets IP multicast loop flag.
     *
     * This makes multicast packets loop back to local sockets.
     *
     * @param enable True to enable multicast loop, false otherwise.
     */
    void multicastLoop(bool enable = true) {
        invoke(&uv_udp_set_multicast_loop, get<uv_udp_t>(), enable);
    }

    /**
     * @brief Sets the multicast ttl.
     * @param val A value in the range `[1, 255]`.
     */
    void multicastTtl(int val) {
        invoke(&uv_udp_set_multicast_ttl, get<uv_udp_t>(), val > 255 ? 255 : val);
    }

    /**
     * @brief Sets the multicast interface to send or receive data on.
     * @param interface Interface address.
     */
    template<typename I = IPv4>
    void multicastInterface(std::string interface) {
        invoke(&uv_udp_set_multicast_interface, get<uv_udp_t>(), interface.data());
    }

    /**
     * @brief Sets broadcast on or off.
     * @param enable True to set broadcast on, false otherwise.
     */
    void broadcast(bool enable = false) {
        invoke(&uv_udp_set_broadcast, get<uv_udp_t>(), enable);
    }

    /**
     * @brief Sets the time to live.
     * @param val A value in the range `[1, 255]`.
     */
    void ttl(int val) {
        invoke(&uv_udp_set_ttl, get<uv_udp_t>(), val > 255 ? 255 : val);
    }

    /**
     * @brief Sends data over the UDP socket.
     *
     * Note that if the socket has not previously been bound with `bind()`, it
     * will be bound to `0.0.0.0` (the _all interfaces_ IPv4 address) and a
     * random port number.
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
    void send(std::string ip, unsigned int port, std::unique_ptr<char[]> data, ssize_t len) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::addrFunc(ip.data(), port, &addr);

        uv_buf_t bufs[] = { uv_buf_init(data.get(), len) };

        auto listener = [ptr = shared_from_this()](const auto &event, details::Send &) {
            ptr->publish(event);
        };

        auto send = loop().resource<details::Send>();
        send->once<ErrorEvent>(listener);
        send->once<SendEvent>(listener);
        send->send(get<uv_udp_t>(), bufs, 1, reinterpret_cast<const sockaddr *>(&addr));
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
    int trySend(std::string ip, unsigned int port, std::unique_ptr<char[]> data, ssize_t len) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::addrFunc(ip.data(), port, &addr);

        uv_buf_t bufs[] = { uv_buf_init(data.get(), len) };
        auto bw = uv_udp_try_send(get<uv_udp_t>(), bufs, 1, reinterpret_cast<const sockaddr *>(&addr));

        if(bw < 0) {
            publish(ErrorEvent{bw});
            bw = 0;
        }

        return bw;
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
        invoke(&uv_udp_recv_start, get<uv_udp_t>(), &allocCallback, &recvCallback<I>);
    }

    /**
     * @brief Stops listening for incoming datagrams.
     */
    void stop() {
        invoke(&uv_udp_recv_stop, get<uv_udp_t>());
    }

private:
    enum { DEFAULT, FLAGS } tag;
    unsigned int flags;
};


}
