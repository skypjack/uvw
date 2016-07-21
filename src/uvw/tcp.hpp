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


class Tcp final: public Stream<Tcp> {
    using SockFunctionType = Addr(*)(const Tcp &);
    using PeerFunctionType = SockFunctionType;

    template<typename I>
    static Addr tSock(const Tcp &tcp) noexcept {
        return details::address<I>(uv_tcp_getsockname, tcp.get<uv_tcp_t>());
    }

    template<typename I>
    static Addr tPeer(const Tcp &tcp) noexcept {
        return details::address<I>(uv_tcp_getpeername, tcp.get<uv_tcp_t>());
    }

    explicit Tcp(std::shared_ptr<Loop> ref)
        : Stream{HandleType<uv_tcp_t>{}, std::move(ref)},
          sockF{&tSock<details::IPv4>},
          peerF{&tPeer<details::IPv4>}
    { }

public:
    using Time = std::chrono::seconds;

    using IPv4 = details::IPv4;
    using IPv6 = details::IPv6;

    enum class Bind: std::underlying_type_t<uv_tcp_flags> {
        IPV6ONLY = UV_TCP_IPV6ONLY
    };

    template<typename... Args>
    static std::shared_ptr<Tcp> create(Args&&... args) {
        return std::shared_ptr<Tcp>{new Tcp{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_tcp_t>(&uv_tcp_init); }

    void noDelay(bool value = false) {
        invoke(&uv_tcp_nodelay, get<uv_tcp_t>(), value);
    }

    void keepAlive(bool enable = false, Time time = Time{0}) {
        invoke(&uv_tcp_keepalive, get<uv_tcp_t>(), enable, time.count());
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void bind(std::string ip, unsigned int port, Flags<Bind> flags = Flags<Bind>{}) {
        typename Traits::Type addr;
        Traits::AddrFunc(ip.data(), port, &addr);

        if(0 == invoke(&uv_tcp_bind, get<uv_tcp_t>(), reinterpret_cast<const sockaddr *>(&addr), flags)) {
            sockF = &tSock<I>;
            peerF = &tPeer<I>;
        }
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void bind(Addr addr, Flags<Bind> flags = Flags<Bind>{}) {
        bind<I>(addr.ip, addr.port, flags);
    }

    Addr sock() const noexcept { return sockF(*this); }
    Addr peer() const noexcept { return peerF(*this); }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void connect(std::string ip, unsigned int port) {
        typename Traits::Type addr;
        Traits::AddrFunc(ip.data(), port, &addr);

        auto listener = [ptr = this->shared_from_this()](const auto &event, details::Connect &) {
            ptr->sockF = &tSock<I>;
            ptr->peerF = &tPeer<I>;
            ptr->publish(event);
        };

        auto connect = loop().resource<details::Connect>();
        connect->once<ErrorEvent>(listener);
        connect->once<ConnectEvent>(listener);
        connect->connect(&uv_tcp_connect, get<uv_tcp_t>(), reinterpret_cast<const sockaddr *>(&addr));
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void connect(Addr addr) { connect<I>(addr.ip, addr.port); }

    void accept(Tcp &tcp) override {
        if(0 == invoke(&uv_accept, get<uv_stream_t>(), tcp.get<uv_stream_t>())) {
            tcp.sockF = sockF;
            tcp.peerF = peerF;
        }
    }

private:
    SockFunctionType sockF;
    PeerFunctionType peerF;
};


}
