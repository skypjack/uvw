#include "tty.h"

namespace uvw {

    bool TTYHandle::init() {
        return initialize(&uv_tty_init, fd, rw);
    }

    bool TTYHandle::mode(TTYHandle::Mode m) {
        return (0 == uv_tty_set_mode(get(), static_cast<std::underlying_type_t<Mode>>(m)));
    }

    bool TTYHandle::reset() noexcept {
        return (0 == uv_tty_reset_mode());
    }

    WinSize TTYHandle::getWinSize() {
        WinSize size;

        if(0 != uv_tty_get_winsize(get(), &size.width, &size.height)) {
            size.width = -1;
            size.height = -1;
        }

        return size;
    }

    void TTYHandle::vtermState(TTYHandle::VTermState s) const noexcept {
        switch(s) {
            case VTermState::SUPPORTED:
                uv_tty_set_vterm_state(uv_tty_vtermstate_t::UV_TTY_SUPPORTED);
                break;
            case VTermState::UNSUPPORTED:
                uv_tty_set_vterm_state(uv_tty_vtermstate_t::UV_TTY_UNSUPPORTED);
                break;
        }
    }

    TTYHandle::VTermState TTYHandle::vtermState() const noexcept {
        uv_tty_vtermstate_t state;
        uv_tty_get_vterm_state(&state);
        return VTermState{state};
    }
}
