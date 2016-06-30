#pragma once


#include <utility>
#include <memory>
#include <string>
#include <chrono>
#include <uv.h>
#include "event.hpp"
#include "stream.hpp"
#include "util.hpp"


namespace uvw {


class Tcp final: public Stream<Tcp> {
    static void connectCallback(Tcp &tcp, uv_connect_t *, int status) {
        ConnectEvent event;
        event.error = UVWError{status};
        tcp.publish(std::move(event));
    }

    explicit Tcp(std::shared_ptr<Loop> ref)
        : Stream{ResourceType<uv_tcp_t>{}, std::move(ref)},
          conn{std::make_unique<uv_connect_t>()}
    {
        initialized = (uv_tcp_init(parent(), get<uv_tcp_t>()) == 0);
    }

    template<typename I, typename F, typename..., typename Traits = details::IpTraits<I>>
    UVWOptionalData<Addr> address(F &&f) {
        sockaddr_storage addr;
        int len = sizeof(addr);
        char name[sizeof(addr)];

        int err = std::forward<F>(f)(get<uv_tcp_t>(), reinterpret_cast<sockaddr *>(&addr), &len);

        if(!err) {
            typename Traits::Type *aptr = reinterpret_cast<typename Traits::Type *>(&addr);
            err = Traits::NameFunc(aptr, name, len);

            if(!err) {
                Addr address{ std::string{name}, ntohs(aptr->sin_port) };

                return UVWOptionalData<Addr>{std::move(address)};
            }
        }

        return UVWOptionalData<Addr>{UVWError{err}};
    }

public:
    using Time = std::chrono::seconds;

    using IPv4 = details::IPv4;
    using IPv6 = details::IPv6;

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

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    UVWError bind(std::string ip, unsigned int port, bool ipv6only = false) noexcept {
        typename Traits::Type addr;
        Traits::AddrFunc(ip.c_str(), port, &addr);
        unsigned int flags = ipv6only ? UV_TCP_IPV6ONLY : 0;
        return UVWError(uv_tcp_bind(get<uv_tcp_t>(), reinterpret_cast<const sockaddr *>(&addr), flags));
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    UVWError bind(Addr addr, bool ipv6only = false) noexcept {
        return bind<I>(addr.first, addr.second, ipv6only);
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    UVWOptionalData<Addr> address() {
        return address<I>(uv_tcp_getsockname);
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    UVWOptionalData<Addr> remote() {
        return address<I>(uv_tcp_getpeername);
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    UVWError connect(std::string ip, unsigned int port) noexcept {
        typename Traits::Type addr;
        Traits::AddrFunc(ip.c_str(), port, &addr);
        using CBF = CallbackFactory<void(uv_connect_t *, int)>;
        auto func = CBF::create<&Tcp::connectCallback>(*this);
        return UVWError{uv_tcp_connect(conn.get(), get<uv_tcp_t>(), reinterpret_cast<const sockaddr *>(&addr), func)};
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    UVWError connect(Addr addr) noexcept {
        return connect<I>(addr.first, addr.second);
    }

    UVWError accept(Tcp &tcp) noexcept {
        return UVWError{uv_accept(get<uv_stream_t>(), tcp.get<uv_stream_t>())};
    }

    explicit operator bool() const noexcept { return initialized; }

private:
    std::unique_ptr<uv_connect_t> conn;
    bool initialized;
};


}
