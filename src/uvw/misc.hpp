#pragma once


#include <type_traits>
#include <stdexcept>
#include <cstddef>
#include <utility>
#include <string>
#include <uv.h>
#include "util.hpp"


namespace uvw {


namespace details {


struct IPv4 { };
struct IPv6 { };


template<typename>
struct IpTraits;

template<>
struct IpTraits<IPv4> {
    using Type = sockaddr_in;
    using AddrFuncType = int(*)(const char *, int, sockaddr_in *);
    using NameFuncType = int(*)(const sockaddr_in *, char *, std::size_t);
    static constexpr AddrFuncType addrFunc = &uv_ip4_addr;
    static constexpr NameFuncType nameFunc = &uv_ip4_name;
};

template<>
struct IpTraits<IPv6> {
    using Type = sockaddr_in6;
    using AddrFuncType = int(*)(const char *, int, sockaddr_in6 *);
    using NameFuncType = int(*)(const sockaddr_in6 *, char *, std::size_t);
    static constexpr AddrFuncType addrFunc = &uv_ip6_addr;
    static constexpr NameFuncType nameFunc = &uv_ip6_name;
};


template<typename I>
Addr address(const typename IpTraits<I>::Type *aptr, int len) noexcept {
    std::pair<std::string, unsigned int> addr{};
    char name[len];

    int err = IpTraits<I>::nameFunc(aptr, name, len);

    if(0 == err) {
        addr = { std::string{name}, ntohs(aptr->sin_port) };
    }

    /**
     * See Boost/Mutant idiom:
     *     https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Boost_mutant
     */
    return reinterpret_cast<Addr&>(addr);
}


template<typename I, typename F, typename H>
Addr address(F &&f, const H *handle) noexcept {
    sockaddr_storage ssto;
    int len = sizeof(ssto);
    Addr addr{};

    int err = std::forward<F>(f)(handle, reinterpret_cast<sockaddr *>(&ssto), &len);

    if(0 == err) {
        typename IpTraits<I>::Type *aptr = reinterpret_cast<typename IpTraits<I>::Type *>(&ssto);
        addr = address<I>(aptr, len);
    }

    return addr;
}


template<typename F, typename H, typename..., std::size_t N = 128>
std::string path(F &&f, H *handle) noexcept {
    std::size_t size = N;
    char buf[size];
    std::string str{};
    auto err = std::forward<F>(f)(handle, buf, &size);

    if(UV_ENOBUFS == err) {
        std::unique_ptr<char[]> data{new char[size]};
        err = std::forward<F>(f)(handle, data.get(), &size);

        if(0 == err) {
            str = data.get();
        }
    } else {
        str.assign(buf, size);
    }

    return str;
}


}


}
