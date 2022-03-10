#ifndef UVW_EMITTER_INCLUDE_H
#define UVW_EMITTER_INCLUDE_H

#include <algorithm>
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
        virtual bool empty() const UVW_NOEXCEPT = 0;
        virtual void clear() UVW_NOEXCEPT = 0;
    };

    template<typename E>
    struct event_handler final: base_handler {
        using listener = std::function<void(E &, T &)>;
        using listener_list = std::list<std::pair<bool, listener>>;
        using connection = typename listener_list::iterator;

        bool empty() const UVW_NOEXCEPT override {
            auto pred = [](auto &&elem) { return elem.first; };
            return std::all_of(once_list.cbegin(), once_list.cend(), pred) && std::all_of(on_list.cbegin(), on_list.cend(), pred);
        }

        void clear() UVW_NOEXCEPT override {
            if(publishing) {
                auto func = [](auto &&elem) { elem.first = true; };
                std::for_each(once_list.begin(), once_list.end(), func);
                std::for_each(on_list.begin(), on_list.end(), func);
            } else {
                once_list.clear();
                on_list.clear();
            }
        }

        connection once(listener f) {
            return once_list.emplace(once_list.cend(), false, std::move(f));
        }

        connection on(listener f) {
            return on_list.emplace(on_list.cend(), false, std::move(f));
        }

        void erase(connection conn) UVW_NOEXCEPT {
            conn->first = true;

            if(!publishing) {
                auto pred = [](auto &&elem) { return elem.first; };
                once_list.remove_if(pred);
                on_list.remove_if(pred);
            }
        }

        void publish(E event, T &ref) {
            listener_list curr;
            once_list.swap(curr);

            auto func = [&event, &ref](auto &&elem) {
                return elem.first ? void() : elem.second(event, ref);
            };

            publishing = true;

            std::for_each(on_list.rbegin(), on_list.rend(), func);
            std::for_each(curr.rbegin(), curr.rend(), func);

            publishing = false;

            on_list.remove_if([](auto &&elem) { return elem.first; });
        }

    private:
        bool publishing{false};
        listener_list once_list{};
        listener_list on_list{};
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
    template<typename E>
    using listener = typename event_handler<E>::listener;

    /**
     * @brief Connection type for a given event type.
     *
     * Given an event type `E`, `connection<E>` is the type of the connection
     * object returned by the event emitter whenever a listener for the given
     * type is registered.
     */
    template<typename E>
    struct connection: private event_handler<E>::connection {
        template<typename>
        friend class emitter;

        connection() = default;
        connection(const connection &) = default;
        connection(connection &&) = default;

        connection(typename event_handler<E>::connection conn)
            : event_handler<E>::connection{std::move(conn)} {}

        connection &operator=(const connection &) = default;
        connection &operator=(connection &&) = default;
    };

    virtual ~emitter() UVW_NOEXCEPT {
        static_assert(std::is_base_of_v<emitter<T>, T>);
    }

    /**
     * @brief Registers a long-lived listener with the event emitter.
     *
     * This method can be used to register a listener that is meant to be
     * invoked more than once for the given event type.<br/>
     * The connection object returned by the method can be freely discarded. It
     * can be used later to disconnect the listener, if needed.
     *
     * A listener is usually defined as a callable object assignable to a
     * `std::function<void(const E &, T &)`, where `E` is the type of the event
     * and `T` is the type of the resource.
     *
     * @param f A valid listener to be registered.
     * @return Connection object to be used later to disconnect the listener.
     */
    template<typename E>
    connection<E> on(listener<E> f) {
        return handler<E>().on(std::move(f));
    }

    /**
     * @brief Registers a short-lived listener with the event emitter.
     *
     * This method can be used to register a listener that is meant to be
     * invoked only once for the given event type.<br/>
     * The connection object returned by the method can be freely discarded. It
     * can be used later to disconnect the listener, if needed.
     *
     * a listener is usually defined as a callable object assignable to a
     * `std::function<void(const E &, T &)`, where `E` is the type of the event
     * and `T` is the type of the resource.
     *
     * @param f A valid listener to be registered.
     * @return Connection object to be used later to disconnect the listener.
     */
    template<typename E>
    connection<E> once(listener<E> f) {
        return handler<E>().once(std::move(f));
    }

    /**
     * @brief Disconnects a listener from the event emitter.
     * @param conn A valid connection object
     */
    template<typename E>
    void erase(connection<E> conn) UVW_NOEXCEPT {
        handler<E>().erase(std::move(conn));
    }

    /**
     * @brief Disconnects all the listeners for the given event type.
     */
    template<typename E>
    void clear() UVW_NOEXCEPT {
        handler<E>().clear();
    }

    /**
     * @brief Disconnects all the listeners.
     */
    void clear() UVW_NOEXCEPT {
        std::for_each(handlers.begin(), handlers.end(), [](auto &&hdlr) { if(hdlr.second) { hdlr.second->clear(); } });
    }

    /**
     * @brief Checks if there are listeners registered for the specific event.
     * @return True if there are no listeners registered for the specific event,
     * false otherwise.
     */
    template<typename E>
    bool empty() const UVW_NOEXCEPT {
        const auto id = type<E>();
        return (!handlers.count(id) || static_cast<event_handler<E> &>(*handlers.at(id)).empty());
    }

    /**
     * @brief Checks if there are listeners registered with the event emitter.
     * @return True if there are no listeners registered with the event emitter,
     * false otherwise.
     */
    bool empty() const UVW_NOEXCEPT {
        return std::all_of(handlers.cbegin(), handlers.cend(), [](auto &&hdlr) { return !hdlr.second || hdlr.second->empty(); });
    }

private:
    std::unordered_map<std::uint32_t, std::unique_ptr<base_handler>> handlers{};
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "emitter.cpp"
#endif

#endif // UVW_EMITTER_INCLUDE_H
