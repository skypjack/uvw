#pragma once


#include <type_traits>
#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "stream.hpp"
#include "util.hpp"


namespace uvw {


class Udp final: public Stream<Udp> {
    explicit Udp(std::shared_ptr<Loop> ref)
        : Stream{HandleType<uv_udp_t>{}, std::move(ref)}
    { }

public:
    enum class Bind: std::underlying_type_t<uv_udp_flags> {
        IPV6ONLY = UV_UDP_IPV6ONLY,
        REUSEADDR = UV_UDP_REUSEADDR
    };

    template<typename... Args>
    static std::shared_ptr<Udp> create(Args&&... args) {
        return std::shared_ptr<Udp>{new Udp{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_udp_t>(&uv_udp_init); }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void bind(std::string ip, unsigned int port, Flags<Bind> flags = Flags<Bind>{}) {
        typename Traits::Type addr;
        Traits::AddrFunc(ip.c_str(), port, &addr);
        invoke(&uv_udp_bind, get<uv_udp_t>(), reinterpret_cast<const sockaddr *>(&addr), flags);
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    void bind(Addr addr, Flags<Bind> flags = Flags<Bind>{}) {
        bind<I>(addr.ip, addr.port, flags);
    }

    template<typename I, typename..., typename Traits = details::IpTraits<I>>
    Addr address() { return Stream::address<I, uv_udp_t>(uv_udp_getsockname); }

    // TODO uv_udp_set_membership
    // TODO uv_udp_set_multicast_loop
    // TODO uv_udp_set_multicast_ttl
    // TODO uv_udp_set_multicast_interface

    void broadcast(bool enable = false) { invoke(&uv_udp_set_broadcast, get<uv_udp_t>(), enable ? 1 : 0); }
    void ttl(int val) { invoke(&uv_udp_set_ttl, get<uv_udp_t>(), val > 255 ? 255 : val); }

    // TODO uv_udp_send
    // TODO uv_udp_try_send
    // TODO uv_udp_recv_start

    void stop() { invoke(&uv_udp_recv_stop, get<uv_udp_t>()); }
};


}
