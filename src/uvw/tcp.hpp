#pragma once


#include <utility>
#include <cstdint>
#include <string>
#include <memory>
#include <chrono>
#include <ratio>
#include <uv.h>
#include "stream.hpp"
#include "util.hpp"


namespace uvw {


class Tcp final: public Stream<Tcp> {
    static void connectCallback(Tcp &tcp, std::function<void(UVWError, Tcp &)> &cb, uv_connect_t *, int status) {
        cb(UVWError{status}, tcp);
    }

    explicit Tcp(std::shared_ptr<Loop> ref)
        : Stream{HandleType<uv_tcp_t>{}, std::move(ref)},
          conn{std::make_unique<uv_connect_t>()}
    {
        initialized = (uv_tcp_init(parent(), get<uv_tcp_t>()) == 0);
    }

    explicit Tcp(std::shared_ptr<Loop> ref, uv_stream_t *srv): Tcp{ref} {
        initialized = initialized || (uv_accept(srv, get<uv_stream_t>()) == 0);
    }

public:
    using Time = std::chrono::duration<uint64_t>;

    enum IP { IPv4, IPv6 };

    template<typename... Args>
    static std::shared_ptr<Tcp> create(Args&&... args) {
        return std::shared_ptr<Tcp>{new Tcp{std::forward<Args>(args)...}};
    }

    UVWError noDelay(bool value = false) noexcept {
        return UVWError{uv_tcp_nodelay(get<uv_tcp_t>(), value ? 1 : 0)};
    }

    UVWError keepAlive(bool enable = false, Time time = Time{0}) noexcept {
        return UVWError{uv_tcp_keepalive(get<uv_tcp_t>(), enable ? 1 : 0, time.count())};
    }

    template<IP>
    UVWError bind(std::string, unsigned int, bool = false) noexcept;

    template<IP>
    UVWError bind(Addr, bool = false) noexcept;

    template<IP>
    void connect(std::string, unsigned int, std::function<void(UVWError, Tcp &)>) noexcept;

    template<IP>
    void connect(Addr addr, std::function<void(UVWError, Tcp &)>) noexcept;

    UVWError accept(Tcp &tcp) {
        return UVWError{uv_accept(get<uv_stream_t>(), tcp.get<uv_stream_t>())};
    }

    explicit operator bool() { return initialized; }

private:
    std::unique_ptr<uv_connect_t> conn;
    bool initialized;
};


template<>
UVWError Tcp::bind<Tcp::IPv4>(std::string ip, unsigned int port, bool ipv6only) noexcept {
    sockaddr_in addr;
    uv_ip4_addr(ip.c_str(), port, &addr);
    unsigned int flags = ipv6only ? UV_TCP_IPV6ONLY : 0;
    return UVWError(uv_tcp_bind(get<uv_tcp_t>(), reinterpret_cast<const sockaddr*>(&addr), flags));
}


template<>
UVWError Tcp::bind<Tcp::IPv6>(std::string ip, unsigned int port, bool ipv6only) noexcept {
    sockaddr_in6 addr;
    uv_ip6_addr(ip.c_str(), port, &addr);
    unsigned int flags = ipv6only ? UV_TCP_IPV6ONLY : 0;
    return UVWError(uv_tcp_bind(get<uv_tcp_t>(), reinterpret_cast<const sockaddr*>(&addr), flags));
}


template<>
UVWError Tcp::bind<Tcp::IPv4>(Addr addr, bool ipv6only) noexcept {
    return bind<Tcp::IPv4>(addr.ip, addr.port, ipv6only);
}


template<>
UVWError Tcp::bind<Tcp::IPv6>(Addr addr, bool ipv6only) noexcept {
    return bind<Tcp::IPv6>(addr.ip, addr.port, ipv6only);
}


template<>
void Tcp::connect<Tcp::IPv4>(std::string ip, unsigned int port, std::function<void(UVWError, Tcp &)> cb) noexcept {
    sockaddr_in addr;
    uv_ip4_addr(ip.c_str(), port, &addr);
    using CBF = CallbackFactory<void(uv_connect_t *, int)>;
    auto func = CBF::template once<&Tcp::connectCallback>(*this, cb);
    auto err = uv_tcp_connect(conn.get(), get<uv_tcp_t>(), reinterpret_cast<const sockaddr*>(&addr), func);
    if(err) { error(err); }
}


template<>
void Tcp::connect<Tcp::IPv6>(std::string ip, unsigned int port, std::function<void(UVWError, Tcp &)> cb) noexcept {
    sockaddr_in6 addr;
    uv_ip6_addr(ip.c_str(), port, &addr);
    using CBF = CallbackFactory<void(uv_connect_t *, int)>;
    auto func = CBF::template once<&Tcp::connectCallback>(*this, cb);
    auto err = uv_tcp_connect(conn.get(), get<uv_tcp_t>(), reinterpret_cast<const sockaddr*>(&addr), func);
    if(err) { error(err); }
}


template<>
void Tcp::connect<Tcp::IPv4>(Addr addr, std::function<void(UVWError, Tcp &)> cb) noexcept {
    connect<Tcp::IPv4>(addr.ip, addr.port, cb);
}


template<>
void Tcp::connect<Tcp::IPv6>(Addr addr, std::function<void(UVWError, Tcp &)> cb) noexcept {
    connect<Tcp::IPv6>(addr.ip, addr.port, cb);
}


}
