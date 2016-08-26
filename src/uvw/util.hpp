#pragma once


#include <type_traits>
#include <algorithm>
#include <stdexcept>
#include <cstddef>
#include <utility>
#include <string>
#include <vector>
#include <uv.h>


namespace uvw {


namespace details {


enum class UVHandleType: std::underlying_type_t<uv_handle_type> {
    UNKNOWN = UV_UNKNOWN_HANDLE,
    ASYNC = UV_ASYNC,
    CHECK = UV_CHECK,
    FS_EVENT = UV_FS_EVENT,
    FS_POLL = UV_FS_POLL,
    HANDLE = UV_HANDLE,
    IDLE = UV_IDLE,
    PIPE = UV_NAMED_PIPE,
    POLL = UV_POLL,
    PREPARE = UV_PREPARE,
    PROCESS = UV_PROCESS,
    STREAM = UV_STREAM,
    TCP = UV_TCP,
    TIMER = UV_TIMER,
    TTY = UV_TTY,
    UDP = UV_UDP,
    SIGNAL = UV_SIGNAL,
    FILE = UV_FILE
};


template<typename T>
struct UVTypeWrapper {
    using Type = T;

    constexpr UVTypeWrapper(Type val): value{val} { }
    constexpr operator Type() const noexcept { return value; }

private:
    const Type value;
};


}


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
 * @brief Windows size representation.
 */
struct WinSize {
    int width; /*!< The _width_ of the given window. */
    int height; /*!< The _height_ of the given window. */
};


using HandleType = details::UVHandleType;

using FileHandle = details::UVTypeWrapper<uv_file>;
using OSSocketHandle = details::UVTypeWrapper<uv_os_sock_t>;
using OSFileDescriptor = details::UVTypeWrapper<uv_os_fd_t>;

using TimeSpec = uv_timespec_t;
using Stat = uv_stat_t;
using Uid = uv_uid_t;
using Gid = uv_gid_t;


/**
 * @brief The IPv4 tag.
 *
 * To be used as template parameter to switch between IPv4 and IPv6.
 */
struct IPv4 { };


/**
 * @brief The IPv6 tag.
 *
 * To be used as template parameter to switch between IPv4 and IPv6.
 */
struct IPv6 { };


/**
 * @brief Address representation.
 */
struct Addr {
    std::string ip; /*!< Either an IPv4 or an IPv6. */
    unsigned int port; /*!< A valid service identifier. */
};


/**
 * \brief CPU information.
 */
struct CPUInfo {
    using CPUTime = decltype(uv_cpu_info_t::cpu_times);

    std::string model; /*!< The model of the CPU. */
    int speed; /*!< The frequency of the CPU. */

    /**
     * @brief CPU times.
     *
     * It is built up of the following data members: `user`, `nice`, `sys`,
     * `idle`, `irq`, all of them having type `uint64_t`.
     */
    CPUTime times;
};


/**
 * \brief Interface address.
 */
struct InterfaceAddress {
    std::string name; /*!< The name of the interface (as an example _eth0_). */
    std::string physical; /*!< The physical address. */
    bool internal; /*!< True if it is an internal interface (as an example _loopback_), false otherwise. */
    Addr address; /*!< The address of the given interface. */
    Addr netmask; /*!< The netmask of the given interface. */
};


namespace details {


template<typename>
struct IpTraits;


template<>
struct IpTraits<IPv4> {
    using Type = sockaddr_in;
    using AddrFuncType = int(*)(const char *, int, Type *);
    using NameFuncType = int(*)(const Type *, char *, std::size_t);
    static constexpr AddrFuncType addrFunc = &uv_ip4_addr;
    static constexpr NameFuncType nameFunc = &uv_ip4_name;
    static constexpr auto sinPort(const Type *addr) { return addr->sin_port; }
};


template<>
struct IpTraits<IPv6> {
    using Type = sockaddr_in6;
    using AddrFuncType = int(*)(const char *, int, Type *);
    using NameFuncType = int(*)(const Type *, char *, std::size_t);
    static constexpr AddrFuncType addrFunc = &uv_ip6_addr;
    static constexpr NameFuncType nameFunc = &uv_ip6_name;
    static constexpr auto sinPort(const Type *addr) { return addr->sin6_port; }
};


template<typename I, typename..., std::size_t N = 128>
Addr address(const typename details::IpTraits<I>::Type *aptr) noexcept {
    Addr addr;
    char name[N];

    int err = details::IpTraits<I>::nameFunc(aptr, name, N);

    if(0 == err) {
        addr.port = ntohs(details::IpTraits<I>::sinPort(aptr));
        addr.ip = std::string{name};
    }

    return addr;
}


template<typename I, typename F, typename H>
Addr address(F &&f, const H *handle) noexcept {
    sockaddr_storage ssto;
    int len = sizeof(ssto);
    Addr addr{};

    int err = std::forward<F>(f)(handle, reinterpret_cast<sockaddr *>(&ssto), &len);

    if(0 == err) {
        typename IpTraits<I>::Type *aptr = reinterpret_cast<typename IpTraits<I>::Type *>(&ssto);
        addr = address<I>(aptr);
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


/**
 * @brief Miscellaneous utilities.
 *
 * Miscellaneous functions that don’t really belong to any other class.
 */
struct Utilities {
    using MallocFuncType = void*(*)(size_t);
    using ReallocFuncType = void*(*)(void*, size_t);
    using CallocFuncType = void*(*)(size_t, size_t);
    using FreeFuncType = void(*)(void*);


    /**
     * @brief Gets the type of the stream to be used with the given descriptor.
     *
     * Returns the type of stream that should be used with a given file
     * descriptor.<br/>
     * Usually this will be used during initialization to guess the type of the
     * stdio streams.
     *
     * @param file A valid descriptor.
     * @return One of the following types:
     *
     * * `HandleType::UNKNOWN`
     * * `HandleType::PIPE`
     * * `HandleType::TCP`
     * * `HandleType::TTY`
     * * `HandleType::UDP`
     * * `HandleType::FILE`
     */
    static HandleType guessHandle(FileHandle file) {
        auto type = uv_guess_handle(file);

        switch(type) {
        case UV_ASYNC:
            return HandleType::ASYNC;
        case UV_CHECK:
            return HandleType::CHECK;
        case UV_FS_EVENT:
            return HandleType::FS_EVENT;
        case UV_FS_POLL:
            return HandleType::FS_POLL;
        case UV_HANDLE:
            return HandleType::HANDLE;
        case UV_IDLE:
            return HandleType::IDLE;
        case UV_NAMED_PIPE:
            return HandleType::PIPE;
        case UV_POLL:
            return HandleType::POLL;
        case UV_PREPARE:
            return HandleType::PREPARE;
        case UV_PROCESS:
            return HandleType::PROCESS;
        case UV_STREAM:
            return HandleType::STREAM;
        case UV_TCP:
            return HandleType::TCP;
        case UV_TIMER:
            return HandleType::TIMER;
        case UV_TTY:
            return HandleType::TTY;
        case UV_UDP:
            return HandleType::UDP;
        case UV_SIGNAL:
            return HandleType::SIGNAL;
        case UV_FILE:
            return HandleType::FILE;
        default:
            return HandleType::UNKNOWN;
        }
    }


    /** @brief Gets information about the CPUs on the system.
     *
     * This function can be used to query the underlying system and get a set of
     * descriptors of all the available CPUs.
     *
     * @return A set of descriptors of all the available CPUs.
     */
    static std::vector<CPUInfo> cpuInfo() noexcept {
        std::vector<CPUInfo> cpuinfos;

        uv_cpu_info_t *infos;
        int count;

        if(0 == uv_cpu_info(&infos, &count)) {
            std::for_each(infos, infos+count, [&cpuinfos](const auto &info) {
                CPUInfo cpuinfo;

                cpuinfo.model = info.model;
                cpuinfo.speed = info.speed;
                cpuinfo.times = info.cpu_times;

                cpuinfos.push_back(std::move(cpuinfo));
            });

            uv_free_cpu_info(infos, count);
        }

        return cpuinfos;
    }


    /**
     * @brief Gets a set of descriptors of all the available interfaces.
     *
     * This function can be used to query the underlying system and get a set of
     * descriptors of all the available interfaces, either internal or not.
     *
     * @return A set of descriptors of all the available interfaces.
     */
    static std::vector<InterfaceAddress> interfaceAddresses() noexcept {
        std::vector<InterfaceAddress> interfaces;

        uv_interface_address_t *ifaces;
        int count;

        if(0 == uv_interface_addresses(&ifaces, &count)) {
            std::for_each(ifaces, ifaces+count, [&interfaces](const auto &iface) {
                InterfaceAddress interface;

                interface.name = iface.name;
                interface.physical = iface.phys_addr;
                interface.internal = iface.is_internal;

                if(iface.address.address4.sin_family == AF_INET) {
                    interface.address = details::address<IPv4>(&iface.address.address4);
                    interface.netmask = details::address<IPv4>(&iface.netmask.netmask4);
                } else if(iface.address.address4.sin_family == AF_INET6) {
                    interface.address = details::address<IPv6>(&iface.address.address6);
                    interface.netmask = details::address<IPv6>(&iface.netmask.netmask6);
                }

                interfaces.push_back(std::move(interface));
            });

            uv_free_interface_addresses(ifaces, count);
        }

        return interfaces;
    }


    /**
     * @brief Override the use of some standard library’s functions.
     *
     * Override the use of the standard library’s memory allocation
     * functions.<br/>
     * This method must be invoked before any other `uvw` function is called or
     * after all resources have been freed and thus the underlying library
     * doesn’t reference any allocated memory chunk.
     *
     * If any of the function pointers is _null_, the invokation will fail.
     *
     * **Note**: there is no protection against changing the allocator multiple
     * times. If the user changes it they are responsible for making sure the
     * allocator is changed while no memory was allocated with the previous
     * allocator, or that they are compatible.
     *
     * @param mallocFunc Replacement function for _malloc_.
     * @param reallocFunc Replacement function for _realloc_.
     * @param callocFunc Replacement function for _calloc_.
     * @param freeFunc Replacement function for _free_.
     * @return True in case of success, false otherwise.
     */
    static bool replaceAllocator(MallocFuncType mallocFunc, ReallocFuncType reallocFunc, CallocFuncType callocFunc, FreeFuncType freeFunc) {
        return (0 == uv_replace_allocator(mallocFunc, reallocFunc, callocFunc, freeFunc));
    }
};


/**
 * TODO
 *
 * * uv_uptime
 * * uv_getrusage
 * * uv_cpu_info
 * * uv_free_cpu_info
 * * uv_loadavg
 * * uv_exepath
 * * uv_cwd
 * * uv_chdir
 * * uv_os_homedir
 * * uv_os_tmpdir
 * * uv_os_get_passwd
 * * uv_os_free_passwd
 * * uv_get_total_memory
 * * uv_hrtime
 */


}
