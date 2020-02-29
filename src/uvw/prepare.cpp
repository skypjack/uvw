#include "prepare.h"
#include "defines.h"

namespace uvw {

    UVW_INLINE_SPECIFIER void PrepareHandle::startCallback(uv_prepare_t *handle) {
        PrepareHandle &prepare = *(static_cast<PrepareHandle *>(handle->data));
        prepare.publish(PrepareEvent{});
    }

    UVW_INLINE_SPECIFIER bool PrepareHandle::init() {
        return initialize(&uv_prepare_init);
    }

    UVW_INLINE_SPECIFIER void PrepareHandle::start() {
        invoke(&uv_prepare_start, get(), &startCallback);
    }

    UVW_INLINE_SPECIFIER void PrepareHandle::stop() {
        invoke(&uv_prepare_stop, get());
    }
}
