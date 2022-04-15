#ifndef UVW_PROCESS_INCLUDE_H
#define UVW_PROCESS_INCLUDE_H

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <uv.h>
#include "config.h"
#include "enum.hpp"
#include "handle.hpp"
#include "loop.h"
#include "stream.h"
#include "util.h"

namespace uvw {

namespace details {

enum class uvw_process_flags : std::underlying_type_t<uv_process_flags> {
    SETUID = UV_PROCESS_SETUID,
    SETGID = UV_PROCESS_SETGID,
    WINDOWS_VERBATIM_ARGUMENTS = UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS,
    DETACHED = UV_PROCESS_DETACHED,
    WINDOWS_HIDE = UV_PROCESS_WINDOWS_HIDE,
    WINDOWS_HIDE_CONSOLE = UV_PROCESS_WINDOWS_HIDE_CONSOLE,
    WINDOWS_HIDE_GUI = UV_PROCESS_WINDOWS_HIDE_GUI,
    _UVW_ENUM = 0
};

enum class uvw_stdio_flags : std::underlying_type_t<uv_stdio_flags> {
    IGNORE_STREAM = UV_IGNORE,
    CREATE_PIPE = UV_CREATE_PIPE,
    INHERIT_FD = UV_INHERIT_FD,
    INHERIT_STREAM = UV_INHERIT_STREAM,
    READABLE_PIPE = UV_READABLE_PIPE,
    WRITABLE_PIPE = UV_WRITABLE_PIPE,
    OVERLAPPED_PIPE = UV_OVERLAPPED_PIPE,
    _UVW_ENUM = 0
};

} // namespace details

/*! @brief Exit event. */
struct exit_event {
    explicit exit_event(int64_t code, int sig) noexcept;

    int64_t status; /*!< The exit status. */
    int signal;     /*!< The signal that caused the process to terminate, if any. */
};

/**
 * @brief The process handle.
 *
 * Process handles will spawn a new process and allow the user to control it and
 * establish communication channels with it using streams.
 */
class process_handle final: public handle<process_handle, uv_process_t, exit_event> {
    static void exit_callback(uv_process_t *hndl, int64_t exit_status, int term_signal);

public:
    using process_flags = details::uvw_process_flags;
    using stdio_flags = details::uvw_stdio_flags;

    process_handle(loop::token token, std::shared_ptr<loop> ref);

    /**
     * @brief Disables inheritance for file descriptors/handles.
     *
     * Disables inheritance for file descriptors/handles that this process
     * inherited from its parent. The effect is that child processes spawned by
     * this process don’t accidentally inherit these handles.<br/>
     * It is recommended to call this function as early in your program as
     * possible, before the inherited file descriptors can be closed or
     * duplicated.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/process.html#c.uv_disable_stdio_inheritance)
     * for further details.
     */
    static void disable_stdio_inheritance() noexcept;

    /**
     * @brief kill Sends the specified signal to the given PID.
     * @param pid A valid process id.
     * @param signum A valid signal identifier.
     * @return True in case of success, false otherwise.
     */
    static bool kill(int pid, int signum) noexcept;

    /**
     * @brief Initializes the handle.
     * @return Underlying return value.
     */
    int init() final;

    /**
     * @brief spawn Starts the process.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/process.html)
     * for further details.
     *
     * @param file Path pointing to the program to be executed.
     * @param args Command line arguments.
     * @param env Optional environment for the new process.
     * @return Underlying return value.
     */
    int spawn(const char *file, char **args, char **env = nullptr);

    /**
     * @brief Sends the specified signal to the internal process handle.
     * @param signum A valid signal identifier.
     * @return Underlying return value.
     */
    int kill(int signum);

    /**
     * @brief Gets the PID of the spawned process.
     *
     * It’s set after calling `spawn()`.
     *
     * @return The PID of the spawned process.
     */
    int pid() noexcept;

    /**
     * @brief Sets the current working directory for the subprocess.
     * @param path The working directory to be used when `spawn()` is invoked.
     * @return A reference to this process handle.
     */
    process_handle &cwd(const std::string &path) noexcept;

    /**
     * @brief Sets flags that control how `spawn()` behaves.
     *
     * Available flags are:
     *
     * * `process_handle::process_flags::SETUID`
     * * `process_handle::process_flags::SETGID`
     * * `process_handle::process_flags::WINDOWS_VERBATIM_ARGUMENTS`
     * * `process_handle::process_flags::DETACHED`
     * * `process_handle::process_flags::WINDOWS_HIDE`
     * * `process_handle::process_flags::WINDOWS_HIDE_CONSOLE`
     * * `process_handle::process_flags::WINDOWS_HIDE_GUI`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/process.html#c.uv_process_flags)
     * for further details.
     *
     * @param flags A valid set of flags.
     * @return A reference to this process handle.
     */
    process_handle &flags(process_flags flags) noexcept;

    /**
     * @brief Makes a `stdio` handle available to the child process.
     *
     * Available flags are:
     *
     * * `process_handle::stdio_flags::IGNORE_STREAM`
     * * `process_handle::stdio_flags::CREATE_PIPE`
     * * `process_handle::stdio_flags::INHERIT_FD`
     * * `process_handle::stdio_flags::INHERIT_STREAM`
     * * `process_handle::stdio_flags::READABLE_PIPE`
     * * `process_handle::stdio_flags::WRITABLE_PIPE`
     * * `process_handle::stdio_flags::OVERLAPPED_PIPE`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/process.html#c.uv_stdio_flags)
     * for further details.
     *
     * @param stream A valid `stdio` handle.
     * @param flags A valid set of flags.
     * @return A reference to this process handle.
     */
    template<typename T, typename U, typename... E>
    process_handle &stdio(stream_handle<T, U, E...> &stream, stdio_flags flags) {
        uv_stdio_container_t container;
        container.flags = static_cast<uv_stdio_flags>(flags);
        container.data.stream = reinterpret_cast<uv_stream_t *>(stream.raw());
        po_stream_stdio.push_back(std::move(container));
        return *this;
    }

    /**
     * @brief Makes a file descriptor available to the child process.
     *
     * Available flags are:
     *
     * * `process_handle::stdio_flags::IGNORE_STREAM`
     * * `process_handle::stdio_flags::CREATE_PIPE`
     * * `process_handle::stdio_flags::INHERIT_FD`
     * * `process_handle::stdio_flags::INHERIT_STREAM`
     * * `process_handle::stdio_flags::READABLE_PIPE`
     * * `process_handle::stdio_flags::WRITABLE_PIPE`
     * * `process_handle::stdio_flags::OVERLAPPED_PIPE`
     *
     * Default file descriptors are:
     *     * `uvw::std_in` for `stdin`
     *     * `uvw::std_out` for `stdout`
     *     * `uvw::std_err` for `stderr`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/process.html#c.uv_stdio_flags)
     * for further details.
     *
     * @param fd A valid file descriptor.
     * @param flags A valid set of flags.
     * @return A reference to this process handle.
     */
    process_handle &stdio(file_handle fd, stdio_flags flags);

    /**
     * @brief Sets the child process' user id.
     * @param id A valid user id to be used.
     * @return A reference to this process handle.
     */
    process_handle &uid(uid_type id);

    /**
     * @brief Sets the child process' group id.
     * @param id A valid group id to be used.
     * @return A reference to this process handle.
     */
    process_handle &gid(gid_type id);

private:
    std::string po_cwd;
    process_flags po_flags;
    std::vector<uv_stdio_container_t> po_fd_stdio;
    std::vector<uv_stdio_container_t> po_stream_stdio;
    uid_type po_uid;
    gid_type po_gid;
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "process.cpp"
#endif

#endif // UVW_PROCESS_INCLUDE_H
