#include "async.h"
#include "config.h"


namespace uvw {


UVW_INLINE void AsyncHandle::sendCallback(uv_async_t *handle) {
    AsyncHandle &async = *(static_cast<AsyncHandle *>(handle->data));
    async.publish(AsyncEvent{});
}


UVW_INLINE bool AsyncHandle::init() {
    return initialize(&uv_async_init, &sendCallback);
}


UVW_INLINE void AsyncHandle::send() {
    invoke(&uv_async_send, get());
}


}
