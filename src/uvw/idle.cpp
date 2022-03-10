#ifdef UVW_AS_LIB
#    include "idle.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE void idle_handle::start_callback(uv_idle_t *hndl) {
    idle_handle &idle = *(static_cast<idle_handle *>(hndl->data));
    idle.publish(idle_event{});
}

UVW_INLINE int idle_handle::init() {
    return leak_if(uv_idle_init(parent().raw(), raw()));
}

UVW_INLINE void idle_handle::start() {
    if(auto err = uv_idle_start(raw(), &start_callback); err != 0) {
        publish(error_event{err});
    }
}

UVW_INLINE void idle_handle::stop() {
    if(auto err = uv_idle_stop(raw()); err != 0) {
        publish(error_event{err});
    }
}

} // namespace uvw
