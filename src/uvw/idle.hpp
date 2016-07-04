#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


class Idle final: public Handle<Idle> {
    static void startCallback(Idle &idle, uv_idle_t *) {
        idle.publish(IdleEvent{});
    }

    using Handle<Idle>::Handle;

public:
    template<typename... Args>
    static std::shared_ptr<Idle> create(Args&&... args) {
        return std::shared_ptr<Idle>{new Idle{std::forward<Args>(args)...}};
    }

    bool init() {
        return Handle<Idle>::init<uv_idle_t>(&uv_idle_init);
    }

    void start() {
        using CBF = CallbackFactory<void(uv_idle_t *)>;
        auto func = &CBF::template proto<&Idle::startCallback>;
        auto err = uv_idle_start(get<uv_idle_t>(), func);
        if(err) publish(ErrorEvent{err});
    }

    void stop() {
        auto err = uv_idle_stop(get<uv_idle_t>());
        if(err) publish(ErrorEvent{err});
    }
};


}
