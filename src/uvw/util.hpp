#pragma once


#include <type_traits>
#include <stdexcept>
#include <cstddef>
#include <utility>
#include <string>
#include <uv.h>


namespace uvw {


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

    constexpr Flags& operator=(const Flags &f) noexcept {
        flags = f.flags;
        return *this;
    }

    constexpr Flags& operator=(Flags &&f) noexcept {
        flags = std::move(f.flags);
        return *this;
    }

    constexpr Flags operator|(const Flags &f) const noexcept { return Flags(flags | f.flags); }
    constexpr Flags operator|(E flag) const noexcept { return Flags(flags | toInnerType(flag)); }

    constexpr Flags operator&(const Flags &f) const noexcept { return Flags(flags & f.flags); }
    constexpr Flags operator&(E flag) const noexcept { return Flags(flags & toInnerType(flag)); }

    explicit constexpr operator bool() const noexcept { return !(flags == InnerType{}); }
    constexpr operator Type() const noexcept { return flags; }

private:
    InnerType flags;
};


template<typename T>
struct UVTypeWrapper {
    using Type = T;
    constexpr UVTypeWrapper(Type val): value{val} { }
    constexpr operator Type() const noexcept { return value; }
private:
    const Type value;
};


using FileHandle = UVTypeWrapper<uv_file>;
using OSSocketHandle = UVTypeWrapper<uv_os_sock_t>;
using OSFileDescriptor = UVTypeWrapper<uv_os_fd_t>;


static constexpr auto STDIN = FileHandle{0};
static constexpr auto STDOUT = FileHandle{1};
static constexpr auto STDERR = FileHandle{2};


struct Addr { std::string ip; unsigned int port; };
struct WinSize { int width; int height; };


using TimeSpec = uv_timespec_t;
using Stat = uv_stat_t;
using Uid = uv_uid_t;
using Gid = uv_gid_t;


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
    static constexpr AddrFuncType AddrFunc = &uv_ip4_addr;
    static constexpr NameFuncType NameFunc = &uv_ip4_name;
};

template<>
struct IpTraits<IPv6> {
    using Type = sockaddr_in6;
    using AddrFuncType = int(*)(const char *, int, sockaddr_in6 *);
    using NameFuncType = int(*)(const sockaddr_in6 *, char *, std::size_t);
    static constexpr AddrFuncType AddrFunc = &uv_ip6_addr;
    static constexpr NameFuncType NameFunc = &uv_ip6_name;
};


template<typename I>
Addr address(const typename details::IpTraits<I>::Type *aptr, int len) noexcept {
    std::pair<std::string, unsigned int> addr{};
    char name[len];

    int err = details::IpTraits<I>::NameFunc(aptr, name, len);

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
        typename details::IpTraits<I>::Type *aptr = reinterpret_cast<typename details::IpTraits<I>::Type *>(&ssto);
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
