#ifdef UVW_BUILD_STATIC_LIB
#include "check.h"
#endif //UVW_BUILD_STATIC_LIB
#include "defines.h"

namespace uvw {

    UVW_INLINE_SPECIFIER void CheckHandle::startCallback(uv_check_t *handle) {
        CheckHandle &check = *(static_cast<CheckHandle *>(handle->data));
        check.publish(CheckEvent{});
    }

    UVW_INLINE_SPECIFIER bool CheckHandle::init() {
        return initialize(&uv_check_init);
    }

    UVW_INLINE_SPECIFIER void CheckHandle::start() {
        invoke(&uv_check_start, get(), &startCallback);
    }

    UVW_INLINE_SPECIFIER void CheckHandle::stop() {
        invoke(&uv_check_stop, get());
    }
}
