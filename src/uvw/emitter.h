#ifndef UVW_EMITTER_INCLUDE_H
#define UVW_EMITTER_INCLUDE_H

#include <cstddef>
#include <cstdint>
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
    template<typename Type, typename = std::enable_if_t<std::is_integral_v<Type>>>
    explicit error_event(Type val) noexcept
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
template<typename Elem, typename... Event>
class emitter {
public:
    template<typename Type>
    using listener_t = std::function<void(Type &, Elem &)>;

private:
    template<typename Type>
    const auto &handler() const noexcept {
        return std::get<listener_t<Type>>(handlers);
    }

    template<typename Type>
    auto &handler() noexcept {
        return std::get<listener_t<Type>>(handlers);
    }

protected:
    template<typename Type>
    void publish(Type event) {
        if(auto &listener = handler<Type>(); listener) {
            listener(event, *static_cast<Elem *>(this));
        }
    }

public:
    virtual ~emitter() noexcept {
        static_assert(std::is_base_of_v<emitter<Elem, Event...>, Elem>);
    }

    /**
     * @brief Registers a long-lived listener with the event emitter.
     *
     * This method is used to register a listener with the emitter.<br/>
     * A listener is usually defined as a callable object assignable to a
     * `std::function<void(const Event &, Elem &)`, where `Event` is the type of
     * the event and `Elem` is the type of the resource.
     *
     * @param f A valid listener to be registered.
     */
    template<typename Type>
    void on(listener_t<Type> f) {
        handler<Type>() = std::move(f);
    }

    /*! @brief Disconnects the listener for the given event type. */
    template<typename Type>
    void reset() noexcept {
        handler<Type>() = nullptr;
    }

    /*! @brief Disconnects all listeners. */
    void reset() noexcept {
        reset<error_event>();
        (reset<Event>(), ...);
    }

    /**
     * @brief Checks if there is a listener registered for the specific event.
     * @return True if there is a listener registered for the specific event,
     * false otherwise.
     */
    template<typename Type>
    bool has() const noexcept {
        return static_cast<bool>(handler<Type>());
    }

private:
    std::tuple<listener_t<error_event>, listener_t<Event>...> handlers{};
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "emitter.cpp"
#endif

#endif // UVW_EMITTER_INCLUDE_H
