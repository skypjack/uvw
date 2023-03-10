#ifdef UVW_AS_LIB
#    include "async.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE void async_handle::send_callback(uv_async_t *hndl) {
    async_handle &async = *(static_cast<async_handle *>(hndl->data));
    async.publish(async_event{});
}

UVW_INLINE int async_handle::init() {
    return leak_if(uv_async_init(parent().raw(), raw(), &send_callback));
}

UVW_INLINE int async_handle::send() {
    return uv_async_send(raw());
}

} // namespace uvw
