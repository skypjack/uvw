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
    const ssize_t len;
};


struct EndEvent: Event<EndEvent> { };


struct ErrorEvent: Event<ErrorEvent> {
    explicit ErrorEvent(int code = 0) noexcept: ec(code) { }

    const char * what() const noexcept { return uv_strerror(ec); }
    int code() const noexcept { return ec; }

private:
    const int ec;
};


template<typename E>
struct FlagsEvent: Event<FlagsEvent<E>> {
    explicit FlagsEvent(Flags<E> f) noexcept: flgs{std::move(f)} { }

    Flags<E> flags() const noexcept { return flgs; }

private:
    Flags<E> flgs;
};


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


struct IdleEvent: Event<IdleEvent> { };
struct ListenEvent: Event<ListenEvent> { };
struct PrepareEvent: Event<PrepareEvent> { };
struct SendEvent: Event<SendEvent> { };
struct ShutdownEvent: Event<ShutdownEvent> { };


struct SignalEvent: Event<SignalEvent> {
    explicit SignalEvent(int sig) noexcept: signum(sig) { }

    int signal() const noexcept { return signum; }

private:
    const int signum;
};


struct TimerEvent: Event<TimerEvent> { };


struct UDPDataEvent: Event<UDPDataEvent> {
    explicit UDPDataEvent(Addr addr, std::unique_ptr<const char[]> ptr, ssize_t l, bool trunc) noexcept
        : dt{std::move(ptr)}, len{l}, sndr{addr}, part{trunc}
    { }

    const char * data() const noexcept { return dt.get(); }
    ssize_t length() const noexcept { return len; }
    Addr sender() const noexcept { return sndr; }
    bool partial() const noexcept { return part; }

private:
    std::unique_ptr<const char[]> dt;
    const ssize_t len;
    Addr sndr;
    const bool part;
};


struct UninitializedEvent: Event<UninitializedEvent> { };
struct WorkEvent: Event<WorkEvent> { };
struct WriteEvent: Event<WriteEvent> { };


}
