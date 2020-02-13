#ifdef UVW_BUILD_STATIC_LIB
#include "tty.h"
#endif //UVW_BUILD_STATIC_LIB
#include "defines.h"

namespace uvw {

    UVW_INLINE_SPECIFIER bool TTYHandle::init() {
        return initialize(&uv_tty_init, fd, rw);
    }

    UVW_INLINE_SPECIFIER bool TTYHandle::mode(TTYHandle::Mode m) {
        return (0 == uv_tty_set_mode(get(), static_cast<std::underlying_type_t<Mode>>(m)));
    }

    UVW_INLINE_SPECIFIER bool TTYHandle::reset() noexcept {
        return (0 == uv_tty_reset_mode());
    }

    UVW_INLINE_SPECIFIER WinSize TTYHandle::getWinSize() {
        WinSize size;

        if (0 != uv_tty_get_winsize(get(), &size.width, &size.height)) {
            size.width = -1;
            size.height = -1;
        }

        return size;
    }

    UVW_INLINE_SPECIFIER void TTYHandle::vtermState(TTYHandle::VTermState s) const noexcept {
        switch (s) {
            case VTermState::SUPPORTED:
                uv_tty_set_vterm_state(uv_tty_vtermstate_t::UV_TTY_SUPPORTED);
                break;
            case VTermState::UNSUPPORTED:
                uv_tty_set_vterm_state(uv_tty_vtermstate_t::UV_TTY_UNSUPPORTED);
                break;
        }
    }

    UVW_INLINE_SPECIFIER TTYHandle::VTermState TTYHandle::vtermState() const noexcept {
        uv_tty_vtermstate_t state;
        uv_tty_get_vterm_state(&state);
        return VTermState{state};
    }
}
