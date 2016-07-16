#pragma once


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


namespace details {


class Connect final: public Request<Connect> {
    explicit Connect(std::shared_ptr<Loop> ref)
        : Request{RequestType<uv_connect_t>{}, std::move(ref)}
    { }

public:
    template<typename... Args>
    static std::shared_ptr<Connect> create(Args&&... args) {
        return std::shared_ptr<Connect>{new Connect{std::forward<Args>(args)...}};
    }

    void connect(uv_tcp_t *handle, const sockaddr *addr) {
        exec<uv_connect_t, ConnectEvent>(&uv_tcp_connect, get<uv_connect_t>(), handle, addr);
    }
};


}


class Tcp final: public Stream<Tcp> {
    explicit Tcp(std::shared_ptr<Loop> ref)
        : Stream{HandleType<uv_tcp_t>{}, std::move(ref)}
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

    void noDelay(bool value = false) {
        invoke(&uv_tcp_nodelay, get<uv_tcp_t>(), value ? 1 : 0);
    }

    void keepAlive(bool enable = false, Time time = Time{0}) {
        invoke(&uv_tcp_keepalive, get<uv_tcp_t>(), enable ? 1 : 0, time.count());
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void bind(std::string ip, unsigned int port, bool ipv6only = false) {
        typename Traits::Type addr;
        Traits::AddrFunc(ip.c_str(), port, &addr);
        unsigned int flags = ipv6only ? UV_TCP_IPV6ONLY : 0;
        invoke(&uv_tcp_bind, get<uv_tcp_t>(), reinterpret_cast<const sockaddr *>(&addr), flags);
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void bind(Addr addr, bool ipv6only = false) {
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
    void connect(std::string ip, unsigned int port) {
        typename Traits::Type addr;
        Traits::AddrFunc(ip.c_str(), port, &addr);

        std::weak_ptr<Tcp> weak = this->shared_from_this();

        auto listener = [weak](const auto &event, details::Connect &) {
            auto ptr = weak.lock();
            if(ptr) { ptr->publish(event); }
        };

        auto connect = loop().resource<details::Connect>();
        connect->once<ErrorEvent>(listener);
        connect->once<ConnectEvent>(listener);
        connect->connect(get<uv_tcp_t>(), reinterpret_cast<const sockaddr *>(&addr));
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void connect(Addr addr) {
        connect<I>(addr.ip, addr.port);
    }

    void accept(Tcp &tcp) {
        invoke(&uv_accept, get<uv_stream_t>(), tcp.get<uv_stream_t>());
    }
};


}
