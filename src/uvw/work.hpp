#pragma once


#include <functional>
#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "util.hpp"


namespace uvw {


class Work final: public Request<Work> {
    static void workCallback(uv_work_t *req) {
        static_cast<Work*>(req->data)->task();
    }

    explicit Work(std::shared_ptr<Loop> ref)
        : Request{RequestType<uv_work_t>{}, std::move(ref)}
    { }

public:
    using Task = std::function<void(void)>;

    template<typename... Args>
    static std::shared_ptr<Work> create(Args&&... args) {
        return std::shared_ptr<Work>{new Work{std::forward<Args>(args)...}};
    }

    void queue(Task t) {
        if(0 == exec<uv_work_t, WorkEvent>(&uv_queue_work, parent(), get<uv_work_t>(), &workCallback)) {
            task = std::move(t);
        }
    }

private:
    Task task;
};


}
