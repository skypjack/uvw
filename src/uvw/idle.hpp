#pragma once


#include <utility>
#include <memory>
#include <uv.h>
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

    UVWError start() {
        using CBF = CallbackFactory<void(uv_idle_t *)>;
        auto func = CBF::create<&Idle::startCallback>(*this);
        return UVWError{uv_idle_start(get<uv_idle_t>(), func)};
    }

    UVWError stop() {
        return UVWError{uv_idle_stop(get<uv_idle_t>())};
    }

    explicit operator bool() const noexcept { return initialized; }

private:
    bool initialized;
};


}
