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


struct ErrorEvent: Event<ErrorEvent> {
    explicit ErrorEvent(int code = 0) noexcept: ec(code) { }

    const char * what() const noexcept { return uv_strerror(ec); }
    int code() const noexcept { return ec; }

private:
    const int ec;
};


}
