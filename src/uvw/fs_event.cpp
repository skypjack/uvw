#include "fs_event.h"

namespace uvw {

    void FsEventHandle::startCallback(uv_fs_event_t *handle, const char *filename, int events, int status) {
        FsEventHandle &fsEvent = *(static_cast<FsEventHandle*>(handle->data));
        if(status) { fsEvent.publish(ErrorEvent{status}); }
        else { fsEvent.publish(FsEventEvent{filename, static_cast<std::underlying_type_t<details::UVFsEvent>>(events)}); }
    }

    bool FsEventHandle::init() {
        return initialize(&uv_fs_event_init);
    }

    void FsEventHandle::start(std::string path, Flags<Event> flags) {
        invoke(&uv_fs_event_start, get(), &startCallback, path.data(), flags);
    }

    void FsEventHandle::start(std::string path, FsEventHandle::Event flag) {
        start(std::move(path), Flags<Event>{flag});
    }

    void FsEventHandle::stop() {
        invoke(&uv_fs_event_stop, get());
    }

    std::string FsEventHandle::path() noexcept {
        return details::tryRead(&uv_fs_event_getpath, get());
    }
}
