#pragma once


#include <cstddef>
#include <memory>
#include <uv.h>
#include "util.hpp"


namespace uvw {


struct BaseEvent {
    virtual ~BaseEvent() noexcept = 0;

    static std::size_t next() noexcept {
        static std::size_t cnt = 0;
        return cnt++;
    }
};

BaseEvent::~BaseEvent() noexcept { }

template<typename E>
struct Event: BaseEvent {
    static std::size_t type() noexcept {
        static std::size_t val = BaseEvent::next();
        return val;
    }
};


struct AsyncEvent: Event<AsyncEvent> { };
struct CheckEvent: Event<CheckEvent> { };
struct CloseEvent: Event<CloseEvent> { };
struct ConnectEvent: Event<ConnectEvent> { };


struct DataEvent: Event<DataEvent> {
    explicit DataEvent(std::unique_ptr<const char[]> ptr, ssize_t l) noexcept
        : dt{std::move(ptr)}, len{l}
    { }

    const char * data() const noexcept { return dt.get(); }
    ssize_t length() const noexcept { return len; }

private:
    std::unique_ptr<const char[]> dt;
    ssize_t len;
};


struct EndEvent: Event<EndEvent> { };


struct ErrorEvent: Event<ErrorEvent> {
    explicit ErrorEvent(int code = 0) noexcept: ec(code) { }

    operator const char *() const noexcept { return uv_strerror(ec); }
    operator int() const noexcept { return ec; }

private:
    int ec;
};


template<typename E>
struct FlagsEvent: Event<FlagsEvent<E>> {
    explicit FlagsEvent(Flags<E> f) noexcept: flags{std::move(f)} { }

    operator Flags<E>() const noexcept { return flags; }

private:
    const Flags<E> flags;
};


struct FsPollEvent: Event<FsPollEvent> {
    explicit FsPollEvent(const Stat &p, const Stat &c) noexcept
        : prev(p), curr(c)
    { }

    const Stat & previous() const noexcept { return prev; }
    const Stat & current() const noexcept { return curr; }

private:
    const Stat prev;
    const Stat curr;
};


struct IdleEvent: Event<IdleEvent> { };
struct ListenEvent: Event<ListenEvent> { };
struct PrepareEvent: Event<PrepareEvent> { };
struct ShutdownEvent: Event<ShutdownEvent> { };


struct SignalEvent: Event<SignalEvent> {
    explicit SignalEvent(int sig) noexcept: signum(sig) { }

    operator int() const noexcept { return signum; }

private:
    int signum;
};


struct TimerEvent: Event<TimerEvent> { };
struct UninitializedEvent: Event<UninitializedEvent> { };
struct WorkEvent: Event<WorkEvent> { };
struct WriteEvent: Event<WriteEvent> { };


}
