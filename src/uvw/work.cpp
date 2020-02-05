#include "work.h"

namespace uvw {

    void WorkReq::workCallback(uv_work_t *req) {
        static_cast<WorkReq*>(req->data)->task();
    }

    void WorkReq::queue() {
        invoke(&uv_queue_work, parent(), get(), &workCallback, &defaultCallback<WorkEvent>);
    }
}
