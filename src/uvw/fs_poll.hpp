#pragma once


#include <utility>
#include <string>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


class FsPoll final: public Handle<FsPoll> {
    static void startCallback(uv_fs_poll_t *handle, int status, const uv_stat_t *prev, const uv_stat_t *curr) {
        FsPoll &fsPoll = *(static_cast<FsPoll*>(handle->data));
        if(status) { fsPoll.publish(ErrorEvent{status}); }
        else { fsPoll.publish(FsPollEvent{ *prev, *curr }); }
    }

    explicit FsPoll(std::shared_ptr<Loop> ref)
        : Handle{HandleType<uv_fs_poll_t>{}, std::move(ref)}
    { }

public:
    template<typename... Args>
    static std::shared_ptr<FsPoll> create(Args&&... args) {
        return std::shared_ptr<FsPoll>{new FsPoll{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_fs_poll_t>(&uv_fs_poll_init); }

    void start(std::string file, unsigned int interval) {
        invoke(&uv_fs_poll_start, get<uv_fs_poll_t>(), &startCallback, file.data(), interval);
    }

    void stop() { invoke(&uv_fs_poll_stop, get<uv_fs_poll_t>()); }
    std::string path() const noexcept { return details::path(&uv_fs_poll_getpath, get<uv_fs_poll_t>()); }
};


}
