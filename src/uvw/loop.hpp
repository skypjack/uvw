#pragma once


#include <new>
#include <memory>
#include <utility>
#include <type_traits>
#include <uv.h>
#include "util.hpp"


namespace uvw {


class Loop;


template<typename R>
class Handle {
    template<typename>
    friend class Handle;

    friend class Loop;

    template<typename... Args>
    explicit constexpr Handle(std::shared_ptr<Loop>&& l, Args&&... args)
        : res{R::create(std::move(l), std::forward<Args>(args)...)}
    { }

    explicit constexpr Handle(std::shared_ptr<R> ptr): res{std::move(ptr)} { }

public:
    explicit constexpr Handle(): res{} { }

    template<typename T, std::enable_if_t<std::is_base_of<R, T>::value>* = nullptr>
    constexpr Handle(const Handle<T> &other): res{other.res} { }

    template<typename T, std::enable_if_t<std::is_base_of<R, T>::value>* = nullptr>
    constexpr Handle(Handle<T> &&other): res{std::move(other.res)} { }

    template<typename T, std::enable_if_t<std::is_base_of<R, T>::value>* = nullptr>
    constexpr void operator=(const Handle<T> &other) { res = other.res; }

    template<typename T, std::enable_if_t<std::is_base_of<R, T>::value>* = nullptr>
    constexpr void operator=(Handle<T> &&other) { res = std::move(other.res); }

    constexpr explicit operator bool() const { return static_cast<bool>(res); }

    constexpr operator R&() noexcept { return *res; }
    constexpr operator const R&() const noexcept { return *res; }

private:
    std::shared_ptr<R> res;
};


class Loop final: public std::enable_shared_from_this<Loop> {
    template<typename>
    friend class Resource;

    using Deleter = std::function<void(uv_loop_t *)>;

    Loop(std::unique_ptr<uv_loop_t, Deleter> ptr): loop{std::move(ptr)} { }

public:
    static std::shared_ptr<Loop> create() {
        auto ptr = std::unique_ptr<uv_loop_t, Deleter>{new uv_loop_t, [](uv_loop_t *l){ delete l; }};
        auto loop = std::shared_ptr<Loop>(new Loop{std::move(ptr)});

        if(uv_loop_init(loop->loop.get())) {
            loop = nullptr;
        }

        return loop;
    }

    static std::shared_ptr<Loop> getDefault() {
        static std::weak_ptr<Loop> ref;
        std::shared_ptr<Loop> loop;

        if(ref.expired()) {
            auto def = uv_default_loop();

            if(def) {
                auto ptr = std::unique_ptr<uv_loop_t, Deleter>(def, [](uv_loop_t *){ });
                loop = std::shared_ptr<Loop>(new Loop{std::move(ptr)});
            }

            ref = loop;
        } else {
            loop = ref.lock();
        }

        return loop;
    }

    Loop(const Loop &) = delete;
    Loop(Loop &&other) = delete;
    Loop& operator=(const Loop &) = delete;
    Loop& operator=(Loop &&other) = delete;

    ~Loop() {
        if(loop) {
            close();
        }
    }

    template<typename R>
    Handle<R> handle(std::shared_ptr<R> ptr) {
        return Handle<R>{std::move(ptr)};
    }

    template<typename R, typename... Args>
    Handle<R> handle(Args&&... args) {
        return Handle<R>{shared_from_this(), std::forward<Args>(args)...};
    }

    UVWError close() noexcept {
        return UVWError{uv_loop_close(loop.get())};
    }

    bool run() noexcept {
        return (uv_run(loop.get(), UV_RUN_DEFAULT) == 0);
    }

    bool runOnce() noexcept {
        return (uv_run(loop.get(), UV_RUN_ONCE) == 0);
    }

    bool runWait() noexcept {
        return (uv_run(loop.get(), UV_RUN_NOWAIT) == 0);
    }

    bool alive() const noexcept {
        return !(uv_loop_alive(loop.get()) == 0);
    }

    void stop() noexcept {
        uv_stop(loop.get());
    }

private:
    std::unique_ptr<uv_loop_t, Deleter> loop;
};


}
