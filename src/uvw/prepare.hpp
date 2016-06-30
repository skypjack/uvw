#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


class Prepare final: public Handle<Prepare> {
    static void startCallback(Prepare &prepare, uv_prepare_t *) {
        prepare.publish(PrepareEvent{});
    }

    explicit Prepare(std::shared_ptr<Loop> ref)
        : Handle{ResourceType<uv_prepare_t>{}, std::move(ref)}
    {
        initialized = (uv_prepare_init(parent(), get<uv_prepare_t>()) == 0);
    }

public:
    template<typename... Args>
    static std::shared_ptr<Prepare> create(Args&&... args) {
        return std::shared_ptr<Prepare>{new Prepare{std::forward<Args>(args)...}};
    }

    UVWError start() {
        using CBF = CallbackFactory<void(uv_prepare_t *)>;
        auto func = CBF::create<&Prepare::startCallback>(*this);
        return UVWError{uv_prepare_start(get<uv_prepare_t>(), func)};
    }

    UVWError stop() {
        return UVWError{uv_prepare_stop(get<uv_prepare_t>())};
    }

    explicit operator bool() const noexcept { return initialized; }

private:
    bool initialized;
};


}
