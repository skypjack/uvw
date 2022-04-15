#ifdef UVW_AS_LIB
#    include "fs_poll.h"
#endif

#include <utility>
#include "config.h"

namespace uvw {

UVW_INLINE fs_poll_event::fs_poll_event(file_info previous, file_info current) noexcept
    : prev{std::move(previous)}, curr{std::move(current)} {}

UVW_INLINE void fs_poll_handle::start_callback(uv_fs_poll_t *hndl, int status, const uv_stat_t *prev, const uv_stat_t *curr) {
    if(fs_poll_handle &fsPoll = *(static_cast<fs_poll_handle *>(hndl->data)); status) {
        fsPoll.publish(error_event{status});
    } else {
        fsPoll.publish(fs_poll_event{*prev, *curr});
    }
}

UVW_INLINE int fs_poll_handle::init() {
    return leak_if(uv_fs_poll_init(parent().raw(), raw()));
}

UVW_INLINE int fs_poll_handle::start(const std::string &file, fs_poll_handle::time interval) {
    return uv_fs_poll_start(raw(), &start_callback, file.data(), interval.count());
}

UVW_INLINE int fs_poll_handle::stop() {
    return uv_fs_poll_stop(raw());
}

UVW_INLINE std::string fs_poll_handle::path() noexcept {
    return details::try_read(&uv_fs_poll_getpath, raw());
}

} // namespace uvw
