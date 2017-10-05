#pragma once


#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <uv.h>
#include "loop.hpp"
#include "underlying_type.hpp"


namespace uvw {


class Thread;
class ThreadLocalStorage;
class Once;
class Mutex;
class RWLock;
class Semaphore;
class Condition;
class Barrier;


class Thread final: public UnderlyingType<Thread, uv_thread_t> {
    using InternalTask = std::function<void(std::shared_ptr<void>)>;

    static void createCallback(void *arg) {
        Thread &thread = *(static_cast<Thread*>(arg));
        thread.task(thread.data);
    }

public:
    using Task = InternalTask;
    using Type = uv_thread_t;

    explicit Thread(ConstructorAccess ca, std::shared_ptr<Loop> ref, InternalTask t, std::shared_ptr<void> d = nullptr) noexcept
        : UnderlyingType{ca, std::move(ref)}, data{std::move(d)}, task{std::move(t)}
    {}

    static Type self() noexcept {
        return uv_thread_self();
    }

    static bool equal(const Thread &tl, const Thread &tr) noexcept {
        return !(0 == uv_thread_equal(tl.get(), tr.get()));
    }

    ~Thread() noexcept {
        join();
    }

    bool run() noexcept {
        return (0 == uv_thread_create(get(), &createCallback, this));
    }

    bool join() noexcept {
        return (0 == uv_thread_join(get()));
    }

private:
    std::shared_ptr<void> data;
    Task task;
};


class ThreadLocalStorage final: public UnderlyingType<ThreadLocalStorage, uv_key_t> {
public:
    explicit ThreadLocalStorage(ConstructorAccess ca, std::shared_ptr<Loop> ref) noexcept
        : UnderlyingType{ca, std::move(ref)}
    {
        uv_key_create(UnderlyingType::get());
    }

    ~ThreadLocalStorage() noexcept {
        uv_key_delete(UnderlyingType::get());
    }

    template<typename T>
    T* get() noexcept {
        return static_cast<T*>(uv_key_get(UnderlyingType::get()));
    }

    template<typename T>
    void set(T *value) noexcept {
        return uv_key_set(UnderlyingType::get(), value);
    }
};


// `Once` is an odd one as it doesn't use a `libuv` structure per object.
class Once final: public UnderlyingType<Once, uv_once_t> {
    static uv_once_t* guard() noexcept {
        static uv_once_t once = UV_ONCE_INIT;
        return &once;
    }

public:
    using UnderlyingType::UnderlyingType;

    template<typename F>
    static void once(F &&f) noexcept {
        using CallbackType = void (*)(void);
        static_assert(std::is_convertible<F, CallbackType>::value, "!");
        CallbackType cb = f;
        uv_once(guard(), cb);
    }
};


class Mutex final: public UnderlyingType<Mutex, uv_mutex_t> {
    friend class Condition;

public:
    explicit Mutex(ConstructorAccess ca, std::shared_ptr<Loop> ref, bool recursive = false) noexcept
        : UnderlyingType{ca, std::move(ref)}
    {
        if(recursive) {
            uv_mutex_init_recursive(get());
        } else {
            uv_mutex_init(get());
        }
    }

    ~Mutex() noexcept {
        uv_mutex_destroy(get());
    }

    void lock() noexcept {
        uv_mutex_lock(get());
    }

    bool tryLock() noexcept {
        return (0 == uv_mutex_trylock(get()));
    }

    void unlock() noexcept {
        uv_mutex_unlock(get());
    }
};


class RWLock final: public UnderlyingType<RWLock, uv_rwlock_t> {
public:
    explicit RWLock(ConstructorAccess ca, std::shared_ptr<Loop> ref) noexcept
        : UnderlyingType{ca, std::move(ref)}
    {
        uv_rwlock_init(get());
    }

    ~RWLock() noexcept {
        uv_rwlock_destroy(get());
    }

    void rdLock() noexcept {
        uv_rwlock_rdlock(get());
    }

    bool tryRdLock() noexcept {
        return (0 == uv_rwlock_tryrdlock(get()));
    }

    void rdUnlock() noexcept {
        uv_rwlock_rdunlock(get());
    }

    void wrLock() noexcept {
        uv_rwlock_wrlock(get());
    }

    bool tryWrLock() noexcept {
        return (0 == uv_rwlock_trywrlock(get()));
    }

    void wrUnlock() noexcept {
        uv_rwlock_wrunlock(get());
    }
};


class Semaphore final: public UnderlyingType<Semaphore, uv_sem_t> {
public:
    explicit Semaphore(ConstructorAccess ca, std::shared_ptr<Loop> ref, unsigned int value) noexcept
        : UnderlyingType{ca, std::move(ref)}
    {
        uv_sem_init(get(), value);
    }

    ~Semaphore() noexcept {
        uv_sem_destroy(get());
    }

    void post() noexcept {
        uv_sem_post(get());
    }

    void wait() noexcept {
        uv_sem_wait(get());
    }

    bool tryWait() noexcept {
        return (0 == uv_sem_trywait(get()));
    }
};


class Condition final: public UnderlyingType<Condition, uv_cond_t> {
public:
    explicit Condition(ConstructorAccess ca, std::shared_ptr<Loop> ref) noexcept
        : UnderlyingType{ca, std::move(ref)}
    {
        uv_cond_init(get());
    }

    ~Condition() noexcept {
        uv_cond_destroy(get());
    }

    void signal() noexcept {
        uv_cond_signal(get());
    }

    void broadcast() noexcept {
        uv_cond_broadcast(get());
    }

    void wait(Mutex &mutex) noexcept {
        uv_cond_wait(get(), mutex.get());
    }

    bool timedWait(Mutex &mutex, uint64_t timeout) noexcept {
        return (0 == uv_cond_timedwait(get(), mutex.get(), timeout));
    }
};


class Barrier final: public UnderlyingType<Barrier, uv_barrier_t> {
public:
    explicit Barrier(ConstructorAccess ca, std::shared_ptr<Loop> ref, unsigned int count) noexcept
        : UnderlyingType{ca, std::move(ref)}
    {
        uv_barrier_init(get(), count);
    }

    ~Barrier() noexcept {
        uv_barrier_destroy(get());
    }

    bool wait() noexcept {
        return (0 == uv_barrier_wait(get()));
    }
};


}
