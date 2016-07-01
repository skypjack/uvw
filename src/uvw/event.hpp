#pragma once


#include <cstddef>
#include <uv.h>
#include "util.hpp"


namespace uvw {


struct BaseEvent {
    virtual ~BaseEvent() = 0;
    static std::size_t next() noexcept {
        static std::size_t cnt = 0;
        return cnt++;
    }
};

BaseEvent::~BaseEvent() { }

template<typename E>
struct Event: BaseEvent {
    static std::size_t type() noexcept {
        static std::size_t val = BaseEvent::next();
        return val;
    }
};


struct CheckEvent: Event<CheckEvent> { };
struct CloseEvent: Event<CloseEvent> { };
struct ConnectEvent: Event<ConnectEvent> { };

struct ErrorEvent: Event<ErrorEvent> {
    explicit ErrorEvent(int code = 0): ec(code) { }

    operator const char *() const noexcept { return uv_strerror(ec); }
    operator int() const noexcept { return ec; }

private:
    int ec;
};

struct IdleEvent: Event<IdleEvent> { };
struct ListenEvent: Event<ListenEvent> { };
struct PrepareEvent: Event<PrepareEvent> { };
struct ShutdownEvent: Event<ShutdownEvent> { };
struct TimerEvent: Event<TimerEvent> { };


}
