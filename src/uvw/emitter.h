#ifndef UVW_EMITTER_INCLUDE_H
#define UVW_EMITTER_INCLUDE_H

#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <uv.h>
#include "config.h"
#include "type_info.hpp"

namespace uvw {

/**
 * @brief Error event.
 *
 * Custom wrapper around error constants of `libuv`.
 */
struct error_event {
    template<typename U, typename = std::enable_if_t<std::is_integral_v<U>>>
    explicit error_event(U val) noexcept
        : ec{static_cast<int>(val)} {}

    /**
     * @brief Returns the `libuv` error code equivalent to the given platform dependent error code.
     *
     * It returns:
     * * POSIX error codes on Unix (the ones stored in errno).
     * * Win32 error codes on Windows (those returned by GetLastError() or WSAGetLastError()).
     *
     * If `sys` is already a `libuv` error code, it is simply returned.
     *
     * @param sys A platform dependent error code.
     * @return The `libuv` error code equivalent to the given platform dependent error code.
     */
    static int translate(int sys) noexcept;

    /**
     * @brief Returns the error message for the given error code.
     *
     * Leaks a few bytes of memory when you call it with an unknown error code.
     *
     * @return The error message for the given error code.
     */
    const char *what() const noexcept;

    /**
     * @brief Returns the error name for the given error code.
     *
     * Leaks a few bytes of memory when you call it with an unknown error code.
     *
     * @return The error name for the given error code.
     */
    const char *name() const noexcept;

    /**
     * @brief Gets the underlying error code, that is an error constant of `libuv`.
     * @return The underlying error code.
     */
    int code() const noexcept;

    /**
     * @brief Checks if the event contains a valid error code.
     * @return True in case of success, false otherwise.
     */
    explicit operator bool() const noexcept;

private:
    const int ec;
};

/**
 * @brief Event emitter base class.
 *
 * Almost everything in `uvw` is an event emitter.<br/>
 * This is the base class from which resources and loops inherit.
 */
template<typename T, typename... E>
class emitter {
public:
    template<typename U>
    using listener_t = std::function<void(U &, T &)>;

private:
    template<typename U>
    const auto &handler() const noexcept {
        return std::get<listener_t<U>>(handlers);
    }

    template<typename U>
    auto &handler() noexcept {
        return std::get<listener_t<U>>(handlers);
    }

protected:
    template<typename U>
    void publish(U event) {
        if(auto &listener = handler<U>(); listener) {
            listener(event, *static_cast<T *>(this));
        }
    }

public:
    virtual ~emitter() noexcept {
        static_assert(std::is_base_of_v<emitter<T, E...>, T>);
    }

    /**
     * @brief Registers a long-lived listener with the event emitter.
     *
     * This method is used to register a listener with the emitter.<br/>
     * A listener is usually defined as a callable object assignable to a
     * `std::function<void(const E &, T &)`, where `E` is the type of the event
     * and `T` is the type of the resource.
     *
     * @param f A valid listener to be registered.
     */
    template<typename U>
    void on(listener_t<U> f) {
        handler<U>() = std::move(f);
    }

    /*! @brief Disconnects the listener for the given event type. */
    template<typename U>
    void reset() noexcept {
        handler<U>() = nullptr;
    }

    /*! @brief Disconnects all listeners. */
    void reset() noexcept {
        reset<error_event>();
        (reset<E>(), ...);
    }

    /**
     * @brief Checks if there is a listener registered for the specific event.
     * @return True if there is a listener registered for the specific event,
     * false otherwise.
     */
    template<typename U>
    bool has() const noexcept {
        return static_cast<bool>(handler<U>());
    }

private:
    std::tuple<listener_t<error_event>, listener_t<E>...> handlers{};
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "emitter.cpp"
#endif

#endif // UVW_EMITTER_INCLUDE_H
