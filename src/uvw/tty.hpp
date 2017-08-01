#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "stream.hpp"
#include "util.hpp"


namespace uvw {


namespace details {


struct ResetModeMemo {
    ~ResetModeMemo() {
        uv_tty_reset_mode();
    }
};


enum class UVTTYModeT: std::underlying_type_t<uv_tty_mode_t> {
    NORMAL = UV_TTY_MODE_NORMAL,
    RAW = UV_TTY_MODE_RAW,
    IO = UV_TTY_MODE_IO
};


}


/**
 * @brief The TTYHandle handle.
 *
 * TTY handles represent a stream for the console.
 *
 * To create a `TTYHandle` through a `Loop`, arguments follow:
 *
 * * A valid FileHandle. Usually the file descriptor will be:
 *     * `uvw::StdIN` or `0` for `stdin`
 *     * `uvw::StdOUT` or `1` for `stdout`
 *     * `uvw::StdERR` or `2` for `stderr`
 * * A boolean value that specifies the plan on calling `read()` with this
 * stream. Remember that `stdin` is readable, `stdout` is not.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/tty.html#c.uv_tty_init)
 * for further details.
 */
class TTYHandle final: public StreamHandle<TTYHandle, uv_tty_t> {
    static auto resetModeMemo() {
        static std::weak_ptr<details::ResetModeMemo> weak;
        auto shared = weak.lock();
        if(!shared) { weak = shared = std::make_shared<details::ResetModeMemo>(); }
        return shared;
    }

public:
    using Mode = details::UVTTYModeT;

    explicit TTYHandle(ConstructorAccess ca, std::shared_ptr<Loop> ref, FileHandle desc, bool readable)
        : StreamHandle{ca, std::move(ref)},
          memo{resetModeMemo()},
          fd{desc},
          rw{readable}
    {}

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return initialize(&uv_tty_init, fd, rw);
    }

    /**
     * @brief Sets the TTY using the specified terminal mode.
     *
     * Available modes are:
     *
     * * `TTY::Mode::NORMAL`
     * * `TTY::Mode::RAW`
     * * `TTY::Mode::IO`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/tty.html#c.uv_tty_mode_t)
     * for further details.
     *
     * @param m The mode to be set.
     * @return True in case of success, false otherwise.
     */
    bool mode(Mode m) {
        return (0 == uv_tty_set_mode(get(), static_cast<std::underlying_type_t<Mode>>(m)));
    }

    /**
     * @brief Resets TTY settings to default values.
     * @return True in case of success, false otherwise.
     */
    bool reset() noexcept {
        return (0 == uv_tty_reset_mode());
    }

    /**
     * @brief Gets the current Window size.
     * @return The current Window size or `{-1, -1}` in case of errors.
     */
    WinSize getWinSize() {
        WinSize size;

        if(0 != uv_tty_get_winsize(get(), &size.width, &size.height)) {
            size.width = -1;
            size.height = -1;
        }

        return size;
    }

private:
    std::shared_ptr<details::ResetModeMemo> memo;
    FileHandle::Type fd;
    int rw;
};


}
