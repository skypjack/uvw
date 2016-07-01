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

    explicit Idle(std::shared_ptr<Loop> ref)
        : Handle{ResourceType<uv_idle_t>{}, std::move(ref)}
    {
        initialized = (uv_idle_init(parent(), get<uv_idle_t>()) == 0);
    }

public:
    template<typename... Args>
    static std::shared_ptr<Idle> create(Args&&... args) {
        return std::shared_ptr<Idle>{new Idle{std::forward<Args>(args)...}};
    }

    void start() {
        using CBF = CallbackFactory<void(uv_idle_t *)>;
        auto func = CBF::create<&Idle::startCallback>(*this);
        auto err = uv_idle_start(get<uv_idle_t>(), func);
        if(err) publish(ErrorEvent{err});
    }

    void stop() {
        auto err = uv_idle_stop(get<uv_idle_t>());
        if(err) publish(ErrorEvent{err});
    }

    explicit operator bool() const noexcept { return initialized; }

private:
    bool initialized;
};


}
