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
    static void connectCallback(uv_connect_t *req, int status) {
        // TODO migrate to Request (see request.hpp for further details)
        Tcp &tcp = *(static_cast<Tcp*>(req->handle->data));
        if(status) { tcp.publish(ErrorEvent{status}); }
        else { tcp.publish(ConnectEvent{}); }
    }

    explicit Tcp(std::shared_ptr<Loop> ref)
        // TODO migrate to Request (see request.hpp for further details)
        : Stream{ResourceType<uv_tcp_t>{}, std::move(ref)},
          conn{std::make_unique<uv_connect_t>()}
    { }

    template<typename I, typename F, typename..., typename Traits = details::IpTraits<I>>
    Addr address(F &&f) {
        sockaddr_storage ssto;
        int len = sizeof(ssto);
        char name[sizeof(ssto)];
        std::pair<std::string, unsigned int> addr{ "", 0 };

        int err = std::forward<F>(f)(get<uv_tcp_t>(), reinterpret_cast<sockaddr *>(&ssto), &len);

        if(!err) {
            typename Traits::Type *aptr = reinterpret_cast<typename Traits::Type *>(&ssto);
            err = Traits::NameFunc(aptr, name, len);

            if(!err) {
                addr = { std::string{name}, ntohs(aptr->sin_port) };
            }
        }

        /**
         * See Boost/Mutant idiom:
         *     https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Boost_mutant
         */
        return reinterpret_cast<Addr&>(addr);
    }

public:
    using Time = std::chrono::seconds;

    using IPv4 = details::IPv4;
    using IPv6 = details::IPv6;

    template<typename... Args>
    static std::shared_ptr<Tcp> create(Args&&... args) {
        return std::shared_ptr<Tcp>{new Tcp{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_tcp_t>(&uv_tcp_init); }

    void noDelay(bool value = false) noexcept {
        invoke(&uv_tcp_nodelay, get<uv_tcp_t>(), value ? 1 : 0);
    }

    void keepAlive(bool enable = false, Time time = Time{0}) noexcept {
        invoke(&uv_tcp_keepalive, get<uv_tcp_t>(), enable ? 1 : 0, time.count());
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void bind(std::string ip, unsigned int port, bool ipv6only = false) noexcept {
        typename Traits::Type addr;
        Traits::AddrFunc(ip.c_str(), port, &addr);
        unsigned int flags = ipv6only ? UV_TCP_IPV6ONLY : 0;
        invoke(&uv_tcp_bind, get<uv_tcp_t>(), reinterpret_cast<const sockaddr *>(&addr), flags);
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void bind(Addr addr, bool ipv6only = false) noexcept {
        bind<I>(addr.ip, addr.port, ipv6only);
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    Addr address() {
        return address<I>(uv_tcp_getsockname);
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    Addr remote() {
        return address<I>(uv_tcp_getpeername);
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void connect(std::string ip, unsigned int port) noexcept {
        typename Traits::Type addr;
        Traits::AddrFunc(ip.c_str(), port, &addr);
        invoke(&uv_tcp_connect, conn.get(), get<uv_tcp_t>(), reinterpret_cast<const sockaddr *>(&addr), &connectCallback);
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void connect(Addr addr) noexcept {
        connect<I>(addr.ip, addr.port);
    }

    void accept(Tcp &tcp) noexcept {
        invoke(&uv_accept, get<uv_stream_t>(), tcp.get<uv_stream_t>());
    }

private:
    // TODO migrate to Request (see request.hpp for further details)
    std::unique_ptr<uv_connect_t> conn;
};


}
