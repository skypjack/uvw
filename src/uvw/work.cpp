#include "work.h"
#include "config.h"


namespace uvw {


UVW_INLINE void WorkReq::workCallback(uv_work_t *req) {
    static_cast<WorkReq*>(req->data)->task();
}


UVW_INLINE void WorkReq::queue() {
    invoke(&uv_queue_work, parent(), get(), &workCallback, &defaultCallback<WorkEvent>);
}


}
