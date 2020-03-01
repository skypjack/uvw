#include <utility>
#include "work.h"
#include "config.h"


namespace uvw {


WorkReq::WorkReq(ConstructorAccess ca, std::shared_ptr<Loop> ref, InternalTask t)
    : Request{ca, std::move(ref)}, task{t}
{}


UVW_INLINE void WorkReq::workCallback(uv_work_t *req) {
    static_cast<WorkReq*>(req->data)->task();
}


UVW_INLINE void WorkReq::queue() {
    invoke(&uv_queue_work, parent(), get(), &workCallback, &defaultCallback<WorkEvent>);
}


}
