#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


struct AsyncEvent: Event<AsyncEvent> { };


class AsyncHandle final: public Handle<AsyncHandle, uv_async_t> {
    static void sendCallback(uv_async_t *handle) {
        AsyncHandle &async = *(static_cast<AsyncHandle*>(handle->data));
        async.publish(AsyncEvent{});
    }

    using Handle::Handle;

public:
    template<typename... Args>
    static std::shared_ptr<AsyncHandle> create(Args&&... args) {
        return std::shared_ptr<AsyncHandle>{new AsyncHandle{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_async_t>(&uv_async_init, &sendCallback); }

    void send() { invoke(&uv_async_send, get<uv_async_t>()); }
};


}
