#include "prepare.h"

namespace uvw {

    void PrepareHandle::startCallback(uv_prepare_t *handle) {
        PrepareHandle &prepare = *(static_cast<PrepareHandle*>(handle->data));
        prepare.publish(PrepareEvent{});
    }

    bool PrepareHandle::init() {
        return initialize(&uv_prepare_init);
    }

    void PrepareHandle::start() {
        invoke(&uv_prepare_start, get(), &startCallback);
    }

    void PrepareHandle::stop() {
        invoke(&uv_prepare_stop, get());
    }
}
