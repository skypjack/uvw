#pragma once


#include <new>
#include <memory>
#include <utility>
#include <type_traits>
#include <chrono>
#include <uv.h>
#include "emitter.hpp"
#include "util.hpp"


namespace uvw {


namespace details {


enum class UVLoopOption: std::underlying_type_t<uv_loop_option> {
    BLOCK_SIGNAL = UV_LOOP_BLOCK_SIGNAL
};


enum class UVRunMode: std::underlying_type_t<uv_run_mode> {
    DEFAULT = UV_RUN_DEFAULT,
    ONCE = UV_RUN_ONCE,
    NOWAIT = UV_RUN_NOWAIT
};


}


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
    using Deleter = void(*)(uv_loop_t *);

    template<typename, typename>
    friend class Resource;

    Loop(std::unique_ptr<uv_loop_t, Deleter> ptr) noexcept
        : loop{std::move(ptr)}
    { }

public:
    using Time = std::chrono::milliseconds;
    using Configure = details::UVLoopOption;
    using Mode = details::UVRunMode;

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

    ~Loop() noexcept {
        if(loop) {
            close();
        }
    }

    template<typename... Args>
    void configure(Configure flag, Args... args) {
        auto err = uv_loop_configure(loop.get(), static_cast<std::underlying_type_t<Configure>>(flag), std::forward<Args>(args)...);
        if(err) { publish(ErrorEvent{err}); }
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

    void close() {
        auto err = uv_loop_close(loop.get());
        if(err) { publish(ErrorEvent{err}); }
    }

    template<Mode mode = Mode::DEFAULT>
    bool run() noexcept {
        auto utm = static_cast<std::underlying_type_t<Mode>>(mode);
        auto uvrm = static_cast<uv_run_mode>(utm);
        return (uv_run(loop.get(), uvrm) == 0);
    }

    bool alive() const noexcept {
        return !(uv_loop_alive(loop.get()) == 0);
    }

    void stop() noexcept {
        uv_stop(loop.get());
    }

    int descriptor() const noexcept {
        return uv_backend_fd(loop.get());
    }

    Time timeout() const noexcept {
        return Time{uv_backend_timeout(loop.get())};
    }

    Time now() const noexcept {
        return Time{uv_now(loop.get())};
    }

    void update() const noexcept {
        return uv_update_time(loop.get());
    }

    void walk(std::function<void(BaseHandle &)> callback) {
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
