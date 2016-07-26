#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


struct CheckEvent: Event<CheckEvent> { };


class Check final: public Handle<Check, uv_check_t> {
    static void startCallback(uv_check_t *handle) {
        Check &check = *(static_cast<Check*>(handle->data));
        check.publish(CheckEvent{});
    }

    using Handle::Handle;

public:
    template<typename... Args>
    static std::shared_ptr<Check> create(Args&&... args) {
        return std::shared_ptr<Check>{new Check{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_check_t>(&uv_check_init); }

    void start() { invoke(&uv_check_start, get<uv_check_t>(), &startCallback); }
    void stop() { invoke(&uv_check_stop, get<uv_check_t>()); }
};


}
