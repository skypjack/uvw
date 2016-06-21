#pragma once


#include <string>
#include <cstdint>
#include <chrono>
#include <ratio>
#include <uv.h>
#include "stream.hpp"
#include "error.hpp"


namespace uvw {


class Tcp final: public Stream<Tcp> {
    static void protoConnect(uv_connect_t* req, int status) {
        auto handle = req->handle;
        delete req;
        static_cast<Tcp*>(handle->data)->connCb(UVWError{status});
    }

public:
    using Time = std::chrono::duration<uint64_t>;
    using CallbackConnect = std::function<void(UVWError)>;

    enum { IPv4, IPv6 };

    explicit Tcp(uv_loop_t *loop): Stream<Tcp>{&handle} {
        uv_tcp_init(loop, &handle);
    }

    ~Tcp() {
        close([](UVWError){});
    }

    bool noDelay(bool value = false) noexcept {
        return (uv_tcp_nodelay(&handle, value ? 1 : 0) == 0);
    }

    bool keepAlive(bool enable = false, Time time = Time{0}) noexcept {
        return (uv_tcp_keepalive(&handle, enable ? 1 : 0, time.count()) == 0);
    }

    template<int>
    void connect(std::string, int, CallbackConnect) noexcept;

private:
    CallbackConnect connCb;
    uv_tcp_t handle;
};


template<>
void Tcp::connect<Tcp::IPv4>(std::string ip, int port, CallbackConnect cb) noexcept {
    uv_connect_t *conn = new uv_connect_t;
    sockaddr_in addr;
    uv_ip4_addr(ip.c_str(), port, &addr);
    connCb = cb;
    auto err = uv_tcp_connect(conn, &handle, reinterpret_cast<const sockaddr*>(&addr), &protoConnect);

    if(err) {
        connCb(UVWError{err});
    }
}

template<>
void Tcp::connect<Tcp::IPv6>(std::string ip, int port, CallbackConnect cb) noexcept {
    uv_connect_t *conn = new uv_connect_t;
    sockaddr_in6 addr;
    uv_ip6_addr(ip.c_str(), port, &addr);
    connCb = cb;
    auto err = uv_tcp_connect(conn, &handle, reinterpret_cast<const sockaddr*>(&addr), &protoConnect);

    if(err) {
        connCb(UVWError{err});
    }
}


}
