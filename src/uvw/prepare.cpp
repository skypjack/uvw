#ifdef UVW_AS_LIB
#    include "prepare.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE void prepare_handle::start_callback(uv_prepare_t *hndl) {
    prepare_handle &prepare = *(static_cast<prepare_handle *>(hndl->data));
    prepare.publish(prepare_event{});
}

UVW_INLINE int prepare_handle::init() {
    return leak_if(uv_prepare_init(parent().raw(), raw()));
}

UVW_INLINE int prepare_handle::start() {
    return uv_prepare_start(raw(), &start_callback);
}

UVW_INLINE int prepare_handle::stop() {
    return uv_prepare_stop(raw());
}

} // namespace uvw
