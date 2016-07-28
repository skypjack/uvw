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
    template<typename U, typename = std::enable_if_t<std::is_integral<U>::value>>
    explicit ErrorEvent(U val) noexcept
        : ec{static_cast<int>(val)}, str{uv_strerror(ec)}
    { }

    const char * what() const noexcept { return str; }
    int code() const noexcept { return ec; }

    explicit operator bool() const noexcept { return ec < 0; }

private:
    const int ec;
    const char *str;
};


template<typename E, E e>
struct TypedEvent;


}
