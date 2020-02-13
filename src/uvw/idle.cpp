#ifdef UVW_BUILD_STATIC_LIB
#include "idle.h"
#endif //UVW_BUILD_STATIC_LIB
#include "defines.h"

namespace uvw {

    UVW_INLINE_SPECIFIER void IdleHandle::startCallback(uv_idle_t *handle) {
        IdleHandle &idle = *(static_cast<IdleHandle *>(handle->data));
        idle.publish(IdleEvent{});
    }

    UVW_INLINE_SPECIFIER bool IdleHandle::init() {
        return initialize(&uv_idle_init);
    }

    UVW_INLINE_SPECIFIER void IdleHandle::start() {
        invoke(&uv_idle_start, get(), &startCallback);
    }

    UVW_INLINE_SPECIFIER void IdleHandle::stop() {
        invoke(&uv_idle_stop, get());
    }
}
