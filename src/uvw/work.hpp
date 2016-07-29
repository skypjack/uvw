#pragma once


#include <functional>
#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "util.hpp"


namespace uvw {


struct WorkEvent: Event<WorkEvent> { };


class WorkReq final: public Request<WorkReq, uv_work_t> {
    using InternalTask = std::function<void(void)>;

    static void workCallback(uv_work_t *req) {
        static_cast<WorkReq*>(req->data)->task();
    }

    explicit WorkReq(std::shared_ptr<Loop> ref, InternalTask t)
        : Request{std::move(ref)}, task{t}
    { }

public:
    using Task = InternalTask;

    template<typename... Args>
    static std::shared_ptr<WorkReq> create(Args&&... args) {
        return std::shared_ptr<WorkReq>{new WorkReq{std::forward<Args>(args)...}};
    }

    void queue() {
        invoke(&uv_queue_work, parent(), get<uv_work_t>(), &workCallback, &defaultCallback<uv_work_t, WorkEvent>);
    }

private:
    Task task{};
};


}
