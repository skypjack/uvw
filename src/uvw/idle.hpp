#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


struct IdleEvent: Event<IdleEvent> { };


class IdleHandle final: public Handle<IdleHandle, uv_idle_t> {
    static void startCallback(uv_idle_t *handle) {
        IdleHandle &idle = *(static_cast<IdleHandle*>(handle->data));
        idle.publish(IdleEvent{});
    }

    using Handle::Handle;

public:
    template<typename... Args>
    static std::shared_ptr<IdleHandle> create(Args&&... args) {
        return std::shared_ptr<IdleHandle>{new IdleHandle{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_idle_t>(&uv_idle_init); }

    void start() { invoke(&uv_idle_start, get<uv_idle_t>(), &startCallback); }
    void stop() { invoke(&uv_idle_stop, get<uv_idle_t>()); }
};


}
