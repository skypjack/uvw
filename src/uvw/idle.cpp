#include "idle.h"

namespace uvw {

    void IdleHandle::startCallback(uv_idle_t *handle) {
        IdleHandle &idle = *(static_cast<IdleHandle*>(handle->data));
        idle.publish(IdleEvent{});
    }

    bool IdleHandle::init() {
        return initialize(&uv_idle_init);
    }

    void IdleHandle::start() {
        invoke(&uv_idle_start, get(), &startCallback);
    }

    void IdleHandle::stop() {
        invoke(&uv_idle_stop, get());
    }
}
