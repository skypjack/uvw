#pragma once


#include <type_traits>
#include <stdexcept>
#include <cstddef>
#include <utility>
#include <string>
#include <uv.h>


namespace uvw {


/**
 * @brief Utility class to handle flags.
 *
 * This class can be used to handle flags of a same enumeration type.<br/>
 * It is meant to be used as an argument for functions and member methods and
 * as part of events.<br/>
 * `Flags<E>` objects can be easily _or-ed_ and _and-ed_ with other instances of
 * the same type or with instances of the type `E` (that is, the actual flag
 * type), thus converted to the underlying type when needed.
 */
template<typename E>
class Flags final {
    using InnerType = std::underlying_type_t<E>;

    constexpr InnerType toInnerType(E flag) const noexcept { return static_cast<InnerType>(flag); }

public:
    using Type = InnerType;

    /**
     * @brief Constructs a Flags object from a value of the enum `E`.
     * @param flag An value of the enum `E`.
     */
    constexpr Flags(E flag) noexcept: flags{toInnerType(flag)} { }

    /**
     * @brief Constructs a Flags object from an instance of the underlying type
     * of the enum `E`.
     * @param f An instance of the underlying type of the enum `E`.
     */
    constexpr Flags(Type f): flags{f} { }

    /**
     * @brief Constructs an uninitialized Flags object.
     */
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

    /**
     * @brief Or operator.
     * @param f A valid instance of Flags.
     * @return This instance _or-ed_ with `f`.
     */
    constexpr Flags operator|(const Flags &f) const noexcept { return Flags(flags | f.flags); }

    /**
     * @brief Or operator.
     * @param flag A value of the enum `E`.
     * @return This instance _or-ed_ with `flag`.
     */
    constexpr Flags operator|(E flag) const noexcept { return Flags(flags | toInnerType(flag)); }

    /**
     * @brief And operator.
     * @param f A valid instance of Flags.
     * @return This instance _and-ed_ with `f`.
     */
    constexpr Flags operator&(const Flags &f) const noexcept { return Flags(flags & f.flags); }

    /**
     * @brief And operator.
     * @param flag A value of the enum `E`.
     * @return This instance _and-ed_ with `flag`.
     */
    constexpr Flags operator&(E flag) const noexcept { return Flags(flags & toInnerType(flag)); }

    /**
     * @brief Checks if this instance is initialized.
     * @return False if it's uninitialized, true otherwise.
     */
    explicit constexpr operator bool() const noexcept { return !(flags == InnerType{}); }

    /**
     * @brief Casts the instance to the underlying type of `E`.
     * @return An integral representation of the contained flags.
     */
    constexpr operator Type() const noexcept { return flags; }

private:
    InnerType flags;
};


/**
 * @brief Wrapper for underlying library's types.
 *
 * In particular, It is used for:
 *
 * * FileHandle (that is an alias for `UVTypeWrapper<uv_file>`)
 * * OSSocketHandle (that is an alias for `UVTypeWrapper<uv_os_sock_t>`)
 * * OSFileDescriptor (that is an alias for `UVTypeWrapper<uv_os_fd_t>`)
 *
 * It can be bound to each value of type `T` and it will be implicitly converted
 * to the underlying type when needed.
 */
template<typename T>
struct UVTypeWrapper {
    using Type = T;

    /**
     * @brief Constructs a new instance of the wrapper.
     * @param val The value to be stored.
     */
    constexpr UVTypeWrapper(Type val): value{val} { }

    /**
     * @brief Cast operator to the underlying type.
     * @return The stored value.
     */
    constexpr operator Type() const noexcept { return value; }
private:
    const Type value;
};


using FileHandle = UVTypeWrapper<uv_file>;
using OSSocketHandle = UVTypeWrapper<uv_os_sock_t>;
using OSFileDescriptor = UVTypeWrapper<uv_os_fd_t>;


/**
 * @brief Address representation.
 *
 * Pair alias (see Boost/Mutant idiom) used to pack together an ip and a
 * port.<br/>
 * Instead of `first` and `second`, the two parameters are named:
 *
 * * `ip`, that is of type `std::string`
 * * `port`, that is of type `unsigned int`
 */
struct Addr { std::string ip; unsigned int port; };

/**
 * @brief Windows size representation.
 *
 * Pair alias (see Boost/Mutant idiom) used to pack together a width and a
 * height.<br/>
 * Instead of `first` and `second`, the two parameters are named:
 *
 * * `width`, that is of type `int`
 * * `height`, that is of type `int`
 */
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
Addr address(const typename details::IpTraits<I>::Type *aptr, int len) noexcept {
    std::pair<std::string, unsigned int> addr{};
    char name[len];

    int err = details::IpTraits<I>::nameFunc(aptr, name, len);

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
