#ifndef UVW_FS_POLL_INCLUDE_H
#define UVW_FS_POLL_INCLUDE_H

#include <chrono>
#include <string>
#include <uv.h>
#include "config.h"
#include "handle.hpp"
#include "loop.h"
#include "util.h"

namespace uvw {

/*! @brief Fs pos event. */
struct fs_poll_event {
    explicit fs_poll_event(file_info previous, file_info current) noexcept;

    file_info prev; /*!< The old file_info struct. */
    file_info curr; /*!< The new file_info struct. */
};

/**
 * @brief The fs poll handle.
 *
 * It allows users to monitor a given path for changes. Unlike fs_event_handle,
 * fs_poll_handle uses stat to detect when a file has changed so it can work on
 * file systems where fs_event_handle handles can’t.
 *
 * To create a `fs_poll_handle` through a `loop`, no arguments are required.
 */
class fs_poll_handle final: public handle<fs_poll_handle, uv_fs_poll_t, fs_poll_event> {
    static void start_callback(uv_fs_poll_t *hndl, int status, const uv_stat_t *prev, const uv_stat_t *curr);

public:
    using time = std::chrono::duration<unsigned int, std::milli>;

    using handle::handle;

    /**
     * @brief Initializes the handle.
     * @return Underlying return value.
     */
    int init() final;

    /**
     * @brief Starts the handle.
     *
     * The handle will start emitting fs_poll_event when needed.
     *
     * @param file The path to the file to be checked.
     * @param interval Milliseconds between successive checks.
     * @return Underlying return value.
     */
    int start(const std::string &file, time interval);

    /**
     * @brief Stops the handle.
     * @return Underlying return value.
     */
    int stop();

    /**
     * @brief Gets the path being monitored by the handle.
     * @return The path being monitored by the handle, an empty string in case
     * of errors.
     */
    std::string path() noexcept;
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "fs_poll.cpp"
#endif

#endif // UVW_FS_POLL_INCLUDE_H
