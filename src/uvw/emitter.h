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
    template<typename U, typename = std::enable_if_t<std::is_integral_v<U>>>
    explicit error_event(U val) UVW_NOEXCEPT
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
    static int translate(int sys) UVW_NOEXCEPT;

    /**
     * @brief Returns the error message for the given error code.
     *
     * Leaks a few bytes of memory when you call it with an unknown error code.
     *
     * @return The error message for the given error code.
     */
    const char *what() const UVW_NOEXCEPT;

    /**
     * @brief Returns the error name for the given error code.
     *
     * Leaks a few bytes of memory when you call it with an unknown error code.
     *
     * @return The error name for the given error code.
     */
    const char *name() const UVW_NOEXCEPT;

    /**
     * @brief Gets the underlying error code, that is an error constant of `libuv`.
     * @return The underlying error code.
     */
    int code() const UVW_NOEXCEPT;

    /**
     * @brief Checks if the event contains a valid error code.
     * @return True in case of success, false otherwise.
     */
    explicit operator bool() const UVW_NOEXCEPT;

private:
    const int ec;
};

/**
 * @brief Event emitter base class.
 *
 * Almost everything in `uvw` is an event emitter.<br/>
 * This is the base class from which resources and loops inherit.
 */
template<typename T>
class emitter {
    struct base_handler {
        virtual ~base_handler() UVW_NOEXCEPT = default;
        virtual bool has() const UVW_NOEXCEPT = 0;
        virtual void reset() UVW_NOEXCEPT = 0;
    };

    template<typename E>
    struct event_handler final: base_handler {
        bool has() const UVW_NOEXCEPT override {
            return static_cast<bool>(listener);
        }

        void reset() UVW_NOEXCEPT override {
            listener = nullptr;
        }

        void on(std::function<void(E &, T &)> func) {
            listener = std::move(func);
        }

        void publish(E event, T &ref) {
            if(listener) {
                listener(event, ref);
            }
        }

    private:
        std::function<void(E &, T &)> listener;
    };

    template<typename E>
    event_handler<E> &handler() UVW_NOEXCEPT {
        const auto id = type<E>();

        if(!handlers.count(id)) {
            handlers[id] = std::make_unique<event_handler<E>>();
        }

        return static_cast<event_handler<E> &>(*handlers.at(id));
    }

protected:
    template<typename E>
    void publish(E event) {
        handler<E>().publish(std::move(event), *static_cast<T *>(this));
    }

public:
    virtual ~emitter() UVW_NOEXCEPT {
        static_assert(std::is_base_of_v<emitter<T>, T>);
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
    template<typename E>
    void on(std::function<void(E &, T &)> f) {
        return handler<E>().on(std::move(f));
    }

    /**
     * @brief Disconnects the listener for the given event type.
     */
    template<typename E>
    void reset() UVW_NOEXCEPT {
        handler<E>().reset();
    }

    /**
     * @brief Disconnects all listeners.
     */
    void reset() UVW_NOEXCEPT {
        for(auto &&curr: handlers) {
            curr.second->reset();
        }
    }

    /**
     * @brief Checks if there is a listener registered for the specific event.
     * @return True if there is a listener registered for the specific event,
     * false otherwise.
     */
    template<typename E>
    bool has() const UVW_NOEXCEPT {
        const auto id = type<E>();
        return (handlers.count(id) && static_cast<event_handler<E> &>(*handlers.at(id)).has());
    }

private:
    std::unordered_map<std::uint32_t, std::unique_ptr<base_handler>> handlers{};
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "emitter.cpp"
#endif

#endif // UVW_EMITTER_INCLUDE_H
