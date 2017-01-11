#pragma once


#include <type_traits>
#include <utility>
#include <memory>
#include <string>
#include <uv.h>
#include "loop.hpp"


namespace uvw {


class Thread;
class ThreadLocalStorage;
class Once;
class Mutex;
class RWLock;
class Semaphore;
class Condition;
class Barrier;


class Thread final {
    using InternalTask = std::function<void(std::shared_ptr<void>)>;

    static void createCallback(void *arg) {
        Thread &thread = *(static_cast<Thread*>(arg));
        thread.task(thread.data);
    }

    explicit Thread(std::shared_ptr<Loop> ref, InternalTask t, std::shared_ptr<void> d = nullptr) noexcept
        : pLoop{std::move(ref)}, data{std::move(d)}, thread{}, task{std::move(t)}
    {}

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

    ~Thread() noexcept {
        uv_thread_join(&thread);
    }

    bool run() noexcept {
        return (0 == uv_thread_create(&thread, &createCallback, this));
    }

    bool join() noexcept {
        return (0 == uv_thread_join(&thread));
    }

    Loop& loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<Loop> pLoop;
    std::shared_ptr<void> data;
    uv_thread_t thread;
    Task task;
};


class ThreadLocalStorage final {
    explicit ThreadLocalStorage(std::shared_ptr<Loop> ref) noexcept
        : pLoop{std::move(ref)}
    {
        uv_key_create(&key);
    }

public:
    static std::shared_ptr<ThreadLocalStorage> create(std::shared_ptr<Loop> loop) {
        return std::shared_ptr<ThreadLocalStorage>{new ThreadLocalStorage{std::move(loop)}};
    }

    ~ThreadLocalStorage() noexcept {
        uv_key_delete(&key);
    }

    template<typename T>
    T* get() noexcept {
        return static_cast<T*>(uv_key_get(&key));
    }

    template<typename T>
    void set(T *value) noexcept {
        return uv_key_set(&key, value);
    }

    Loop& loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<Loop> pLoop;
    uv_key_t key;
};


class Once final {
    explicit Once(std::shared_ptr<Loop> ref) noexcept
        : pLoop{std::move(ref)}
    {}

    static uv_once_t* guard() noexcept {
        static uv_once_t once = UV_ONCE_INIT;
        return &once;
    }

public:
    static std::shared_ptr<Once> create(std::shared_ptr<Loop> loop) {
        return std::shared_ptr<Once>{new Once{std::move(loop)}};
    }

    template<typename F>
    static void once(F &&f) noexcept {
        using CallbackType = void (*)(void);
        static_assert(std::is_convertible<F, CallbackType>::value, "!");
        CallbackType cb = f;
        uv_once(guard(), cb);
    }

    Loop& loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<Loop> pLoop;
};


class Mutex final {
    friend class Condition;

    explicit Mutex(std::shared_ptr<Loop> ref) noexcept
        : pLoop{std::move(ref)}
    {
        uv_mutex_init(&mutex);
    }

public:
    static std::shared_ptr<Mutex> create(std::shared_ptr<Loop> loop) {
        return std::shared_ptr<Mutex>{new Mutex{std::move(loop)}};
    }

    ~Mutex() noexcept {
        uv_mutex_destroy(&mutex);
    }

    void lock() noexcept {
        uv_mutex_lock(&mutex);
    }

    bool tryLock() noexcept {
        return (0 == uv_mutex_trylock(&mutex));
    }

    void unlock() noexcept {
        uv_mutex_unlock(&mutex);
    }

    Loop& loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<Loop> pLoop;
    uv_mutex_t mutex;
};


class RWLock final {
    explicit RWLock(std::shared_ptr<Loop> ref) noexcept
        : pLoop{std::move(ref)}
    {
        uv_rwlock_init(&rwlock);
    }

public:
    static std::shared_ptr<RWLock> create(std::shared_ptr<Loop> loop) {
        return std::shared_ptr<RWLock>{new RWLock{std::move(loop)}};
    }

    ~RWLock() noexcept {
        uv_rwlock_destroy(&rwlock);
    }

    void rdLock() noexcept {
        uv_rwlock_rdlock(&rwlock);
    }

    bool tryRdLock() noexcept {
        return (0 == uv_rwlock_tryrdlock(&rwlock));
    }

    void rdUnlock() noexcept {
        uv_rwlock_rdunlock(&rwlock);
    }

    void wrLock() noexcept {
        uv_rwlock_wrlock(&rwlock);
    }

    bool tryWrLock() noexcept {
        return (0 == uv_rwlock_trywrlock(&rwlock));
    }

    void wrUnlock() noexcept {
        uv_rwlock_wrunlock(&rwlock);
    }

    Loop& loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<Loop> pLoop;
    uv_rwlock_t rwlock;
};


class Semaphore final {
    explicit Semaphore(std::shared_ptr<Loop> ref, unsigned int value) noexcept
        : pLoop{std::move(ref)}
    {
        uv_sem_init(&sem, value);
    }

public:
    static std::shared_ptr<Semaphore> create(std::shared_ptr<Loop> loop, unsigned int value) {
        return std::shared_ptr<Semaphore>{new Semaphore{std::move(loop), value}};
    }

    ~Semaphore() noexcept {
        uv_sem_destroy(&sem);
    }

    void post() noexcept {
        uv_sem_post(&sem);
    }

    void wait() noexcept {
        uv_sem_wait(&sem);
    }

    bool tryWait() noexcept {
        return (0 == uv_sem_trywait(&sem));
    }

    Loop& loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<Loop> pLoop;
    uv_sem_t sem;
};


class Condition final {
    explicit Condition(std::shared_ptr<Loop> ref) noexcept
        : pLoop{std::move(ref)}
    {
        uv_cond_init(&cond);
    }

public:
    static std::shared_ptr<Condition> create(std::shared_ptr<Loop> loop) {
        return std::shared_ptr<Condition>{new Condition{std::move(loop)}};
    }

    ~Condition() noexcept {
        uv_cond_destroy(&cond);
    }

    void signal() noexcept {
        uv_cond_signal(&cond);
    }

    void broadcast() noexcept {
        uv_cond_broadcast(&cond);
    }

    void wait(Mutex &mutex) noexcept {
        uv_cond_wait(&cond, &mutex.mutex);
    }

    bool timedWait(Mutex &mutex, uint64_t timeout) noexcept {
        return (0 == uv_cond_timedwait(&cond, &mutex.mutex, timeout));
    }

    Loop& loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<Loop> pLoop;
    uv_cond_t cond;
};


class Barrier final {
    explicit Barrier(std::shared_ptr<Loop> ref, unsigned int count) noexcept
        : pLoop{std::move(ref)}
    {
        uv_barrier_init(&barrier, count);
    }

public:
    static std::shared_ptr<Barrier> create(std::shared_ptr<Loop> loop, unsigned int count) {
        return std::shared_ptr<Barrier>{new Barrier{std::move(loop), count}};
    }

    ~Barrier() noexcept {
        uv_barrier_destroy(&barrier);
    }

    bool wait() noexcept {
        return (0 == uv_barrier_wait(&barrier));
    }

    Loop& loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<Loop> pLoop;
    uv_barrier_t barrier;
};


}
