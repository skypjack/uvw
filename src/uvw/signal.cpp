#ifdef UVW_AS_LIB
#    include "signal.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE signal_event::signal_event(int sig) UVW_NOEXCEPT
    : signum{sig} {}

UVW_INLINE void signal_handle::start_callback(uv_signal_t *hndl, int signum) {
    signal_handle &signal = *(static_cast<signal_handle *>(hndl->data));
    signal.publish(signal_event{signum});
}

UVW_INLINE int signal_handle::init() {
    return leak_if(uv_signal_init(parent().raw(), raw()));
}

UVW_INLINE void signal_handle::start(int signum) {
    if(auto err = uv_signal_start(raw(), &start_callback, signum); err != 0) {
        publish(error_event{err});
    }
}

UVW_INLINE void signal_handle::one_shot(int signum) {
    if(auto err = uv_signal_start_oneshot(raw(), &start_callback, signum); err != 0) {
        publish(error_event{err});
    }
}

UVW_INLINE void signal_handle::stop() {
    if(auto err = uv_signal_stop(raw()); err != 0) {
        publish(error_event{err});
    }
}

UVW_INLINE int signal_handle::signal() const UVW_NOEXCEPT {
    return raw()->signum;
}

} // namespace uvw
