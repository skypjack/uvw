#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


class Async final: public Handle<Async> {
    static void sendCallback(uv_async_t *handle) {
        Async &async = *(static_cast<Async*>(handle->data));
        async.publish(AsyncEvent{});
    }

    explicit Async(std::shared_ptr<Loop> ref)
        : Handle{HandleType<uv_async_t>{}, std::move(ref)}
    { }

public:
    template<typename... Args>
    static std::shared_ptr<Async> create(Args&&... args) {
        return std::shared_ptr<Async>{new Async{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_async_t>(&uv_async_init, &sendCallback); }

    void send() { invoke(&uv_async_send, get<uv_async_t>()); }
};


}
