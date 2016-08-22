#pragma once


#include <type_traits>
#include <stdexcept>
#include <cstddef>
#include <utility>
#include <string>
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


using HandleType = details::UVHandleType;

using FileHandle = details::UVTypeWrapper<uv_file>;
using OSSocketHandle = details::UVTypeWrapper<uv_os_sock_t>;
using OSFileDescriptor = details::UVTypeWrapper<uv_os_fd_t>;

using TimeSpec = uv_timespec_t;
using Stat = uv_stat_t;
using Uid = uv_uid_t;
using Gid = uv_gid_t;


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
HandleType guessHandle(FileHandle file) {
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


/**
 * TODO
 *
 * * uv_replace_allocator
 * * uv_uptime
 * * uv_getrusage
 * * uv_cpu_info
 * * uv_free_cpu_info
 * * uv_interface_addresses
 * * uv_free_interface_addresses
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
