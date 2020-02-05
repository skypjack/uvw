#include "signal.h"

namespace uvw {

    void SignalHandle::startCallback(uv_signal_t *handle, int signum) {
        SignalHandle &signal = *(static_cast<SignalHandle*>(handle->data));
        signal.publish(SignalEvent{signum});
    }

    bool SignalHandle::init() {
        return initialize(&uv_signal_init);
    }

    void SignalHandle::start(int signum) {
        invoke(&uv_signal_start, get(), &startCallback, signum);
    }

    void SignalHandle::oneShot(int signum) {
        invoke(&uv_signal_start_oneshot, get(), &startCallback, signum);
    }

    void SignalHandle::stop() {
        invoke(&uv_signal_stop, get());
    }

    int SignalHandle::signal() const noexcept {
        return get()->signum;
    }

}
