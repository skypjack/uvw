#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


class Prepare final: public Handle<Prepare> {
    static void startCallback(Prepare &prepare, uv_prepare_t *) {
        prepare.publish(PrepareEvent{});
    }

    using Handle<Prepare>::Handle;

public:
    template<typename... Args>
    static std::shared_ptr<Prepare> create(Args&&... args) {
        return std::shared_ptr<Prepare>{new Prepare{std::forward<Args>(args)...}};
    }

    bool init() {
        return Handle<Prepare>::init<uv_prepare_t>(&uv_prepare_init);
    }

    void start() {
        using CBF = CallbackFactory<void(uv_prepare_t *)>;
        auto func = &CBF::template proto<&Prepare::startCallback>;
        auto err = uv_prepare_start(get<uv_prepare_t>(), func);
        if(err) publish(ErrorEvent{err});
    }

    void stop() {
        auto err = uv_prepare_stop(get<uv_prepare_t>());
        if(err) publish(ErrorEvent{err});
    }
};


}
