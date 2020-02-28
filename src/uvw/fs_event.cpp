#include "fs_event.h"
#include "defines.h"


namespace uvw {

    UVW_INLINE_SPECIFIER void
    FsEventHandle::startCallback(uv_fs_event_t *handle, const char *filename, int events, int status) {
        FsEventHandle &fsEvent = *(static_cast<FsEventHandle *>(handle->data));
        if (status) {
            fsEvent.publish(ErrorEvent{status});
        }
        else {
            fsEvent.publish(FsEventEvent{filename, static_cast<std::underlying_type_t<details::UVFsEvent>>(events)});
        }
    }

    UVW_INLINE_SPECIFIER bool FsEventHandle::init() {
        return initialize(&uv_fs_event_init);
    }

    UVW_INLINE_SPECIFIER void FsEventHandle::start(std::string path, Flags<Event> flags) {
        invoke(&uv_fs_event_start, get(), &startCallback, path.data(), flags);
    }

    UVW_INLINE_SPECIFIER void FsEventHandle::start(std::string path, FsEventHandle::Event flag) {
        start(std::move(path), Flags<Event>{flag});
    }

    UVW_INLINE_SPECIFIER void FsEventHandle::stop() {
        invoke(&uv_fs_event_stop, get());
    }

    UVW_INLINE_SPECIFIER std::string FsEventHandle::path() noexcept {
        return details::tryRead(&uv_fs_event_getpath, get());
    }
}
