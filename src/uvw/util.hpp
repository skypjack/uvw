#pragma once


#include <type_traits>
#include <stdexcept>
#include <cstddef>
#include <utility>
#include <string>
#include <uv.h>


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
    static const AddrFuncType AddrFunc;
    static const NameFuncType NameFunc;
};

template<>
struct IpTraits<IPv6> {
    using Type = sockaddr_in6;
    using AddrFuncType = int(*)(const char *, int, sockaddr_in6 *);
    using NameFuncType = int(*)(const sockaddr_in6 *, char *, std::size_t);
    static const AddrFuncType AddrFunc;
    static const NameFuncType NameFunc;
};

const IpTraits<IPv4>::AddrFuncType IpTraits<IPv4>::AddrFunc = uv_ip4_addr;
const IpTraits<IPv6>::AddrFuncType IpTraits<IPv6>::AddrFunc = uv_ip6_addr;
const IpTraits<IPv4>::NameFuncType IpTraits<IPv4>::NameFunc = uv_ip4_name;
const IpTraits<IPv6>::NameFuncType IpTraits<IPv6>::NameFunc = uv_ip6_name;


}


template<typename E>
class Flags final {
    using InnerType = std::underlying_type_t<E>;

    constexpr InnerType toInnerType(E flag) const noexcept { return static_cast<InnerType>(flag); }

public:
    using Type = InnerType;

    constexpr Flags(E flag) noexcept: flags{toInnerType(flag)} { }
    constexpr Flags(Type f): flags{f} { }
    constexpr Flags(): flags{} { }

    constexpr Flags(const Flags &f) noexcept: flags{f.flags} {  }
    constexpr Flags(Flags &&f) noexcept: flags{std::move(f.flags)} {  }

    ~Flags() noexcept { static_assert(std::is_enum<E>::value, "!"); }

    constexpr Flags operator|(const Flags& f) const noexcept { return Flags(flags | f.flags); }
    constexpr Flags operator|(E flag) const noexcept { return Flags(flags | toInnerType(flag)); }

    constexpr Flags operator&(const Flags& f) const noexcept { return Flags(flags & f.flags); }
    constexpr Flags operator&(E flag) const noexcept { return Flags(flags & toInnerType(flag)); }

    constexpr operator bool() const noexcept { return !(flags == InnerType{}); }
    constexpr operator Type() const noexcept { return flags; }

private:
    InnerType flags;
};


struct FileDescriptor {
    using Type = uv_file;

    constexpr FileDescriptor(Type desc): fd{desc} { }

    constexpr operator Type() const noexcept { return fd; }

private:
    const Type fd;
};


static constexpr auto STDIN = FileDescriptor{0};
static constexpr auto STDOUT = FileDescriptor{1};
static constexpr auto STDERR = FileDescriptor{2};


/**
 * See Boost/Mutant idiom:
 *     https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Boost_mutant
 */
struct Addr { std::string ip; unsigned int port; };
struct WinSize { int width; int height; };


using TimeSpec = uv_timespec_t;
using Stat = uv_stat_t;


}
