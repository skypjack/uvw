#ifdef UVW_AS_LIB
#    include "work.h"
#endif

#include <utility>

#include "config.h"

namespace uvw {

UVW_INLINE work_req::work_req(loop::token token, std::shared_ptr<loop> ref, task t)
    : request{token, std::move(ref)}, func{t} {}

UVW_INLINE void work_req::work_callback(uv_work_t *req) {
    static_cast<work_req *>(req->data)->func();
}

UVW_INLINE void work_req::after_work_callback(uv_work_t *req, int status) {
    if(auto ptr = reserve(req); status) {
        ptr->publish(error_event{status});
    } else {
        ptr->publish(work_event{});
    }
}

UVW_INLINE void work_req::queue() {
    // uv_queue_work only returns an error if the callback is null which is not the case here
    this->leak_if(uv_queue_work(parent().raw(), raw(), &work_callback, &after_work_callback));
}

} // namespace uvw
