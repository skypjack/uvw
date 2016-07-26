#pragma once


#include <type_traits>
#include <functional>
#include <utility>
#include <cstddef>
#include <vector>
#include <memory>
#include <tuple>
#include <list>
#include "event.hpp"


namespace uvw {


template<typename T>
class Emitter {
    struct BaseHandler {
        virtual ~BaseHandler() = default;
        virtual bool empty() const noexcept = 0;
    };

    template<typename E>
    struct Handler: BaseHandler {
        using Listener = std::function<void(const E &, T &)>;
        using ListenerList = std::list<Listener>;
        using ListenerIt = typename ListenerList::const_iterator;
        using Connection = std::pair<ListenerList &, ListenerIt>;

        bool empty() const noexcept override {
            return onceL.empty() && onL.empty();
        }

        Connection once(Listener f) {
            auto conn = onceL.insert(onceL.cbegin(), std::move(f));
            return { onceL, std::move(conn) };
        }

        Connection on(Listener f) {
            auto conn = onL.insert(onL.cbegin(), std::move(f));
            return { onL, std::move(conn) };
        }

        void erase(Connection conn) noexcept {
            conn.first.erase(conn.second);
        }

        void clear() noexcept {
            onceL.clear();
            onL.clear();
        }

        void publish(const E &event, T &ref) {
            for(auto &&listener: onceL) { listener(event, ref); }
            for(auto &&listener: onL) { listener(event, ref); }
            onceL.clear();
        }

    private:
        ListenerList onceL{};
        ListenerList onL{};
    };

    template<typename E>
    Handler<E> & handler() noexcept {
        static_assert(std::is_base_of<Event<E>, E>::value, "!");

        std::size_t type = E::type();

        if(!(type < handlers.size())) {
            handlers.resize(type+1);
        }

        if(!handlers[type]) {
           handlers[type] = std::make_unique<Handler<E>>();
        }

        return static_cast<Handler<E>&>(*handlers[type]);
    }

protected:
    template<typename E>
    void publish(E event) {
        handler<E>().publish(event, *static_cast<T*>(this));
    }

public:
    template<typename E>
    using Listener = typename Handler<E>::Listener;

    template<typename E>
    struct Connection: private Handler<E>::Connection {
        template<typename> friend class Emitter;
        Connection(typename Handler<E>::Connection conn)
            : Handler<E>::Connection{std::move(conn)}
        { }
    };

    virtual ~Emitter() noexcept {
        static_assert(std::is_base_of<Emitter<T>, T>::value, "!");
    }

    template<typename E>
    Connection<E> on(Listener<E> f) {
        return handler<E>().on(std::move(f));
    }

    template<typename E>
    Connection<E> once(Listener<E> f) {
        return handler<E>().once(std::move(f));
    }

    template<typename E>
    void erase(Connection<E> conn) noexcept {
        handler<E>().erase(std::move(conn));
    }

    template<typename E>
    void clear() noexcept {
        handler<E>().clear();
    }

    bool empty() const noexcept {
        bool empty = true;

        for(auto &&h: handlers) {
            empty = !h || h->empty();
            if(!empty) { break; }
        }

        return empty;
    }

private:
    std::vector<std::unique_ptr<BaseHandler>> handlers{};
};


}
