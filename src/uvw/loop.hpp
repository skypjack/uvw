#pragma once


#include <new>
#include <memory>
#include <utility>
#include <type_traits>
#include <uv.h>
#include "emitter.hpp"
#include "util.hpp"


namespace uvw {


class BaseHandle {
public:
    virtual bool active() const noexcept = 0;
    virtual bool closing() const noexcept = 0;
    virtual void reference() noexcept = 0;
    virtual void unreference() noexcept = 0;
    virtual bool referenced() const noexcept = 0;
    virtual void close() noexcept = 0;
};


class Loop final: public Emitter<Loop>, public std::enable_shared_from_this<Loop> {
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

    template<typename R, typename... Args>
    std::enable_if_t<std::is_base_of<BaseHandle, R>::value, std::shared_ptr<R>>
    resource(Args&&... args) {
        auto ptr = R::create(shared_from_this(), std::forward<Args>(args)...);
        ptr = ptr->init() ? ptr : nullptr;
        return ptr;
    }

    template<typename R, typename... Args>
    std::enable_if_t<not std::is_base_of<BaseHandle, R>::value, std::shared_ptr<R>>
    resource(Args&&... args) {
        return R::create(shared_from_this(), std::forward<Args>(args)...);
    }

    void close() noexcept {
        auto err = uv_loop_close(loop.get());
        if(err) { publish(ErrorEvent{err}); }
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

    void walk(std::function<void(BaseHandle &)> callback) noexcept {
        // remember: non-capturing lambdas decay to pointers to functions
        uv_walk(loop.get(), [](uv_handle_t *handle, void *func) {
            BaseHandle &ref = *static_cast<BaseHandle*>(handle->data);
            std::function<void(BaseHandle &)> &f =
                    *static_cast<std::function<void(BaseHandle &)>*>(func);
            f(ref);
        }, &callback);
    }

private:
    std::unique_ptr<uv_loop_t, Deleter> loop;
};


}
