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
    explicit FsPollEvent(Stat _prev, Stat _curr) noexcept
        : prev{std::move(_prev)}, curr{std::move(_curr)}
    {}

    Stat prev; /*!< The old Stat struct. */
    Stat curr; /*!< The new Stat struct. */
};


/**
 * @brief The FsPollHandle handle.
 *
 * It allows the user to monitor a given path for changes. Unlike FsEventHandle
 * handles, FsPollHandle handles use stat to detect when a file has changed so
 * they can work on file systems where FsEventHandle handles can’t.
 *
 * To create a `FsPollHandle` through a `Loop`, no arguments are required.
 */
class FsPollHandle final: public Handle<FsPollHandle, uv_fs_poll_t> {
    static void startCallback(uv_fs_poll_t *handle, int status, const uv_stat_t *prev, const uv_stat_t *curr) {
        FsPollHandle &fsPoll = *(static_cast<FsPollHandle*>(handle->data));
        if(status) { fsPoll.publish(ErrorEvent{status}); }
        else { fsPoll.publish(FsPollEvent{ *prev, *curr }); }
    }

public:
    using Handle::Handle;

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return initialize(&uv_fs_poll_init);
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
        invoke(&uv_fs_poll_start, get(), &startCallback, file.data(), interval);
    }

    /**
     * @brief Stops the handle.
     */
    void stop() {
        invoke(&uv_fs_poll_stop, get());
    }

    /**
     * @brief Gets the path being monitored by the handle.
     * @return The path being monitored by the handle.
     */
    std::string path() noexcept {
        return details::path(&uv_fs_poll_getpath, get());
    }
};


}
