#pragma once


#include <type_traits>
#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


namespace details {


enum class UVPollEvent: std::underlying_type_t<uv_poll_event> {
    READABLE = UV_READABLE,
    WRITABLE = UV_WRITABLE,
    DISCONNECT = UV_DISCONNECT
};


}


struct PollEvent: Event<PollEvent> {
    explicit PollEvent(Flags<details::UVPollEvent> f) noexcept
        : flgs{std::move(f)}
    { }

    Flags<details::UVPollEvent> flags() const noexcept { return flgs; }

private:
    Flags<details::UVPollEvent> flgs;
};


class PollHandle final: public Handle<PollHandle, uv_poll_t> {
    static void startCallback(uv_poll_t *handle, int status, int events) {
        PollHandle &poll = *(static_cast<PollHandle*>(handle->data));
        if(status) { poll.publish(ErrorEvent{status}); }
        else { poll.publish(PollEvent{static_cast<std::underlying_type_t<Event>>(events)}); }
    }

    using Handle::Handle;

public:
    using Event = details::UVPollEvent;

    template<typename... Args>
    static std::shared_ptr<PollHandle> create(Args&&... args) {
        return std::shared_ptr<PollHandle>{new PollHandle{std::forward<Args>(args)...}};
    }

    bool init(int fd) { return initialize<uv_poll_t>(&uv_poll_init, fd); }

    void start(Flags<Event> flags) { invoke(&uv_poll_start, get<uv_poll_t>(), flags, &startCallback); }
    void start(Event event) { start(Flags<Event>{event}); }
    void stop() { invoke(&uv_poll_stop, get<uv_poll_t>()); }
};


}
