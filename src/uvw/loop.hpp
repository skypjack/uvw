#pragma once


#include <new>
#include <memory>
#include <utility>
#include <uv.h>
#include <type_traits>
#include "error.hpp"


namespace uvw {


class Loop;


template<typename R>
class Handle {
    friend class Loop;

    template<typename... Args>
    explicit constexpr Handle(uv_loop_t *loop, Args&&... args)
        : res{std::make_shared<R>(loop, std::forward<Args>(args)...)}
    { }

public:
    constexpr Handle(const Handle &other): res{other.res} { }
    constexpr Handle(Handle &&other): res{std::move(other.res)} { }

    constexpr void operator=(const Handle &other) { res = other.res; }
    constexpr void operator=(Handle &&other) { res = std::move(other.res); }

    constexpr operator R&() noexcept { return *res; }
    operator const R&() const noexcept { return *res; }

private:
    std::shared_ptr<R> res;
};


class Loop final {
public:
    Loop(bool def = true)
        : loop{def ? uv_default_loop() : new uv_loop_t, [def](uv_loop_t *l){ if(!def) delete l; }}
    {
        if(!def) {
            auto err = uv_loop_init(loop.get());

            if(err) {
                throw UVWException{err};
            }
        } else if(!loop) {
            throw std::bad_alloc{};
        }
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

    template<typename R, typename... Args>
    Handle<R> handle(Args&&... args) {
        return Handle<R>{loop.get(), std::forward<Args>(args)...};
    }

    bool close() noexcept {
        return (uv_loop_close(loop.get()) == 0);
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
    using Deleter = std::function<void(uv_loop_t *)>;
    std::unique_ptr<uv_loop_t, Deleter> loop;
};


}
