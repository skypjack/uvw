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


class Send final: public Request<Send> {
    explicit Send(std::shared_ptr<Loop> ref)
        : Request{RequestType<uv_udp_send_t>{}, std::move(ref)}
    { }

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


class Udp final: public Handle<Udp> {
    using SockFunctionType = Addr(*)(const Udp &);
    using PeerFunctionType = Addr(*)(const sockaddr *);

    template<typename I>
    static Addr tSock(const Udp &udp) noexcept {
        return details::address<I>(uv_udp_getsockname, udp.get<uv_udp_t>());
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    static Addr tPeer(const sockaddr *addr) noexcept {
        const typename Traits::Type *aptr = reinterpret_cast<const typename Traits::Type *>(addr);
        int len = sizeof(*addr);
        return details::address<I>(aptr, len);
    }

    explicit Udp(std::shared_ptr<Loop> ref)
        : Handle{HandleType<uv_udp_t>{}, std::move(ref)},
          sockF{&tSock<details::IPv4>},
          peerF{&tPeer<details::IPv4>}
    { }

    static void recvCallback(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const sockaddr *addr, unsigned flags) {
        Udp &udp = *(static_cast<Udp*>(handle->data));
        // data will be destroyed no matter of what the value of nread is
        std::unique_ptr<const char[]> data{buf->base};

        if(nread > 0) {
            // data available (can be truncated)
            udp.publish(UDPDataEvent{udp.peerF(addr), std::move(data), nread, flags & UV_UDP_PARTIAL});
        } else if(nread == 0 && addr == nullptr) {
            // no more data to be read, doing nothing is fine
        } else if(nread == 0 && addr != nullptr) {
            // empty udp packet
            udp.publish(UDPDataEvent{udp.peerF(addr), std::move(data), nread, false});
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

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void bind(std::string ip, unsigned int port, Flags<Bind> flags = Flags<Bind>{}) {
        typename Traits::Type addr;
        Traits::AddrFunc(ip.data(), port, &addr);

        if(0 == invoke(&uv_udp_bind, get<uv_udp_t>(), reinterpret_cast<const sockaddr *>(&addr), flags)) {
            sockF = &tSock<I>;
            peerF = &tPeer<I>;
        }
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void bind(Addr addr, Flags<Bind> flags = Flags<Bind>{}) {
        bind<I>(addr.ip, addr.port, flags);
    }

    Addr sock() const noexcept { return sockF(*this); }

    template<typename I>
    void multicastMembership(std::string multicast, std::string interface, Membership membership) {
        if(0 == invoke(&uv_udp_set_membership, get<uv_udp_t>(), multicast.data(), interface.data(), static_cast<uv_membership>(membership))) {
            sockF = &tSock<I>;
            peerF = &tPeer<I>;
        }
    }

    void multicastLoop(bool enable = true) {
        invoke(&uv_udp_set_multicast_loop, get<uv_udp_t>(), enable);
    }

    void multicastTttl(int val) {
        invoke(&uv_udp_set_multicast_ttl, get<uv_udp_t>(), val > 255 ? 255 : val);
    }

    template<typename I>
    void multicastInterface(std::string interface) {
        if(0 == invoke(&uv_udp_set_multicast_interface, get<uv_udp_t>(), interface.data())) {
            sockF = &tSock<I>;
            peerF = &tPeer<I>;
        }
    }

    void broadcast(bool enable = false) { invoke(&uv_udp_set_broadcast, get<uv_udp_t>(), enable); }
    void ttl(int val) { invoke(&uv_udp_set_ttl, get<uv_udp_t>(), val > 255 ? 255 : val); }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void send(std::string ip, unsigned int port, char *data, ssize_t len) {
        typename Traits::Type addr;
        Traits::AddrFunc(ip.data(), port, &addr);

        uv_buf_t bufs[] = { uv_buf_init(data, len) };

        auto listener = [ptr = this->shared_from_this()](const auto &event, details::Send &) {
            ptr->sockF = &tSock<I>;
            ptr->peerF = &tPeer<I>;
            ptr->publish(event);
        };

        auto send = this->loop().resource<details::Send>();
        send->once<ErrorEvent>(listener);
        send->once<SendEvent>(listener);
        send->send(get<uv_udp_t>(), bufs, 1, reinterpret_cast<const sockaddr *>(&addr));

        sockF = &tSock<I>;
        peerF = &tPeer<I>;
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void send(std::string ip, unsigned int port, std::unique_ptr<char[]> data, ssize_t len) {
        send<I>(ip, port, data.get(), len);
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    int trySend(std::string ip, unsigned int port, char *data, ssize_t len) {
        typename Traits::Type addr;
        Traits::AddrFunc(ip.data(), port, &addr);

        uv_buf_t bufs[] = { uv_buf_init(data, len) };
        auto bw = uv_udp_try_send(get<uv_udp_t>(), bufs, 1, reinterpret_cast<const sockaddr *>(&addr));

        if(bw < 0) {
            this->publish(ErrorEvent{bw});
            bw = 0;
        } else {
            sockF = &tSock<I>;
            peerF = &tPeer<I>;
        }

        return bw;
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    int trySend(std::string ip, unsigned int port, std::unique_ptr<char[]> data, ssize_t len) {
        return trySend<I>(ip, port, data.get(), len);
    }

    void recv() { invoke(&uv_udp_recv_start, get<uv_udp_t>(), &allocCallback, &recvCallback); }
    void stop() { invoke(&uv_udp_recv_stop, get<uv_udp_t>()); }

private:
    SockFunctionType sockF;
    PeerFunctionType peerF;
};


}
