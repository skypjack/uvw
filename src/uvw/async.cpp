#include "async.h"
#include "defines.h"

namespace uvw {

    UVW_INLINE_SPECIFIER void AsyncHandle::sendCallback(uv_async_t *handle) {
        AsyncHandle &async = *(static_cast<AsyncHandle *>(handle->data));
        async.publish(AsyncEvent{});
    }

    UVW_INLINE_SPECIFIER bool AsyncHandle::init() {
        return initialize(&uv_async_init, &sendCallback);
    }

    UVW_INLINE_SPECIFIER void AsyncHandle::send() {
        invoke(&uv_async_send, get());
    }
}
