#include "signal.h"
#include "defines.h"

namespace uvw {

    UVW_INLINE_SPECIFIER void SignalHandle::startCallback(uv_signal_t *handle, int signum) {
        SignalHandle &signal = *(static_cast<SignalHandle *>(handle->data));
        signal.publish(SignalEvent{signum});
    }

    UVW_INLINE_SPECIFIER bool SignalHandle::init() {
        return initialize(&uv_signal_init);
    }

    UVW_INLINE_SPECIFIER void SignalHandle::start(int signum) {
        invoke(&uv_signal_start, get(), &startCallback, signum);
    }

    UVW_INLINE_SPECIFIER void SignalHandle::oneShot(int signum) {
        invoke(&uv_signal_start_oneshot, get(), &startCallback, signum);
    }

    UVW_INLINE_SPECIFIER void SignalHandle::stop() {
        invoke(&uv_signal_stop, get());
    }

    UVW_INLINE_SPECIFIER int SignalHandle::signal() const noexcept {
        return get()->signum;
    }

}
