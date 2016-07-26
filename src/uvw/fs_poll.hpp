#pragma once


#include <utility>
#include <string>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


struct FsPollEvent: Event<FsPollEvent> {
    explicit FsPollEvent(const Stat &p, const Stat &c) noexcept
        : prev(p), curr(c)
    { }

    const Stat & previous() const noexcept { return prev; }
    const Stat & current() const noexcept { return curr; }

private:
    Stat prev;
    Stat curr;
};


class FsPoll final: public Handle<FsPoll, uv_fs_poll_t> {
    static void startCallback(uv_fs_poll_t *handle, int status, const uv_stat_t *prev, const uv_stat_t *curr) {
        FsPoll &fsPoll = *(static_cast<FsPoll*>(handle->data));
        if(status) { fsPoll.publish(ErrorEvent{status}); }
        else { fsPoll.publish(FsPollEvent{ *prev, *curr }); }
    }

    using Handle::Handle;

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
    std::string path() noexcept { return details::path(&uv_fs_poll_getpath, get<uv_fs_poll_t>()); }
};


}
