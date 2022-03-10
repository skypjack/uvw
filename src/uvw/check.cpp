#ifdef UVW_AS_LIB
#    include "check.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE void check_handle::start_callback(uv_check_t *hndl) {
    check_handle &check = *(static_cast<check_handle *>(hndl->data));
    check.publish(check_event{});
}

UVW_INLINE int check_handle::init() {
    return leak_if(uv_check_init(parent().raw(), raw()));
}

UVW_INLINE void check_handle::start() {
    if(auto err = uv_check_start(raw(), &start_callback); err != 0) {
        publish(error_event{err});
    }
}

UVW_INLINE void check_handle::stop() {
    if(auto err = uv_check_stop(raw()); err != 0) {
        publish(error_event{err});
    }
}

} // namespace uvw
