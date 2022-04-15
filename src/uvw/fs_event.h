#ifndef UVW_FS_EVENT_INCLUDE_H
#define UVW_FS_EVENT_INCLUDE_H

#include <string>
#include <type_traits>
#include <uv.h>
#include "config.h"
#include "enum.hpp"
#include "handle.hpp"
#include "loop.h"
#include "util.h"

namespace uvw {

namespace details {

enum class uvw_fs_event_flags : std::underlying_type_t<uv_fs_event_flags> {
    WATCH_ENTRY = UV_FS_EVENT_WATCH_ENTRY,
    STAT = UV_FS_EVENT_STAT,
    RECURSIVE = UV_FS_EVENT_RECURSIVE,
    _UVW_ENUM = 0
};

enum class uvw_fs_event : std::underlying_type_t<uv_fs_event> {
    RENAME = UV_RENAME,
    CHANGE = UV_CHANGE
};

} // namespace details

/*! @brief Fs event event. */
struct fs_event_event {
    fs_event_event(const char *pathname, details::uvw_fs_event events);

    /**
     * @brief The path to the file being monitored.
     *
     * If the handle was started with a directory, the filename parameter will
     * be a relative path to a file contained in the directory.
     */
    const char *filename;

    /**
     * @brief Detected events all in one.
     *
     * Available flags are:
     *
     * * `fs_event_handle::watch::RENAME`
     * * `fs_event_handle::watch::CHANGE`
     */
    details::uvw_fs_event flags;
};

/**
 * @brief The fs event handle.
 *
 * These handles allow the user to monitor a given path for changes, for
 * example, if the file was renamed or there was a generic change in it. The
 * best backend for the job on each platform is chosen by the handle.
 *
 * To create a `fs_event_handle` through a `loop`, no arguments are required.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/fs_event.html)
 * for further details.
 */
class fs_event_handle final: public handle<fs_event_handle, uv_fs_event_t, fs_event_event> {
    static void start_callback(uv_fs_event_t *hndl, const char *filename, int events, int status);

public:
    using watch = details::uvw_fs_event;
    using event_flags = details::uvw_fs_event_flags;

    using handle::handle;

    /**
     * @brief Initializes the handle.
     * @return Underlying return value.
     */
    int init() final;

    /**
     * @brief Starts watching the specified path.
     *
     * It will watch the specified path for changes.<br/>
     * As soon as a change is observed, a fs_event_event is emitted by the
     * handle.
     *
     * Available flags are:
     *
     * * `fs_event_handle::event_flags::WATCH_ENTRY`
     * * `fs_event_handle::event_flags::STAT`
     * * `fs_event_handle::event_flags::RECURSIVE`
     *
     * @param path The file or directory to be monitored.
     * @param flags Additional flags to control the behavior.
     * @return Underlying return value.
     */
    int start(const std::string &path, event_flags flags = event_flags::_UVW_ENUM);

    /**
     * @brief Stops polling the file descriptor.
     * @return Underlying return value.
     */
    int stop();

    /**
     * @brief Gets the path being monitored.
     * @return The path being monitored, an empty string in case of errors.
     */
    std::string path() noexcept;
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "fs_event.cpp"
#endif

#endif // UVW_FS_EVENT_INCLUDE_H
