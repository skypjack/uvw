#ifndef UVW_UTIL_INCLUDE_H
#define UVW_UTIL_INCLUDE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <uv.h>
#include "config.h"

namespace uvw {

namespace details {

enum class uvw_handle_type : std::underlying_type_t<uv_handle_type> {
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

enum class uvw_clock_id : std::underlying_type_t<uv_clock_id> {
    MONOTONIC = UV_CLOCK_MONOTONIC,
    REALTIME = UV_CLOCK_REALTIME
};

template<typename T>
struct uv_type_wrapper {
    using Type = T;

    constexpr uv_type_wrapper()
        : value{} {}

    constexpr uv_type_wrapper(Type val)
        : value{val} {}

    constexpr operator Type() const noexcept {
        return value;
    }

    bool operator==(uv_type_wrapper other) const noexcept {
        return value == other.value;
    }

private:
    const Type value;
};

template<typename T>
bool operator==(uv_type_wrapper<T> lhs, uv_type_wrapper<T> rhs) {
    return !(lhs == rhs);
}

} // namespace details

/**
 * @brief Windows size representation.
 */
struct win_size {
    int width;  /*!< The _width_ of the given window. */
    int height; /*!< The _height_ of the given window. */
};

using handle_type = details::uvw_handle_type;                     /*!< The type of a handle. */
using handle_category = details::uv_type_wrapper<uv_handle_type>; /*!< Utility class that wraps an internal handle type. */
using file_handle = details::uv_type_wrapper<uv_file>;            /*!< Utility class that wraps an internal file handle. */
using os_socket_handle = details::uv_type_wrapper<uv_os_sock_t>;  /*!< Utility class that wraps an os socket handle. */
using os_file_descriptor = details::uv_type_wrapper<uv_os_fd_t>;  /*!< Utility class that wraps an os file descriptor. */
using pid_type = details::uv_type_wrapper<uv_pid_t>;              /*!< Utility class that wraps a cross platform representation of a pid. */
using clock_id = details::uvw_clock_id;                           /*!< Utility class that wraps a clock source. */

constexpr file_handle std_in{0};  /*!< Placeholder for stdin descriptor. */
constexpr file_handle std_out{1}; /*!< Placeholder for stdout descriptor. */
constexpr file_handle std_err{2}; /*!< Placeholder for stderr descriptor. */

using time_spec = uv_timespec_t; /*!< Library equivalent for uv_timespec_t. */
using file_info = uv_stat_t;     /*!< Library equivalent for uv_stat_t. */
using fs_info = uv_statfs_t;     /*!< Library equivalent for uv_statfs_t. */
using uid_type = uv_uid_t;       /*!< Library equivalent for uv_uid_t. */
using gid_type = uv_gid_t;       /*!< Library equivalent for uv_gid_t. */

using timeval = uv_timeval_t;       /*!< Library equivalent for uv_timeval_t. */
using timeval64 = uv_timeval64_t;   /*!< Library equivalent for uv_timeval64_t. */
using timespec64 = uv_timespec64_t; /*!< Library equivalent for uv_timespec64_t. */
using resource_usage = uv_rusage_t; /*!< Library equivalent for uv_rusage_t. */

/**
 * @brief Utility class.
 *
 * This class can be used to query the subset of the password file entry for the
 * current effective uid (not the real uid).
 *
 * \sa utilities::passwd
 */
struct passwd_info {
    passwd_info(std::shared_ptr<uv_passwd_t> pwd);

    /**
     * @brief Gets the username.
     * @return The username of the current effective uid (not the real uid).
     */
    std::string username() const noexcept;

    /**
     * @brief Gets the uid.
     * @return The current effective uid (not the real uid).
     */
    decltype(uv_passwd_t::uid) uid() const noexcept;

    /**
     * @brief Gets the gid.
     * @return The gid of the current effective uid (not the real uid).
     */
    decltype(uv_passwd_t::gid) gid() const noexcept;

    /**
     * @brief Gets the shell.
     * @return The shell of the current effective uid (not the real uid).
     */
    std::string shell() const noexcept;

    /**
     * @brief Gets the homedir.
     * @return The homedir of the current effective uid (not the real uid).
     */
    std::string homedir() const noexcept;

    /**
     * @brief Checks if the instance contains valid data.
     * @return True if data are all valid, false otherwise.
     */
    operator bool() const noexcept;

private:
    std::shared_ptr<uv_passwd_t> value;
};

/**
 * @brief Utility class.
 *
 * This class can be used to get name and information about the current kernel.
 * The populated data includes the operating system name, release, version, and
 * machine.
 *
 * \sa utilities::uname
 */
struct uts_name {
    uts_name(std::shared_ptr<uv_utsname_t> init);

    /**
     * @brief Gets the operating system name (like "Linux").
     * @return The operating system name.
     */
    std::string sysname() const noexcept;

    /**
     * @brief Gets the operating system release (like "2.6.28").
     * @return The operating system release.
     */
    std::string release() const noexcept;

    /**
     * @brief Gets the operating system version.
     * @return The operating system version
     */
    std::string version() const noexcept;

    /**
     * @brief Gets the hardware identifier.
     * @return The hardware identifier.
     */
    std::string machine() const noexcept;

private:
    std::shared_ptr<uv_utsname_t> uname;
};

/**
 * @brief The IPv4 tag.
 *
 * To be used as template parameter to switch between IPv4 and IPv6.
 */
struct ipv4 {};

/**
 * @brief The IPv6 tag.
 *
 * To be used as template parameter to switch between IPv4 and IPv6.
 */
struct ipv6 {};

/**
 * @brief Address representation.
 */
struct socket_address {
    std::string ip;    /*!< Either an IPv4 or an IPv6. */
    unsigned int port; /*!< A valid service identifier. */
};

/**
 * \brief CPU information.
 */
struct cpu_info {
    using cpu_time = decltype(uv_cpu_info_t::cpu_times);

    std::string model; /*!< The model of the CPU. */
    int speed;         /*!< The frequency of the CPU. */

    /**
     * @brief CPU times.
     *
     * It is built up of the following data members: `user`, `nice`, `sys`,
     * `idle`, `irq`, all of them having type `uint64_t`.
     */
    cpu_time times;
};

/**
 * \brief Interface address.
 */
struct interface_address {
    std::string name;       /*!< The name of the interface (as an example _eth0_). */
    char physical[6];       /*!< The physical address. */
    bool internal;          /*!< True if it is an internal interface (as an example _loopback_), false otherwise. */
    socket_address address; /*!< The address of the given interface. */
    socket_address netmask; /*!< The netmask of the given interface. */
};

namespace details {

static constexpr std::size_t DEFAULT_SIZE = 128;

template<typename F, typename... Args>
std::string try_read(F &&f, Args &&...args) noexcept {
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
    } else if(0 == err) {
        str.assign(buf, size);
    }

    return str;
}

void common_alloc_callback(uv_handle_t *, std::size_t suggested, uv_buf_t *buf);

template<typename Type, auto Alloc>
void common_alloc_callback(uv_handle_t *handle, std::size_t suggested, uv_buf_t *buf) {
    auto [alloc, size] = Alloc(*static_cast<const Type *>(handle->data), suggested);
    *buf = uv_buf_init(alloc, static_cast<unsigned int>(size));
}

sockaddr ip_addr(const char *addr, unsigned int port);
socket_address sock_addr(const sockaddr_in &addr);
socket_address sock_addr(const sockaddr_in6 &addr);
socket_address sock_addr(const sockaddr &addr);
socket_address sock_addr(const sockaddr_storage &storage);

} // namespace details

/**
 * @brief Miscellaneous utilities.
 *
 * Miscellaneous functions that don’t really belong to any other class.
 */
struct utilities {
    using malloc_func_type = void *(*)(size_t);
    using realloc_func_type = void *(*)(void *, size_t);
    using calloc_func_type = void *(*)(size_t, size_t);
    using free_func_type = void (*)(void *);

    /**
     * @brief OS dedicated utilities.
     */
    struct os {
        /**
         * @brief Returns the current process id.
         *
         * See the official
         * [documentation](http://docs.libuv.org/en/v1.x/misc.html#c.uv_os_getpid)
         * for further details.
         *
         * @return The current process id.
         */
        static pid_type pid() noexcept;

        /**
         * @brief Returns the parent process id.
         *
         * See the official
         * [documentation](http://docs.libuv.org/en/v1.x/misc.html#c.uv_os_getppid)
         * for further details.
         *
         * @return The parent process id.
         */
        static pid_type ppid() noexcept;

        /**
         * @brief Gets the current user's home directory.
         *
         * See the official
         * [documentation](http://docs.libuv.org/en/v1.x/misc.html#c.uv_os_homedir)
         * for further details.
         *
         * @return The current user's home directory, an empty string in case of
         * errors.
         */
        static std::string homedir() noexcept;

        /**
         * @brief Gets the temp directory.
         *
         * See the official
         * [documentation](http://docs.libuv.org/en/v1.x/misc.html#c.uv_os_tmpdir)
         * for further details.
         *
         * @return The temp directory, an empty string in case of errors.
         */
        static std::string tmpdir() noexcept;

        /**
         * @brief Retrieves an environment variable.
         * @param name The name of the variable to be retrieved.
         * @return The value of the environment variable, an empty string in
         * case of errors.
         */
        static std::string env(const std::string &name) noexcept;

        /**
         * @brief Creates, updates or deletes an environment variable.
         * @param name The name of the variable to be updated.
         * @param value The value to be used for the variable (an empty string
         * to unset it).
         * @return True in case of success, false otherwise.
         */
        static bool env(const std::string &name, const std::string &value) noexcept;

        /**
         * @brief Retrieves all environment variables and iterates them.
         *
         * Environment variables are passed one at a time to the callback in the
         * form of `std::string_view`s.<br/>
         * The signature of the function call operator must be such that it
         * accepts two parameters, the name and the value of the i-th variable.
         *
         * @tparam Func Type of a function object to which to pass environment
         * variables.
         * @param func A function object to which to pass environment variables.
         * @return True in case of success, false otherwise.
         */
        template<typename Func>
        static std::enable_if_t<std::is_invocable_v<Func, std::string_view, std::string_view>, bool>
        env(Func func) noexcept {
            uv_env_item_t *items = nullptr;
            int count{};

            const bool ret = (uv_os_environ(&items, &count) == 0);

            if(ret) {
                for(int pos = 0; pos < count; ++pos) {
                    func(std::string_view{items[pos].name}, std::string_view{items[pos].value});
                }

                uv_os_free_environ(items, count);
            }

            return ret;
        }

        /**
         * @brief Returns the hostname.
         * @return The hostname, an empty string in case of errors.
         */
        static std::string hostname() noexcept;

        /**
         * @brief Gets name and information about the current kernel.
         *
         * This function can be used to get name and information about the
         * current kernel. The populated data includes the operating system
         * name, release, version, and machine.
         *
         * @return Name and information about the current kernel.
         */
        static uts_name uname() noexcept;

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
        static passwd_info passwd() noexcept;

        /**
         * @brief Retrieves the scheduling priority of a process.
         *
         * The returned value is between -20 (high priority) and 19 (low priority).
         * A value that is out of range is returned in case of errors.
         *
         * @note
         * On Windows, the result won't equal necessarily the exact value of the
         * priority because of a mapping to a Windows priority class.
         *
         * @param pid A valid process id.
         * @return The scheduling priority of the process.
         */
        static int priority(pid_type pid);

        /**
         * @brief Sets the scheduling priority of a process.
         *
         * The returned value range is between -20 (high priority) and 19 (low
         * priority).
         *
         * @note
         * On Windows, the priority is mapped to a Windows priority class. When
         * retrieving the process priority, the result won't equal necessarily the
         * exact value of the priority.
         *
         * @param pid A valid process id.
         * @param prio The scheduling priority to set to the process.
         * @return True in case of success, false otherwise.
         */
        static bool priority(pid_type pid, int prio);
    };

    /**
     * @brief Gets the type of the handle given a category.
     * @param category A properly initialized handle category.
     * @return The actual type of the handle as defined by handle_type
     */
    static handle_type guess_handle(handle_category category) noexcept;

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
     * * `handle_type::UNKNOWN`
     * * `handle_type::PIPE`
     * * `handle_type::TCP`
     * * `handle_type::TTY`
     * * `handle_type::UDP`
     * * `handle_type::FILE`
     */
    static handle_type guess_handle(file_handle file) noexcept;

    /** @brief Gets information about the CPUs on the system.
     *
     * This function can be used to query the underlying system and get a set of
     * descriptors of all the available CPUs.
     *
     * @return A set of descriptors of all the available CPUs.
     */
    static std::vector<cpu_info> cpu() noexcept;

    /**
     * @brief Gets a set of descriptors of all the available interfaces.
     *
     * This function can be used to query the underlying system and get a set of
     * descriptors of all the available interfaces, either internal or not.
     *
     * @return A set of descriptors of all the available interfaces.
     */
    static std::vector<interface_address> interface_addresses() noexcept;

    /**
     * @brief IPv6-capable implementation of
     * [if_indextoname](https://linux.die.net/man/3/if_indextoname).
     *
     * Mapping between network interface names and indexes.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/misc.html#c.uv_if_indextoname)
     * for further details.
     *
     * @param index Network interface index.
     * @return Network interface name.
     */
    static std::string index_to_name(unsigned int index) noexcept;

    /**
     * @brief Retrieves a network interface identifier.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/misc.html#c.uv_if_indextoiid)
     * for further details.
     *
     * @param index Network interface index.
     * @return Network interface identifier.
     */
    static std::string index_to_iid(unsigned int index) noexcept;

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
     * @note
     * There is no protection against changing the allocator multiple times. If
     * the user changes it they are responsible for making sure the allocator is
     * changed while no memory was allocated with the previous allocator, or
     * that they are compatible.
     *
     * @param malloc_func Replacement function for _malloc_.
     * @param realloc_func Replacement function for _realloc_.
     * @param calloc_func Replacement function for _calloc_.
     * @param free_func Replacement function for _free_.
     * @return True in case of success, false otherwise.
     */
    static bool replace_allocator(malloc_func_type malloc_func, realloc_func_type realloc_func, calloc_func_type calloc_func, free_func_type free_func) noexcept;

    /**
     * @brief Gets the load average.
     * @return `[0,0,0]` on Windows (not available), the load average otherwise.
     */
    static std::array<double, 3> load_average() noexcept;

    /**
     * @brief Store the program arguments.
     *
     * Required for getting / setting the process title.
     *
     * @return Arguments that haven't been consumed internally.
     */
    static char **setup_args(int argc, char **argv);

    /**
     * @brief Gets the title of the current process.
     * @return The process title.
     */
    static std::string process_title();

    /**
     * @brief Sets the current process title.
     * @param title The process title to be set.
     * @return True in case of success, false otherwise.
     */
    static bool process_title(const std::string &title);

    /**
     * @brief Gets memory information (in bytes).
     * @return Memory information.
     */
    static uint64_t total_memory() noexcept;

    /**
     * @brief Gets the amount of memory available to the process (in bytes).
     *
     * Gets the amount of memory available to the process based on limits
     * imposed by the OS. If there is no such constraint, or the constraint is
     * unknown, `0` is returned.<br/>
     * Note that it is not unusual for this value to be less than or greater
     * than `totalMemory`.
     *
     * @return Amount of memory available to the process.
     */
    static uint64_t constrained_memory() noexcept;

    /**
     * @brief Gets the amount of free memory still available to the process.
     * @return Amount of free memory still available to the process (in bytes).
     */
    static uint64_t available_memory() noexcept;

    /**
     * @brief Gets the current system uptime.
     * @return The current system uptime or 0 in case of errors.
     */
    static double uptime() noexcept;

    /**
     * @brief Gets the resource usage measures for the current process.
     * @return Resource usage measures, zeroes-filled object in case of errors.
     */
    static resource_usage rusage() noexcept;

    /**
     * @brief Gets the current system time from a high-resolution clock source.
     * @param source Clock source, either real-time or monotonic.
     * @return Current system time from the given high-resolution clock source.
     */
    static timespec64 gettime(clock_id source) noexcept;

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
    static uint64_t hrtime() noexcept;

    /**
     * @brief Gets the executable path.
     * @return The executable path, an empty string in case of errors.
     */
    static std::string path() noexcept;

    /**
     * @brief Gets the current working directory.
     * @return The current working directory, an empty string in case of errors.
     */
    static std::string cwd() noexcept;

    /**
     * @brief Changes the current working directory.
     * @param dir The working directory to be set.
     * @return True in case of success, false otherwise.
     */
    static bool chdir(const std::string &dir) noexcept;

    /**
     * @brief Cross-platform implementation of
     * [`gettimeofday`](https://linux.die.net/man/2/gettimeofday)
     * @return The current time.
     */
    static timeval64 time_of_day() noexcept;

    /**
     * @brief Causes the calling thread to sleep for a while.
     * @param msec Number of milliseconds to sleep.
     */
    static void sleep(unsigned int msec) noexcept;

    /**
     * @brief Returns an estimate of the amount of parallelism a program should
     * use (always a non-zero value).
     * @return Estimate of the amount of parallelism a program should use.
     */
    static unsigned int available_parallelism() noexcept;
};

/**
 * @brief Helper type for visitors.
 * @tparam Func Types of function objects.
 */
template<class... Func>
struct overloaded: Func... {
    using Func::operator()...;
};

/**
 * @brief Deduction guide.
 * @tparam Func Types of function objects.
 */
template<class... Func>
overloaded(Func...) -> overloaded<Func...>;

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "util.cpp"
#endif

#endif // UVW_UTIL_INCLUDE_H
