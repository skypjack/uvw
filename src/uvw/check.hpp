#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


class Check final: public Handle<Check> {
    static void startCallback(Check &check, uv_check_t *) {
        check.publish(CheckEvent{});
    }

    using Handle<Check>::Handle;

public:
    template<typename... Args>
    static std::shared_ptr<Check> create(Args&&... args) {
        return std::shared_ptr<Check>{new Check{std::forward<Args>(args)...}};
    }

    bool init() {
        return Handle<Check>::init<uv_check_t>(&uv_check_init);
    }

    void start() {
        using CBF = CallbackFactory<void(uv_check_t *)>;
        auto func = &CBF::template proto<&Check::startCallback>;
        auto err = uv_check_start(get<uv_check_t>(), func);
        if(err) publish(ErrorEvent{err});
    }

    void stop() {
        auto err = uv_check_stop(get<uv_check_t>());
        if(err) publish(ErrorEvent{err});
    }

private:
    bool initialized = false;
};


}
