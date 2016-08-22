#pragma once


#include <utility>
#include <string>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "misc.hpp"


namespace uvw {


namespace details {


enum class UVFsEventFlags: std::underlying_type_t<uv_fs_event_flags> {
    WATCH_ENTRY = UV_FS_EVENT_WATCH_ENTRY,
    STAT = UV_FS_EVENT_STAT,
    RECURSIVE = UV_FS_EVENT_RECURSIVE
};


enum class UVFsEvent: std::underlying_type_t<uv_fs_event> {
    RENAME = UV_RENAME,
    CHANGE = UV_CHANGE
};


}


/**
 * @brief FsEventEvent event.
 *
 * It will be emitted by FsEventHandle according with its functionalities.
 */
struct FsEventEvent: Event<FsEventEvent> {
    FsEventEvent(std::string fPath, Flags<details::UVFsEvent> f)
        : flgs{std::move(f)}, relPath{std::move(fPath)}
    { }

    /**
     * @brief Gets the path to the file being monitored.
     *
     * If the handle was started with a directory, the filename parameter will
     * be a relative path to a file contained in the directory.
     *
     * @return The path to the file being monitored.
     */
    const char * filename() const noexcept { return relPath.data(); }

    /**
     * @brief Gets the detected events.
     *
     * Available flags are:
     *
     * * FsEventHandle::Watch::RENAME
     * * FsEventHandle::Watch::CHANGE
     *
     * @return Detected events all in one.
     */
    Flags<details::UVFsEvent> flags() const noexcept { return flgs; }

private:
    Flags<details::UVFsEvent> flgs;
    std::string relPath;
};


/**
 * @brief The FsEventHandle handle.
 *
 * These handles allow the user to monitor a given path for changes, for
 * example, if the file was renamed or there was a generic change in it. The
 * best backend for the job on each platform is chosen by the handle.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/fs_event.html)
 * for further details.
 */
class FsEventHandle final: public Handle<FsEventHandle, uv_fs_event_t> {
    static void startCallback(uv_fs_event_t *handle, const char *filename, int events, int status) {
        FsEventHandle &fsEvent = *(static_cast<FsEventHandle*>(handle->data));
        if(status) { fsEvent.publish(ErrorEvent{status}); }
        else { fsEvent.publish(FsEventEvent{filename, static_cast<std::underlying_type_t<Event>>(events)}); }
    }

    using Handle::Handle;

public:
    using Watch = details::UVFsEvent;
    using Event = details::UVFsEventFlags;

    /**
     * @brief Creates a new fs event handle.
     * @param args A pointer to the loop from which the handle generated.
     * @return A pointer to the newly created handle.
     */
    template<typename... Args>
    static std::shared_ptr<FsEventHandle> create(Args&&... args) {
        return std::shared_ptr<FsEventHandle>{new FsEventHandle{std::forward<Args>(args)...}};
    }

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return initialize<uv_fs_event_t>(&uv_fs_event_init);
    }

    /**
     * @brief Starts watching the specified path.
     *
     * It will watch the specified path for changes.<br/>
     * As soon as a change is observed, a FsEventEvent is emitted by the
     * handle.<br>
     * It could happen that ErrorEvent events are emitted while running.
     *
     * Available flags are:
     *
     * * FsEventHandle::Event::WATCH_ENTRY
     * * FsEventHandle::Event::STAT
     * * FsEventHandle::Event::RECURSIVE
     *
     * @param path The file or directory to be monitored.
     * @param flags Additional flags to control the behavior.
     */
    void start(std::string path, Flags<Watch> flags = Flags<Watch>{}) {
        invoke(&uv_fs_event_start, get<uv_fs_event_t>(), &startCallback, path.data(), flags);
    }

    /**
     * @brief Starts watching the specified path.
     *
     * It will watch the specified path for changes.<br/>
     * As soon as a change is observed, a FsEventEvent is emitted by the
     * handle.<br>
     * It could happen that ErrorEvent events are emitted while running.
     *
     * Available flags are:
     *
     * * FsEventHandle::Event::WATCH_ENTRY
     * * FsEventHandle::Event::STAT
     * * FsEventHandle::Event::RECURSIVE
     *
     * @param path The file or directory to be monitored.
     * @param watch Additional flag to control the behavior.
     */
    void start(std::string path, Watch watch) {
        start(std::move(path), Flags<Watch>{watch});
    }

    /**
     * @brief Stops polling the file descriptor.
     */
    void stop() {
        invoke(&uv_fs_event_stop, get<uv_fs_event_t>());
    }

    /**
     * @brief Gets the path being monitored.
     * @return The path being monitored.
     */
    std::string path() noexcept {
        return details::path(&uv_fs_event_getpath, get<uv_fs_event_t>());
    }
};


}
