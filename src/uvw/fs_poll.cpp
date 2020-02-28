#include "fs_poll.h"
#include "defines.h"

namespace uvw {

    UVW_INLINE_SPECIFIER void
    FsPollHandle::startCallback(uv_fs_poll_t *handle, int status, const uv_stat_t *prev, const uv_stat_t *curr) {
        FsPollHandle &fsPoll = *(static_cast<FsPollHandle *>(handle->data));
        if (status) {
            fsPoll.publish(ErrorEvent{status});
        }
        else {
            fsPoll.publish(FsPollEvent{*prev, *curr});
        }
    }

    UVW_INLINE_SPECIFIER bool FsPollHandle::init() {
        return initialize(&uv_fs_poll_init);
    }

    UVW_INLINE_SPECIFIER void FsPollHandle::start(std::string file, FsPollHandle::Time interval) {
        invoke(&uv_fs_poll_start, get(), &startCallback, file.data(), interval.count());
    }

    UVW_INLINE_SPECIFIER void FsPollHandle::stop() {
        invoke(&uv_fs_poll_stop, get());
    }

    UVW_INLINE_SPECIFIER std::string FsPollHandle::path() noexcept {
        return details::tryRead(&uv_fs_poll_getpath, get());
    }
}
