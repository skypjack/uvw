#ifndef UVW_TTY_INCLUDE_H
#define UVW_TTY_INCLUDE_H

#include <memory>
#include <type_traits>
#include <uv.h>
#include "config.h"
#include "stream.h"
#include "util.h"

namespace uvw {

namespace details {

struct reset_mode_memo {
    ~reset_mode_memo();
};

enum class uvw_tty_mode_t : std::underlying_type_t<uv_tty_mode_t> {
    NORMAL = UV_TTY_MODE_NORMAL,
    RAW = UV_TTY_MODE_RAW,
    IO = UV_TTY_MODE_IO
};

enum class uvw_tty_vtermstate_t : std::underlying_type_t<uv_tty_vtermstate_t> {
    SUPPORTED = UV_TTY_SUPPORTED,
    UNSUPPORTED = UV_TTY_UNSUPPORTED
};

} // namespace details

/**
 * @brief The tty handle.
 *
 * TTY handles represent a stream for the console.
 *
 * To create a `tty_handle` through a `loop`, arguments follow:
 *
 * * A valid file_handle. Usually the file descriptor will be:
 *     * `uvw::std_in` or `0` for `stdin`
 *     * `uvw::std_out` or `1` for `stdout`
 *     * `uvw::std_err` or `2` for `stderr`
 * * A boolean value that specifies the plan on calling `read()` with this
 * stream. Remember that `stdin` is readable, `stdout` is not.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/tty.html#c.uv_tty_init)
 * for further details.
 */
class tty_handle final: public stream_handle<tty_handle, uv_tty_t> {
    static std::shared_ptr<details::reset_mode_memo> mode_memo_handler();

public:
    using tty_mode = details::uvw_tty_mode_t;
    using tty_vtermstate = details::uvw_tty_vtermstate_t;

    explicit tty_handle(loop::token token, std::shared_ptr<loop> ref, file_handle desc, bool readable);

    /**
     * @brief Initializes the handle.
     * @return Underlying return value.
     */
    int init() final;

    /**
     * @brief Sets the TTY using the specified terminal mode.
     *
     * Available modes are:
     *
     * * `TTY::tty_mode::NORMAL`
     * * `TTY::tty_mode::RAW`
     * * `TTY::tty_mode::IO`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/tty.html#c.uv_tty_mode_t)
     * for further details.
     *
     * @param m The mode to be set.
     * @return True in case of success, false otherwise.
     */
    bool mode(tty_mode m);

    /**
     * @brief Resets TTY settings to default values.
     * @return True in case of success, false otherwise.
     */
    bool reset_mode() noexcept;

    /**
     * @brief Gets the current Window size.
     * @return The current Window size or `{-1, -1}` in case of errors.
     */
    win_size get_win_size();

    /**
     * @brief Controls whether console virtual terminal sequences are processed
     * by the library or console.
     *
     * This function is only meaningful on Windows systems. On Unix it is
     * silently ignored.
     *
     * Available states are:
     *
     * * `TTY::tty_vtermstate::SUPPORTED`
     * * `TTY::tty_vtermstate::UNSUPPORTED`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/tty.html#c.uv_tty_vtermstate_t)
     * for further details.
     *
     * @param s The state to be set.
     */
    void vterm_state(tty_vtermstate s) const noexcept;

    /**
     * @brief Gets the current state of whether console virtual terminal
     * sequences are handled by the library or the console.
     *
     * This function is not implemented on Unix.
     *
     * Available states are:
     *
     * * `TTY::tty_vtermstate::SUPPORTED`
     * * `TTY::tty_vtermstate::UNSUPPORTED`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/tty.html#c.uv_tty_vtermstate_t)
     * for further details.
     *
     * @return The current state.
     */
    tty_vtermstate vterm_state() const noexcept;

private:
    std::shared_ptr<details::reset_mode_memo> memo;
    file_handle fd;
    int rw;
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "tty.cpp"
#endif

#endif // UVW_TTY_INCLUDE_H
