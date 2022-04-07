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

UVW_INLINE int check_handle::start() {
    return uv_check_start(raw(), &start_callback);
}

UVW_INLINE int check_handle::stop() {
    return uv_check_stop(raw());
}

} // namespace uvw
