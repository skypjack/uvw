#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


struct PrepareEvent: Event<PrepareEvent> { };


class PrepareHandle final: public Handle<PrepareHandle, uv_prepare_t> {
    static void startCallback(uv_prepare_t *handle) {
        PrepareHandle &prepare = *(static_cast<PrepareHandle*>(handle->data));
        prepare.publish(PrepareEvent{});
    }

    using Handle::Handle;

public:
    template<typename... Args>
    static std::shared_ptr<PrepareHandle> create(Args&&... args) {
        return std::shared_ptr<PrepareHandle>{new PrepareHandle{std::forward<Args>(args)...}};
    }

    bool init() {
        return initialize<uv_prepare_t>(&uv_prepare_init);
    }

    void start() {
        invoke(&uv_prepare_start, get<uv_prepare_t>(), &startCallback);
    }

    void stop() {
        invoke(&uv_prepare_stop, get<uv_prepare_t>());
    }
};


}
