#include "async.h"

namespace uvw {

    void AsyncHandle::sendCallback(uv_async_t *handle) {
        AsyncHandle &async = *(static_cast<AsyncHandle*>(handle->data));
        async.publish(AsyncEvent{});
    }

    bool AsyncHandle::init() {
        return initialize(&uv_async_init, &sendCallback);
    }

    void AsyncHandle::send() {
        invoke(&uv_async_send, get());
    }
}
