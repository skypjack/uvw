#pragma once


#include <type_traits>
#include <algorithm>
#include <stdexcept>
#include <cstddef>
#include <utility>
#include <string>
#include <vector>
#include <array>
#include <uv.h>


#ifdef _WIN32
// MSVC doesn't have C++14 relaxed constexpr support yet. Hence the jugglery.
#define CONSTEXPR_SPECIFIER
#else
#define CONSTEXPR_SPECIFIER constexpr
#endif


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

    constexpr UVTypeWrapper(Type val): value{val} {}
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
     * @brief Utility factory method to pack a set of values all at once.
     * @return A valid instance of Flags instantiated from values `V`.
     */
    template<E... V>
    static CONSTEXPR_SPECIFIER Flags<E> from() {
        auto flags = Flags<E>{};
        int _[] = { 0, (flags = flags | V, 0)... };
        return void(_), flags;
    }

    /**
     * @brief Constructs a Flags object from a value of the enum `E`.
     * @param flag A value of the enum `E`.
     */
    constexpr Flags(E flag) noexcept: flags{toInnerType(flag)} {}

    /**
     * @brief Constructs a Flags object from an instance of the underlying type
     * of the enum `E`.
     * @param f An instance of the underlying type of the enum `E`.
     */
    constexpr Flags(Type f): flags{f} {}

    /**
     * @brief Constructs an uninitialized Flags object.
     */
    constexpr Flags(): flags{} {}

    constexpr Flags(const Flags &f) noexcept: flags{f.flags} {  }
    constexpr Flags(Flags &&f) noexcept: flags{std::move(f.flags)} {  }

    ~Flags() noexcept { static_assert(std::is_enum<E>::value, "!"); }

    CONSTEXPR_SPECIFIER Flags& operator=(const Flags &f) noexcept {
        flags = f.flags;
        return *this;
    }

    CONSTEXPR_SPECIFIER Flags& operator=(Flags &&f) noexcept {
        flags = std::move(f.flags);
        return *this;
    }

    /**
     * @brief Or operator.
     * @param f A valid instance of Flags.
     * @return This instance _or-ed_ with `f`.
     */
    constexpr Flags operator|(const Flags &f) const noexcept { return Flags{flags | f.flags}; }

    /**
     * @brief Or operator.
     * @param flag A value of the enum `E`.
     * @return This instance _or-ed_ with `flag`.
     */
    constexpr Flags operator|(E flag) const noexcept { return Flags{flags | toInnerType(flag)}; }

    /**
     * @brief And operator.
     * @param f A valid instance of Flags.
     * @return This instance _and-ed_ with `f`.
     */
    constexpr Flags operator&(const Flags &f) const noexcept { return Flags{flags & f.flags}; }

    /**
     * @brief And operator.
     * @param flag A value of the enum `E`.
     * @return This instance _and-ed_ with `flag`.
     */
    constexpr Flags operator&(E flag) const noexcept { return Flags{flags & toInnerType(flag)}; }

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

constexpr FileHandle StdIN{0}; /*!< Placeholder for stdin descriptor. */
constexpr FileHandle StdOUT{1}; /*!< Placeholder for stdout descriptor. */
constexpr FileHandle StdERR{2}; /*!< Placeholder for stderr descriptor. */

using TimeSpec = uv_timespec_t;
using Stat = uv_stat_t;
using Uid = uv_uid_t;
using Gid = uv_gid_t;

using TimeVal = uv_timeval_t;
using RUsage = uv_rusage_t;


struct Passwd {
    Passwd(std::shared_ptr<uv_passwd_t> pwd): passwd{pwd} {}

    std::string username() const noexcept { return passwd->username; }
    auto uid() const noexcept { return passwd->uid; }
    auto gid() const noexcept { return passwd->gid; }
    std::string shell() const noexcept { return passwd->shell; }
    std::string homedir() const noexcept { return passwd->homedir; }

private:
    std::shared_ptr<uv_passwd_t> passwd;
};


/**
 * @brief The IPv4 tag.
 *
 * To be used as template parameter to switch between IPv4 and IPv6.
 */
struct IPv4 {};


/**
 * @brief The IPv6 tag.
 *
 * To be used as template parameter to switch between IPv4 and IPv6.
 */
struct IPv6 {};


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
    char physical[6]; /*!< The physical address. */
    bool internal; /*!< True if it is an internal interface (as an example _loopback_), false otherwise. */
    Addr address; /*!< The address of the given interface. */
    Addr netmask; /*!< The netmask of the given interface. */
};


namespace details {


static constexpr std::size_t DEFAULT_SIZE = 128;


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


template<typename I>
Addr address(const typename details::IpTraits<I>::Type *aptr) noexcept {
    Addr addr;
    char name[DEFAULT_SIZE];

    int err = details::IpTraits<I>::nameFunc(aptr, name, DEFAULT_SIZE);

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


template<typename F, typename... Args>
std::string path(F &&f, Args&&... args) noexcept {
    std::size_t size = DEFAULT_SIZE;
    char buf[DEFAULT_SIZE];
    std::string str{};
    auto err = std::forward<F>(f)(args..., buf, &size);

    if(UV_ENOBUFS == err) {
        std::unique_ptr<char[]> data{new char[size]};
        err = std::forward<F>(f)(args..., data.get(), &size);

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

    struct OS {
        /**
         * @brief Gets the current user's home directory.
         *
         * See the official
         * [documentation](http://docs.libuv.org/en/v1.x/misc.html#c.uv_os_homedir)
         * for further details.
         *
         * @return The current user's home directory.
         */
        static std::string homedir() noexcept {
            return details::path(&uv_os_homedir);
        }

        /**
         * @brief Gets the temp directory.
         *
         * See the official
         * [documentation](http://docs.libuv.org/en/v1.x/misc.html#c.uv_os_tmpdir)
         * for further details.
         *
         * @return The temp directory.
         */
        static std::string tmpdir() noexcept {
            return details::path(&uv_os_tmpdir);
        }

        /**
         * @brief Gets a subset of the password file entry.
         *
         * This function can be used to get the subset of the password file
         * entry for the current effective uid (not the real uid).
         *
         * See the official
         * [documentation](http://docs.libuv.org/en/v1.x/misc.html#c.uv_os_get_passwd)
         * for further details.
         *
         * @return The accessible subset of the password file entry.
         */
        static Passwd passwd() noexcept {
            auto deleter = [](uv_passwd_t *passwd){
                uv_os_free_passwd(passwd);
                delete passwd;
            };

            std::shared_ptr<uv_passwd_t> ptr{new uv_passwd_t, std::move(deleter)};
            uv_os_get_passwd(ptr.get());
            return ptr;
        }
    };

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
    static HandleType guessHandle(FileHandle file) noexcept {
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
                cpuinfos.push_back({ info.model, info.speed, info.cpu_times });
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

        uv_interface_address_t *ifaces{nullptr};
        int count{0};

        if(0 == uv_interface_addresses(&ifaces, &count)) {
            std::for_each(ifaces, ifaces+count, [&interfaces](const auto &iface) {
                InterfaceAddress interfaceAddress;

                interfaceAddress.name = iface.name;
                std::copy(iface.phys_addr, (iface.phys_addr+6), interfaceAddress.physical);
                interfaceAddress.internal = iface.is_internal == 0 ? false : true;

                if(iface.address.address4.sin_family == AF_INET) {
                    interfaceAddress.address = details::address<IPv4>(&iface.address.address4);
                    interfaceAddress.netmask = details::address<IPv4>(&iface.netmask.netmask4);
                } else if(iface.address.address4.sin_family == AF_INET6) {
                    interfaceAddress.address = details::address<IPv6>(&iface.address.address6);
                    interfaceAddress.netmask = details::address<IPv6>(&iface.netmask.netmask6);
                }

                interfaces.push_back(std::move(interfaceAddress));
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
    static bool replaceAllocator(MallocFuncType mallocFunc, ReallocFuncType reallocFunc, CallocFuncType callocFunc, FreeFuncType freeFunc) noexcept {
        return (0 == uv_replace_allocator(mallocFunc, reallocFunc, callocFunc, freeFunc));
    }

    /**
     * @brief Gets the load average.
     * @return `[0,0,0]` on Windows (not available), the load average otherwise.
     */
    static std::array<double, 3> loadAverage() noexcept {
        std::array<double, 3> avg;
        uv_loadavg(avg.data());
        return avg;
    }

    /**
     * @brief Gets memory information (in bytes).
     * @return Memory information.
     */
    static uint64_t totalMemory() noexcept {
        return uv_get_total_memory();
    }

    /**
     * @brief Gets the current system uptime.
     * @return The current system uptime or 0 in case of errors.
     */
    static double uptime() noexcept {
        double ret;

        if(0 != uv_uptime(&ret)) {
            ret = 0;
        }

        return ret;
    }

    /**
     * @brief Gets the resource usage measures for the current process.
     * @return Resource usage measures, zeroes-filled object in case of errors.
     */
    static RUsage rusage() noexcept {
        RUsage ru;
        auto err = uv_getrusage(&ru);
        return err ? RUsage{} : ru;
    }

    /**
     * @brief Gets the current high-resolution real time.
     *
     * The time is expressed in nanoseconds. It is relative to an arbitrary time
     * in the past. It is not related to the time of the day and therefore not
     * subject to clock drift. The primary use is for measuring performance
     * between interval.
     *
     * @return The current high-resolution real time.
     */
    static uint64_t hrtime() noexcept {
        return uv_hrtime();
    }

    /**
     * @brief Gets the executable path.
     * @return The executable path, an empty string in case of errors.
     */
    static std::string exepath() noexcept {
        return details::path(&uv_exepath);
    }

    /**
     * @brief Gets the current working directory.
     * @return The current working directory, an empty string in case of errors.
     */
    static std::string cwd() noexcept {
        return details::path(&uv_cwd);
    }

    /**
     * @brief Changes the current working directory.
     * @param dir The working directory to be set.
     * @return True in case of success, false otherwise.
     */
    static bool chdir(const std::string &dir) noexcept {
        return (0 == uv_chdir(dir.data()));
    }
};


}
