#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


class Prepare final: public Handle<Prepare> {
    static void startCallback(uv_prepare_t *handle) {
        Prepare &prepare = *(static_cast<Prepare*>(handle->data));
        prepare.publish(PrepareEvent{});
    }

    explicit Prepare(std::shared_ptr<Loop> ref)
        : Handle{ResourceType<uv_prepare_t>{}, std::move(ref)}
    { }

public:
    template<typename... Args>
    static std::shared_ptr<Prepare> create(Args&&... args) {
        return std::shared_ptr<Prepare>{new Prepare{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_prepare_t>(&uv_prepare_init); }

    void start() { invoke(&uv_prepare_start, get<uv_prepare_t>(), &startCallback); }
    void stop() { invoke(&uv_prepare_stop, get<uv_prepare_t>()); }
};


}
