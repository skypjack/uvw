#pragma once


#include <cstddef>
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
struct ConnectEvent: Event<ConnectEvent> { UVWError error; };
struct IdleEvent: Event<IdleEvent> { };
struct ListenEvent: Event<ListenEvent> { UVWError error; };
struct PrepareEvent: Event<PrepareEvent> { };
struct ShutdownEvent: Event<ShutdownEvent> { UVWError error; };
struct TimerEvent: Event<TimerEvent> { };


}
