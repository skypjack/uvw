#pragma once


#include <cstddef>
#include <memory>
#include <uv.h>


namespace uvw {


namespace details {


struct BaseEvent {
    virtual ~BaseEvent() noexcept {}

    static std::size_t next() noexcept {
        static std::size_t cnt = 0;
        return cnt++;
    }
};


}


/**
 * @brief Generic event type.
 *
 * Events in `uvw` must be associated with an unique numerical identifier. To do
 * that, they shall inherit from this class.
 */
template<typename E>
struct Event: details::BaseEvent {
    /**
     * @brief Gets the unique numerical identifier.
     * @return An unique numerical identifier for the given event type.
     */
    static std::size_t type() noexcept {
        static std::size_t val = BaseEvent::next();
        return val;
    }
};


/**
 * @brief The ErrorEvent event.
 *
 * Custom wrapper around libuv's error constants.
 */
struct ErrorEvent: Event<ErrorEvent> {
    template<typename U, typename = std::enable_if_t<std::is_integral<U>::value>>
    explicit ErrorEvent(U val) noexcept
        : ec{static_cast<int>(val)}, str{uv_strerror(ec)}
    {}

    /**
     * @brief Returns the error message for the given error code.
     *
     * Leaks a few bytes of memory when you call it with an unknown error code.
     *
     * @return The error message for the given error code.
     */
    const char * what() const noexcept { return str; }

    /**
     * @brief Gets the underlying error code, that is a libuv's error constant.
     * @return The underlying error code.
     */
    int code() const noexcept { return ec; }

    /**
     * @brief Checks if the event contains a valid error code.
     * @return True in case of success, false otherwise.
     */
    explicit operator bool() const noexcept { return ec < 0; }

private:
    const int ec;
    const char *str;
};


}
