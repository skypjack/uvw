#ifdef UVW_AS_LIB
#    include "fs_event.h"
#endif

#include <utility>
#include "config.h"

namespace uvw {

UVW_INLINE fs_event_event::fs_event_event(const char *pathname, details::uvw_fs_event events)
    : filename{pathname}, flags{std::move(events)} {}

UVW_INLINE void fs_event_handle::start_callback(uv_fs_event_t *hndl, const char *filename, int events, int status) {
    if(fs_event_handle &fsEvent = *(static_cast<fs_event_handle *>(hndl->data)); status) {
        fsEvent.publish(error_event{status});
    } else {
        fsEvent.publish(fs_event_event{filename, details::uvw_fs_event(events)});
    }
}

UVW_INLINE int fs_event_handle::init() {
    return leak_if(uv_fs_event_init(parent().raw(), raw()));
}

UVW_INLINE int fs_event_handle::start(const std::string &path, event_flags flags) {
    return uv_fs_event_start(raw(), &start_callback, path.data(), static_cast<uv_fs_event_flags>(flags));
}

UVW_INLINE int fs_event_handle::stop() {
    return uv_fs_event_stop(raw());
}

UVW_INLINE std::string fs_event_handle::path() noexcept {
    return details::try_read(&uv_fs_event_getpath, raw());
}

} // namespace uvw
