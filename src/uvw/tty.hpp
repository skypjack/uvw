#pragma once


#include <utility>
#include <cstddef>
#include <memory>
#include <uv.h>
#include "event.hpp"
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
 */
class TTYHandle final: public StreamHandle<TTYHandle, uv_tty_t> {
    explicit TTYHandle(std::shared_ptr<Loop> ref,
                 FileHandle desc,
                 bool readable,
                 std::shared_ptr<details::ResetModeMemo> rmm)
        : StreamHandle{std::move(ref)},
          memo{std::move(rmm)},
          fd{desc},
          rw{readable}
    { }

public:
    using Mode = details::UVTTYModeT;

    /**
     * @brief Creates a new tty handle.
     * @param args
     *
     * * A pointer to the loop from which the handle generated.
     * * A valid FileHandle. Usually the file descriptor will be:
     *     * `0` = `stdin`
     *     * `1` = `stdout`
     *     * `2` = `stderr`
     * * A boolean value (_readable_) that specifies the plan on calling `read()` with this
     * stream. Remember that `stdin` is readable, `stdout` is not.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/tty.html#c.uv_tty_init)
     * for further details.
     *
     * @return A pointer to the newly created handle.
     */
    template<typename... Args>
    static std::shared_ptr<TTYHandle> create(Args&&... args) {
        static std::weak_ptr<details::ResetModeMemo> rmm;
        auto ptr = rmm.lock();
        if(!ptr) { rmm = ptr = std::make_shared<details::ResetModeMemo>(); }
        return std::shared_ptr<TTYHandle>{new TTYHandle{std::forward<Args>(args)..., ptr}};
    }

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init() {
        return initialize<uv_tty_t>(&uv_tty_init, fd, rw);
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
     */
    void mode(Mode m) {
        // uv_tty_set_mode is inline, it cannot be used with invoke directly
        invoke([](auto *handle, auto m) {
            return uv_tty_set_mode(handle, m);
        }, get<uv_tty_t>(), static_cast<std::underlying_type_t<Mode>>(m));
    }

    /**
     * @brief Resets TTY settings to default values..
     */
    void reset() noexcept {
        invoke(&uv_tty_reset_mode);
    }

    /**
     * @brief Gets the current Window size.
     * @return The current Window size or `{-1, -1}` in case of errors.
     */
    WinSize getWinSize() {
        WinSize size;

        if(0 != invoke(&uv_tty_get_winsize, get<uv_tty_t>(), &size.width, &size.height)) {
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
