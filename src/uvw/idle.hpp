#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "resource.hpp"
#include "util.hpp"


namespace uvw {


class Idle final: public Resource<Idle> {
    static void startCallback(Idle &idle, std::function<void(UVWError, Idle &)> &cb, uv_idle_t *) {
        cb(UVWError{}, idle);
    }

    explicit Idle(std::shared_ptr<Loop> ref)
        : Resource{HandleType<uv_idle_t>{}, std::move(ref)}
    {
        initialized = (uv_idle_init(parent(), get<uv_idle_t>()) == 0);
    }

public:
    template<typename... Args>
    static std::shared_ptr<Idle> create(Args&&... args) {
        return std::shared_ptr<Idle>{new Idle{std::forward<Args>(args)...}};
    }

    void start(std::function<void(UVWError, Idle &)> cb) noexcept {
        using CBF = CallbackFactory<void(uv_idle_t *)>;
        auto func = CBF::on<&Idle::startCallback>(*this, cb);
        auto err = uv_idle_start(get<uv_idle_t>(), func);
        if(err) { error(err); }
    }

    UVWError stop() noexcept { return UVWError{uv_idle_stop(get<uv_idle_t>())}; }

    explicit operator bool() const noexcept { return initialized; }

private:
    bool initialized;
};


}
