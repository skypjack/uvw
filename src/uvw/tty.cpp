#ifdef UVW_AS_LIB
#    include "tty.h"
#endif

#include <utility>
#include "config.h"

namespace uvw {

UVW_INLINE details::reset_mode_memo::~reset_mode_memo() {
    uv_tty_reset_mode();
}

UVW_INLINE tty_handle::tty_handle(loop::token token, std::shared_ptr<loop> ref, file_handle desc, bool readable)
    : stream_handle{token, std::move(ref)},
      memo{mode_memo_handler()},
      fd{desc},
      rw{readable} {}

UVW_INLINE std::shared_ptr<details::reset_mode_memo> tty_handle::mode_memo_handler() {
    static std::weak_ptr<details::reset_mode_memo> weak;
    auto shared = weak.lock();
    if(!shared) { weak = shared = std::make_shared<details::reset_mode_memo>(); }
    return shared;
};

UVW_INLINE int tty_handle::init() {
    return leak_if(uv_tty_init(parent().raw(), raw(), fd, rw));
}

UVW_INLINE bool tty_handle::mode(tty_handle::tty_mode m) {
    return (0 == uv_tty_set_mode(raw(), static_cast<uv_tty_mode_t>(m)));
}

UVW_INLINE bool tty_handle::reset_mode() noexcept {
    return (0 == uv_tty_reset_mode());
}

UVW_INLINE win_size tty_handle::get_win_size() {
    win_size size;

    if(0 != uv_tty_get_winsize(raw(), &size.width, &size.height)) {
        size.width = -1;
        size.height = -1;
    }

    return size;
}

UVW_INLINE void tty_handle::vterm_state(tty_handle::tty_vtermstate s) const noexcept {
    switch(s) {
    case tty_vtermstate::SUPPORTED:
        uv_tty_set_vterm_state(uv_tty_vtermstate_t::UV_TTY_SUPPORTED);
        break;
    case tty_vtermstate::UNSUPPORTED:
        uv_tty_set_vterm_state(uv_tty_vtermstate_t::UV_TTY_UNSUPPORTED);
        break;
    }
}

UVW_INLINE tty_handle::tty_vtermstate tty_handle::vterm_state() const noexcept {
    uv_tty_vtermstate_t state;
    uv_tty_get_vterm_state(&state);
    return tty_vtermstate{state};
}

} // namespace uvw
