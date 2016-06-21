#pragma once


#include <cstdint>
#include <memory>
#include <chrono>
#include <ratio>
#include <uv.h>
#include "stream.hpp"
#include "error.hpp"


namespace uvw {


class Tcp final: public Stream {
    static void protoConnect(uv_connect_t* req, int status) {
        Tcp &tcp = *(static_cast<Tcp*>(req->handle->data));

        if(status) {
            tcp.connCb(UVWError{status}, Handle<Connection>{});
        } else {
            auto h = tcp.spawn<Connection>();
            tcp.reset<uv_tcp_t>();
            uv_tcp_init(tcp.parent(), tcp.get<uv_tcp_t>());
            tcp.connCb(UVWError{}, h);
        }
    }

public:
    using Time = std::chrono::duration<uint64_t>;
    using CallbackConnect = std::function<void(UVWError, Handle<Connection>)>;

    enum { IPv4, IPv6 };

    explicit Tcp(std::shared_ptr<Loop> ref)
        : Stream{HandleType<uv_tcp_t>{}, ref},
          conn{std::make_unique<uv_connect_t>()}
    {
        uv_tcp_init(parent(), get<uv_tcp_t>());
    }

    bool noDelay(bool value = false) noexcept {
        return (uv_tcp_nodelay(get<uv_tcp_t>(), value ? 1 : 0) == 0);
    }

    bool keepAlive(bool enable = false, Time time = Time{0}) noexcept {
        return (uv_tcp_keepalive(get<uv_tcp_t>(), enable ? 1 : 0, time.count()) == 0);
    }

    template<int>
    void connect(std::string, int, CallbackConnect) noexcept;

private:
    std::unique_ptr<uv_connect_t> conn;
    CallbackConnect connCb;
};


template<>
void Tcp::connect<Tcp::IPv4>(std::string ip, int port, CallbackConnect cb) noexcept {
    sockaddr_in addr;
    uv_ip4_addr(ip.c_str(), port, &addr);
    connCb = cb;
    auto err = uv_tcp_connect(conn.get(), get<uv_tcp_t>(), reinterpret_cast<const sockaddr*>(&addr), &protoConnect);

    if(err) {
        connCb(UVWError{err}, Handle<Connection>{});
    }
}


template<>
void Tcp::connect<Tcp::IPv6>(std::string ip, int port, CallbackConnect cb) noexcept {
    sockaddr_in6 addr;
    uv_ip6_addr(ip.c_str(), port, &addr);
    connCb = cb;
    auto err = uv_tcp_connect(conn.get(), get<uv_tcp_t>(), reinterpret_cast<const sockaddr*>(&addr), &protoConnect);

    if(err) {
        connCb(UVWError{err}, Handle<Connection>{});
    }
}


}
