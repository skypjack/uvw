#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "util.hpp"


namespace uvw {


class Work final: public Request<Work> {
    static void workCallback(uv_work_t *req) {
        Work &work = *(static_cast<Work*>(req->data));
        work.publish(WorkEvent{});
    }

    static void afterWorkCallback(uv_work_t *req, int status) {
        Work &work = *(static_cast<Work*>(req->data));
        auto ptr = work.shared_from_this();
        ptr->reset();
        if(status) { work.publish(ErrorEvent{status}); }
        else { work.publish(AfterWorkEvent{}); }
    }

    explicit Work(std::shared_ptr<Loop> ref)
        : Request{ResourceType<uv_work_t>{}, std::move(ref)}
    { }

public:
    template<typename... Args>
    static std::shared_ptr<Work> create(Args&&... args) {
        return std::shared_ptr<Work>{new Work{std::forward<Args>(args)...}};
    }

    void queue() noexcept {
        if(0 == invoke(&uv_queue_work, parent(), get<uv_work_t>(), &workCallback, &afterWorkCallback)) {
            leak();
        }
    }
};


}
