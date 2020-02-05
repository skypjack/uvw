#include "check.h"

namespace uvw {

    void CheckHandle::startCallback(uv_check_t *handle) {
        CheckHandle &check = *(static_cast<CheckHandle*>(handle->data));
        check.publish(CheckEvent{});
    }

    bool CheckHandle::init() {
        return initialize(&uv_check_init);
    }

    void CheckHandle::start() {
        invoke(&uv_check_start, get(), &startCallback);
    }

    void CheckHandle::stop() {
        invoke(&uv_check_stop, get());
    }
}
