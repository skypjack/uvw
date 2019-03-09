#pragma once


#include <memory>
#include <string>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <uv.h>
#include "loop.hpp"
#include "underlying_type.hpp"


namespace uvw {


namespace details {


enum class UVThreadCreateFlags: std::underlying_type_t<uv_thread_create_flags> {
    THREAD_NO_FLAGS = UV_THREAD_NO_FLAGS,
    THREAD_HAS_STACK_SIZE = UV_THREAD_HAS_STACK_SIZE
};


}


class Thread;
class ThreadLocalStorage;
class Once;
class Mutex;
class RWLock;
class Semaphore;
class Condition;
class Barrier;


/**
 * @brief The Thread wrapper.
 *
 * To create a `Thread` through a `Loop`, arguments follow:
 *
 * * A callback invoked to initialize thread execution. The type must be such
 * that it can be assigned to an `std::function<void(std::shared_ptr<void>)>`.
 * * An optional payload the type of which is `std::shared_ptr<void>`.
 */
class Thread final: public UnderlyingType<Thread, uv_thread_t> {
    using InternalTask = std::function<void(std::shared_ptr<void>)>;

    static void createCallback(void *arg) {
        Thread &thread = *(static_cast<Thread*>(arg));
        thread.task(thread.data);
    }

public:
    using Options = details::UVThreadCreateFlags;
    using Task = InternalTask;
    using Type = uv_thread_t;

    explicit Thread(ConstructorAccess ca, std::shared_ptr<Loop> ref, Task t, std::shared_ptr<void> d = nullptr) noexcept
        : UnderlyingType{ca, std::move(ref)}, data{std::move(d)}, task{std::move(t)}
    {}

    /**
     * @brief Obtains the identifier of the calling thread.
     * @return The identifier of the calling thread.
     */
    static Type self() noexcept {
        return uv_thread_self();
    }

    /**
     * @brief Compares thread by means of their identifiers.
     * @param tl A valid instance of a thread.
     * @param tr A valid instance of a thread.
     * @return True if the two threads are the same thread, false otherwise.
     */
    static bool equal(const Thread &tl, const Thread &tr) noexcept {
        return !(0 == uv_thread_equal(tl.get(), tr.get()));
    }

    ~Thread() noexcept {
        join();
    }

    /**
     * @brief Creates a new thread.
     * @return True in case of success, false otherwise.
     */
    bool run() noexcept {
        return (0 == uv_thread_create(get(), &createCallback, this));
    }

    /**
     * @brief Creates a new thread.
     *
     * Available flags are:
     *
     * * `Thread::Options::THREAD_NO_FLAGS`: no flags set.
     * * `Thread::Options::THREAD_HAS_STACK_SIZE`: if set, `stack` specifies a
     * stack size for the new thread. 0 indicates that the default value should
     * be used (it behaves as if the flag was not set). Other values will be
     * rounded up to the nearest page boundary.
     *
     * @return True in case of success, false otherwise.
     */
    bool run(Flags<Options> opts, std::size_t stack = {}) noexcept {
        uv_thread_options_t params{opts, stack};
        return (0 == uv_thread_create_ex(get(), &params, &createCallback, this));
    }

    /**
     * @brief Joins with a terminated thread.
     * @return True in case of success, false otherwise.
     */
    bool join() noexcept {
        return (0 == uv_thread_join(get()));
    }

private:
    std::shared_ptr<void> data;
    Task task;
};


/**
 * @brief The ThreadLocalStorage wrapper.
 *
 * A storage area that can only be accessed by one thread. The variable can be
 * seen as a global variable that is only visible to a particular thread and not
 * the whole program.
 */
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

    /**
     * @brief Gets the value of a given variable.
     * @tparam T Type to which to cast the opaque storage area.
     * @return A pointer to the given variable.
     */
    template<typename T>
    T* get() noexcept {
        return static_cast<T*>(uv_key_get(UnderlyingType::get()));
    }

    /**
     * @brief Sets the value of a given variable.
     * @tparam T Type of the variable to store aside.
     * @param value A valid pointer to the variable to store
     */
    template<typename T>
    void set(T *value) noexcept {
        return uv_key_set(UnderlyingType::get(), value);
    }
};


/**
 * @brief The Once wrapper.
 *
 * Runs a function once and only once. Concurrent calls to `once` will block all
 * callers except one (it’s unspecified which one).
 */
class Once final: public UnderlyingType<Once, uv_once_t> {
    static uv_once_t* guard() noexcept {
        static uv_once_t once = UV_ONCE_INIT;
        return &once;
    }

public:
    using UnderlyingType::UnderlyingType;

    /**
     * @brief Runs a function once and only once.
     *
     * The callback must be such that it's convertible to `void(*)(void)`. Free
     * functions and non-capturing lambdas are both viable solutions.
     *
     * @tparam F Type of the callback.
     * @param f A valid callback function.
     */
    template<typename F>
    static void once(F &&f) noexcept {
        using CallbackType = void(*)(void);
        static_assert(std::is_convertible<F, CallbackType>::value, "!");
        CallbackType cb = f;
        uv_once(guard(), cb);
    }
};


/**
 * @brief The Mutex wrapper.
 *
 * To create a `Mutex` through a `Loop`, arguments follow:
 *
 * * An option boolean that specifies if the mutex is a recursive one. The
 * default value is false, the mutex isn't recursive.
 */
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

    /**
     * @brief Locks the mutex.
     */
    void lock() noexcept {
        uv_mutex_lock(get());
    }

    /**
     * @brief Tries to lock the mutex.
     * @return True in case of success, false otherwise.
     */
    bool tryLock() noexcept {
        return (0 == uv_mutex_trylock(get()));
    }

    /**
     * @brief Unlocks the mutex.
     */
    void unlock() noexcept {
        uv_mutex_unlock(get());
    }
};


/**
 * @brief The RWLock wrapper.
 */
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

    /**
     * @brief Locks a read-write lock object for reading.
     */
    void rdLock() noexcept {
        uv_rwlock_rdlock(get());
    }

    /**
     * @brief Tries to lock a read-write lock object for reading.
     * @return True in case of success, false otherwise.
     */
    bool tryRdLock() noexcept {
        return (0 == uv_rwlock_tryrdlock(get()));
    }

    /**
     * @brief Unlocks a read-write lock object previously locked for reading.
     */
    void rdUnlock() noexcept {
        uv_rwlock_rdunlock(get());
    }

    /**
     * @brief Locks a read-write lock object for writing.
     */
    void wrLock() noexcept {
        uv_rwlock_wrlock(get());
    }

    /**
     * @brief Tries to lock a read-write lock object for writing.
     * @return True in case of success, false otherwise.
     */
    bool tryWrLock() noexcept {
        return (0 == uv_rwlock_trywrlock(get()));
    }

    /**
     * @brief Unlocks a read-write lock object previously locked for writing.
     */
    void wrUnlock() noexcept {
        uv_rwlock_wrunlock(get());
    }
};


/**
 * @brief The Semaphore wrapper.
 *
 * To create a `Semaphore` through a `Loop`, arguments follow:
 *
 * * An unsigned integer that specifies the initial value for the semaphore.
 */
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

    /**
     * @brief Unlocks a semaphore.
     */
    void post() noexcept {
        uv_sem_post(get());
    }

    /**
     * @brief Locks a semaphore.
     */
    void wait() noexcept {
        uv_sem_wait(get());
    }

    /**
     * @brief Tries to lock a semaphore.
     * @return True in case of success, false otherwise.
     */
    bool tryWait() noexcept {
        return (0 == uv_sem_trywait(get()));
    }
};


/**
 * @brief The Condition wrapper.
 */
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

    /**
     * @brief Signals a condition.
     *
     * This function shall unblock at least one of the threads that are blocked
     * on the specified condition variable (if any threads are blocked on it).
     */
    void signal() noexcept {
        uv_cond_signal(get());
    }

    /**
     * @brief Broadcasts a condition.
     *
     * This function shall unblock threads blocked on a condition variable.
     */
    void broadcast() noexcept {
        uv_cond_broadcast(get());
    }

    /**
     * @brief Waits on a condition.
     *
     * These function atomically releases the mutex and causes the calling
     * thread to block on the condition variable.
     *
     * @param mutex A mutex locked by the calling thread, otherwise expect
     * undefined behavior.
     */
    void wait(Mutex &mutex) noexcept {
        uv_cond_wait(get(), mutex.get());
    }

    /**
     * @brief Waits on a condition.
     *
     * These function atomically releases the mutex and causes the calling
     * thread to block on the condition variable.<br/>
     * The functions returns with an error if the absolute time specified passes
     * (that is, system time equals or exceeds it) before the condition is
     * signaled or broadcasted, or if the absolute time specified has already
     * been passed at the time of the call.
     *
     * @param mutex A mutex locked by the calling thread, otherwise expect
     * undefined behavior.
     * @param timeout The maximum time to wait before to return.
     * @return True in case of success, false otherwise.
     */
    bool timedWait(Mutex &mutex, uint64_t timeout) noexcept {
        return (0 == uv_cond_timedwait(get(), mutex.get(), timeout));
    }
};


/**
 * @brief The Barrier wrapper.
 *
 * To create a `Barrier` through a `Loop`, arguments follow:
 *
 * * An unsigned integer that specifies the number of threads that must call
 * `wait` before any of them successfully return from the call. The value
 * specified must be greater than zero.
 */
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

    /**
     * @brief Synchronizes at a barrier.
     * @return True in case of success, false otherwise.
     */
    bool wait() noexcept {
        return (0 == uv_barrier_wait(get()));
    }
};


}
