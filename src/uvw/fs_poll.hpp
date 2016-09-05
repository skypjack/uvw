#pragma once


#include <utility>
#include <string>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"
#include "loop.hpp"


namespace uvw {


/**
 * @brief FsPollEvent event.
 *
 * It will be emitted by FsPollHandle according with its functionalities.
 */
struct FsPollEvent: Event<FsPollEvent> {
    explicit FsPollEvent(const Stat &p, const Stat &c) noexcept
        : prev(p), curr(c)
    { }

    /**
     * @brief Gets the old Stat struct.
     * @return The old Stat struct.
     */
    const Stat & previous() const noexcept { return prev; }

    /**
     * @brief Gets the new Stat struct.
     * @return The new Stat struct.
     */
    const Stat & current() const noexcept { return curr; }

private:
    Stat prev;
    Stat curr;
};


/**
 * @brief The FsPollHandle handle.
 *
 * It allows the user to monitor a given path for changes. Unlike FsEventHandle
 * handles, FsPollHandle handles use stat to detect when a file has changed so
 * they can work on file systems where FsEventHandle handles can’t.
 */
class FsPollHandle final: public Handle<FsPollHandle, uv_fs_poll_t> {
    static void startCallback(uv_fs_poll_t *handle, int status, const uv_stat_t *prev, const uv_stat_t *curr) {
        FsPollHandle &fsPoll = *(static_cast<FsPollHandle*>(handle->data));
        if(status) { fsPoll.publish(ErrorEvent{status}); }
        else { fsPoll.publish(FsPollEvent{ *prev, *curr }); }
    }

    using Handle::Handle;

public:
    /**
     * @brief Creates a new fs poll handle.
     * @param loop A pointer to the loop from which the handle generated.
     * @return A pointer to the newly created handle.
     */
    static std::shared_ptr<FsPollHandle> create(std::shared_ptr<Loop> loop) {
        return std::shared_ptr<FsPollHandle>{new FsPollHandle{std::move(loop)}};
    }

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return initialize<uv_fs_poll_t>(&uv_fs_poll_init);
    }

    /**
     * @brief Starts the handle.
     *
     * The handle will start emitting FsPollEvent when needed.
     *
     * @param file The path to the file to be checked.
     * @param interval Milliseconds between successive checks.
     */
    void start(std::string file, unsigned int interval) {
        invoke(&uv_fs_poll_start, get<uv_fs_poll_t>(), &startCallback, file.data(), interval);
    }

    /**
     * @brief Stops the handle.
     */
    void stop() {
        invoke(&uv_fs_poll_stop, get<uv_fs_poll_t>());
    }

    /**
     * @brief Gets the path being monitored by the handle.
     * @return The path being monitored by the handle.
     */
    std::string path() noexcept {
        return details::path(&uv_fs_poll_getpath, get<uv_fs_poll_t>());
    }
};


}
