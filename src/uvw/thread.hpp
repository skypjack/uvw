#pragma once


#include <utility>
#include <memory>
#include <string>
#include <uv.h>
#include "loop.hpp"


namespace uvw {


class Thread final {
    using InternalTask = std::function<void(std::shared_ptr<void>)>;

    static void createCallback(void *arg) {
        Thread &thread = *(static_cast<Thread*>(arg));
        thread.task(thread.data);
    }

    explicit Thread(std::shared_ptr<Loop> ref, InternalTask t, std::shared_ptr<void> d = nullptr) noexcept
        : pLoop{std::move(ref)},
          data{std::move(d)},
          thread{},
          task{std::move(t)},
          err{0}
    { }

public:
    using Task = InternalTask;
    using Type = uv_thread_t;

    template<typename... Args>
    static std::shared_ptr<Thread> create(Args&&... args) {
        return std::shared_ptr<Thread>{new Thread{std::forward<Args>(args)...}};
    }

    static Type self() noexcept {
        return uv_thread_self();
    }

    static bool equal(const Thread &tl, const Thread &tr) noexcept {
        return !(0 == uv_thread_equal(&tl.thread, &tr.thread));
    }

    ~Thread() {
        uv_thread_join(&thread);
    }

    bool run() noexcept {
        err = uv_thread_create(&thread, &createCallback, this);
        return static_cast<bool>(*this);
    }

    bool join() noexcept {
        err = uv_thread_join(&thread);
        return static_cast<bool>(*this);
    }

    explicit operator bool() const noexcept { return (0 == err); }

    int error() const noexcept { return err; }

    Loop& loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<Loop> pLoop;
    std::shared_ptr<void> data;
    uv_thread_t thread;
    Task task;
    int err;
};


// TODO Thread-local storage
// TODO Once-only initialization
// TODO Mutex locks
// TODO Read-write locks
// TODO Semaphores
// TODO Conditions
// TODO Barriers


}
