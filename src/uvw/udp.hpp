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


namespace details {


class Send final: public Request<Send, uv_udp_send_t> {
    using Request::Request;

public:
    template<typename... Args>
    static std::shared_ptr<Send> create(Args&&... args) {
        return std::shared_ptr<Send>{new Send{std::forward<Args>(args)...}};
    }

    void send(uv_udp_t *handle, const uv_buf_t bufs[], unsigned int nbufs, const struct sockaddr* addr) {
        exec<uv_udp_send_t, SendEvent>(&uv_udp_send, get<uv_udp_send_t>(), handle, bufs, nbufs, addr);
    }
};


}


class Udp final: public Handle<Udp, uv_udp_t> {
    using Handle::Handle;

    template<typename I>
    static void recvCallback(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const sockaddr *addr, unsigned flags) {
        typename details::IpTraits<I>::Type *aptr = reinterpret_cast<const typename details::IpTraits<I>::Type *>(addr);
        int len = sizeof(*addr);

        Udp &udp = *(static_cast<Udp*>(handle->data));
        // data will be destroyed no matter of what the value of nread is
        std::unique_ptr<const char[]> data{buf->base};

        if(nread > 0) {
            // data available (can be truncated)
            udp.publish(UDPDataEvent{details::address<I>(aptr, len), std::move(data), nread, flags & UV_UDP_PARTIAL});
        } else if(nread == 0 && addr == nullptr) {
            // no more data to be read, doing nothing is fine
        } else if(nread == 0 && addr != nullptr) {
            // empty udp packet
            udp.publish(UDPDataEvent{details::address<I>(aptr, len), std::move(data), nread, false});
        } else {
            // transmission error
            udp.publish(ErrorEvent(nread));
        }
    }

public:
    using IPv4 = details::IPv4;
    using IPv6 = details::IPv6;

    enum class Bind: std::underlying_type_t<uv_udp_flags> {
        IPV6ONLY = UV_UDP_IPV6ONLY,
        REUSEADDR = UV_UDP_REUSEADDR
    };

    enum class Membership: std::underlying_type_t<uv_membership> {
        LEAVE_GROUP = UV_LEAVE_GROUP,
        JOIN_GROUP = UV_JOIN_GROUP
    };

    template<typename... Args>
    static std::shared_ptr<Udp> create(Args&&... args) {
        return std::shared_ptr<Udp>{new Udp{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_udp_t>(&uv_udp_init); }

    template<typename T, typename... Args>
    bool init(T&& t, Args&&... args) {
        return initialize<uv_udp_t>(&uv_udp_init_ex, std::forward<T>(t), std::forward<Args>(args)...);
    }

    void open(OSSocketHandle sock) {
        invoke(&uv_udp_open, get<uv_udp_t>(), sock);
    }

    template<typename I = IPv4>
    void bind(std::string ip, unsigned int port, Flags<Bind> flags = Flags<Bind>{}) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::AddrFunc(ip.data(), port, &addr);
        invoke(&uv_udp_bind, get<uv_udp_t>(), reinterpret_cast<const sockaddr *>(&addr), flags);
    }

    template<typename I = IPv4>
    void bind(Addr addr, Flags<Bind> flags = Flags<Bind>{}) {
        bind<I>(addr.ip, addr.port, flags);
    }

    template<typename I = IPv4>
    Addr sock() const noexcept {
        return details::address<I>(&uv_udp_getsockname, get<uv_udp_t>());
    }

    template<typename I = IPv4>
    void multicastMembership(std::string multicast, std::string interface, Membership membership) {
        invoke(&uv_udp_set_membership, get<uv_udp_t>(), multicast.data(), interface.data(), static_cast<uv_membership>(membership));
    }

    void multicastLoop(bool enable = true) {
        invoke(&uv_udp_set_multicast_loop, get<uv_udp_t>(), enable);
    }

    void multicastTttl(int val) {
        invoke(&uv_udp_set_multicast_ttl, get<uv_udp_t>(), val > 255 ? 255 : val);
    }

    template<typename I = IPv4>
    void multicastInterface(std::string interface) {
        invoke(&uv_udp_set_multicast_interface, get<uv_udp_t>(), interface.data());
    }

    void broadcast(bool enable = false) { invoke(&uv_udp_set_broadcast, get<uv_udp_t>(), enable); }
    void ttl(int val) { invoke(&uv_udp_set_ttl, get<uv_udp_t>(), val > 255 ? 255 : val); }

    template<typename I = IPv4>
    void send(std::string ip, unsigned int port, char *data, ssize_t len) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::AddrFunc(ip.data(), port, &addr);

        uv_buf_t bufs[] = { uv_buf_init(data, len) };

        auto listener = [ptr = shared_from_this()](const auto &event, details::Send &) {
            ptr->publish(event);
        };

        auto send = loop().resource<details::Send>();
        send->once<ErrorEvent>(listener);
        send->once<SendEvent>(listener);
        send->send(get<uv_udp_t>(), bufs, 1, reinterpret_cast<const sockaddr *>(&addr));
    }

    template<typename I = IPv4>
    void send(std::string ip, unsigned int port, std::unique_ptr<char[]> data, ssize_t len) {
        send<I>(ip, port, data.get(), len);
    }

    template<typename I = IPv4>
    int trySend(std::string ip, unsigned int port, char *data, ssize_t len) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::AddrFunc(ip.data(), port, &addr);

        uv_buf_t bufs[] = { uv_buf_init(data, len) };
        auto bw = uv_udp_try_send(get<uv_udp_t>(), bufs, 1, reinterpret_cast<const sockaddr *>(&addr));

        if(bw < 0) {
            publish(ErrorEvent{bw});
            bw = 0;
        }

        return bw;
    }

    template<typename I = IPv4>
    int trySend(std::string ip, unsigned int port, std::unique_ptr<char[]> data, ssize_t len) {
        return trySend<I>(ip, port, data.get(), len);
    }

    template<typename I = IPv4>
    void recv() {
        invoke(&uv_udp_recv_start, get<uv_udp_t>(), &allocCallback, &recvCallback<I>);
    }

    void stop() { invoke(&uv_udp_recv_stop, get<uv_udp_t>()); }
};


}
