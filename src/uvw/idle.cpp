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

UVW_INLINE int idle_handle::start() {
    return uv_idle_start(raw(), &start_callback);
}

UVW_INLINE int idle_handle::stop() {
    return uv_idle_stop(raw());
}

} // namespace uvw
