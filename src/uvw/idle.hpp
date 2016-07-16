#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


class Idle final: public Handle<Idle> {
    static void startCallback(uv_idle_t *handle) {
        Idle &idle = *(static_cast<Idle*>(handle->data));
        idle.publish(IdleEvent{});
    }

    explicit Idle(std::shared_ptr<Loop> ref)
        : Handle{HandleType<uv_idle_t>{}, std::move(ref)}
    { }

public:
    template<typename... Args>
    static std::shared_ptr<Idle> create(Args&&... args) {
        return std::shared_ptr<Idle>{new Idle{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_idle_t>(&uv_idle_init); }

    void start() { invoke(&uv_idle_start, get<uv_idle_t>(), &startCallback); }
    void stop() { invoke(&uv_idle_stop, get<uv_idle_t>()); }
};


}
