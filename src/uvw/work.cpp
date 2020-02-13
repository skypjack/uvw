#ifdef UVW_BUILD_STATIC_LIB
#include "work.h"
#endif //UVW_BUILD_STATIC_LIB
#include "defines.h"

namespace uvw {

    UVW_INLINE_SPECIFIER void WorkReq::workCallback(uv_work_t *req) {
        static_cast<WorkReq*>(req->data)->task();
    }

    UVW_INLINE_SPECIFIER  void WorkReq::queue() {
        invoke(&uv_queue_work, parent(), get(), &workCallback, &defaultCallback<WorkEvent>);
    }
}
