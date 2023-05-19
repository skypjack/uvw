#ifndef UVW_THREAD_INCLUDE_H
#define UVW_THREAD_INCLUDE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <uv.h>
#include "config.h"
#include "enum.hpp"
#include "loop.h"
#include "uv_type.hpp"

namespace uvw {

namespace details {

enum class uvw_thread_create_flags : std::underlying_type_t<uv_thread_create_flags> {
    THREAD_NO_FLAGS = UV_THREAD_NO_FLAGS,
    THREAD_HAS_STACK_SIZE = UV_THREAD_HAS_STACK_SIZE
};

}

class thread;
class thread_local_storage;
class once;
class mutex;
class rwlock;
class semaphore;
class condition;
class barrier;

/**
 * @brief The thread wrapper.
 *
 * To create a `thread` through a `loop`, arguments follow:
 *
 * * A callback invoked to initialize thread execution. The type must be such
 * that it can be assigned to an `std::function<void(std::shared_ptr<void>)>`.
 * * An optional payload the type of which is `std::shared_ptr<void>`.
 */
class thread final: public uv_type<uv_thread_t> {
    using internal_task = std::function<void(std::shared_ptr<void>)>;

    static void create_callback(void *arg);

public:
    using create_flags = details::uvw_thread_create_flags;
    using task = internal_task;
    using type = uv_thread_t;

    explicit thread(loop::token token, std::shared_ptr<loop> ref, task t, std::shared_ptr<void> d = nullptr) noexcept;

    /**
     * @brief Obtains the identifier of the calling thread.
     * @return The identifier of the calling thread.
     */
    static type self() noexcept;

    /**
     * @brief Gets the CPU number on which the calling thread is running.
     * @return The CPU number on which the calling thread is running.
     */
    static int getcpu() noexcept;

    /**
     * @brief Compares thread by means of their identifiers.
     * @param tl A valid instance of a thread.
     * @param tr A valid instance of a thread.
     * @return True if the two threads are the same thread, false otherwise.
     */
    static bool equal(const thread &tl, const thread &tr) noexcept;

    ~thread() noexcept;

    /**
     * @brief Creates a new thread.
     * @return True in case of success, false otherwise.
     */
    bool run() noexcept;

    /**
     * @brief Creates a new thread.
     *
     * Available flags are:
     *
     * * `thread::create_flags::THREAD_NO_FLAGS`: no flags set.
     * * `thread::create_flags::THREAD_HAS_STACK_SIZE`: if set, `stack` specifies a
     * stack size for the new thread. 0 indicates that the default value should
     * be used (it behaves as if the flag was not set). Other values will be
     * rounded up to the nearest page boundary.
     *
     * @return True in case of success, false otherwise.
     */
    bool run(create_flags opts, std::size_t stack = {}) noexcept;

    /**
     * @brief Joins with a terminated thread.
     * @return True in case of success, false otherwise.
     */
    bool join() noexcept;

private:
    std::shared_ptr<void> data;
    task func;
};

/**
 * @brief The thread local storage wrapper.
 *
 * A storage area that can only be accessed by one thread. The variable can be
 * seen as a global variable that is only visible to a particular thread and not
 * the whole program.
 */
class thread_local_storage final: public uv_type<uv_key_t> {
public:
    explicit thread_local_storage(loop::token token, std::shared_ptr<loop> ref) noexcept;

    ~thread_local_storage() noexcept;

    /**
     * @brief Gets the value of a given variable.
     * @tparam T Type to which to cast the opaque storage area.
     * @return A pointer to the given variable.
     */
    template<typename T>
    T *get() noexcept {
        return static_cast<T *>(uv_key_get(uv_type::raw()));
    }

    /**
     * @brief Sets the value of a given variable.
     * @tparam T Type of the variable to store aside.
     * @param value A valid pointer to the variable to store
     */
    template<typename T>
    void set(T *value) noexcept {
        return uv_key_set(uv_type::raw(), value);
    }
};

/**
 * @brief The once wrapper.
 *
 * Runs a function once and only once. Concurrent calls to `once` will block all
 * callers except one (it’s unspecified which one).
 */
class once final: public uv_type<uv_once_t> {
    static uv_once_t *guard() noexcept;

public:
    using uv_type::uv_type;

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
    static void run(F &&f) noexcept {
        using callback_type = void (*)(void);
        static_assert(std::is_convertible_v<F, callback_type>);
        callback_type cb = f;
        uv_once(guard(), cb);
    }
};

/**
 * @brief The mutex wrapper.
 *
 * To create a `mutex` through a `loop`, arguments follow:
 *
 * * An option boolean that specifies if the mutex is a recursive one. The
 * default value is false, the mutex isn't recursive.
 */
class mutex final: public uv_type<uv_mutex_t> {
    friend class condition;

public:
    explicit mutex(loop::token token, std::shared_ptr<loop> ref, bool recursive = false) noexcept;

    ~mutex() noexcept;

    /**
     * @brief Locks the mutex.
     */
    void lock() noexcept;

    /**
     * @brief Tries to lock the mutex.
     * @return True in case of success, false otherwise.
     */
    bool try_lock() noexcept;

    /**
     * @brief Unlocks the mutex.
     */
    void unlock() noexcept;
};

/**
 * @brief The rwlock wrapper.
 */
class rwlock final: public uv_type<uv_rwlock_t> {
public:
    explicit rwlock(loop::token token, std::shared_ptr<loop> ref) noexcept;

    ~rwlock() noexcept;

    /**
     * @brief Locks a read-write lock object for reading.
     */
    void rdlock() noexcept;

    /**
     * @brief Tries to lock a read-write lock object for reading.
     * @return True in case of success, false otherwise.
     */
    bool try_rdlock() noexcept;

    /**
     * @brief Unlocks a read-write lock object previously locked for reading.
     */
    void rdunlock() noexcept;

    /**
     * @brief Locks a read-write lock object for writing.
     */
    void wrlock() noexcept;

    /**
     * @brief Tries to lock a read-write lock object for writing.
     * @return True in case of success, false otherwise.
     */
    bool try_wrlock() noexcept;

    /**
     * @brief Unlocks a read-write lock object previously locked for writing.
     */
    void wrunlock() noexcept;
};

/**
 * @brief The semaphore wrapper.
 *
 * To create a `semaphore` through a `loop`, arguments follow:
 *
 * * An unsigned integer that specifies the initial value for the semaphore.
 */
class semaphore final: public uv_type<uv_sem_t> {
public:
    explicit semaphore(loop::token token, std::shared_ptr<loop> ref, unsigned int value) noexcept;

    ~semaphore() noexcept;

    /**
     * @brief Unlocks a semaphore.
     */
    void post() noexcept;

    /**
     * @brief Locks a semaphore.
     */
    void wait() noexcept;

    /**
     * @brief Tries to lock a semaphore.
     * @return True in case of success, false otherwise.
     */
    bool try_wait() noexcept;
};

/**
 * @brief The condition wrapper.
 */
class condition final: public uv_type<uv_cond_t> {
public:
    explicit condition(loop::token token, std::shared_ptr<loop> ref) noexcept;

    ~condition() noexcept;

    /**
     * @brief Signals a condition.
     *
     * This function shall unblock at least one of the threads that are blocked
     * on the specified condition variable (if any threads are blocked on it).
     */
    void signal() noexcept;

    /**
     * @brief Broadcasts a condition.
     *
     * This function shall unblock threads blocked on a condition variable.
     */
    void broadcast() noexcept;

    /**
     * @brief Waits on a condition.
     *
     * These function atomically releases the mutex and causes the calling
     * thread to block on the condition variable.
     *
     * @param mtx A mutex locked by the calling thread, otherwise expect
     * undefined behavior.
     */
    void wait(mutex &mtx) noexcept;

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
     * @param mtx A mutex locked by the calling thread, otherwise expect
     * undefined behavior.
     * @param timeout The maximum time to wait before to return.
     * @return True in case of success, false otherwise.
     */
    bool timed_wait(mutex &mtx, uint64_t timeout) noexcept;
};

/**
 * @brief The barrier wrapper.
 *
 * To create a `barrier` through a `loop`, arguments follow:
 *
 * * An unsigned integer that specifies the number of threads that must call
 * `wait` before any of them successfully return from the call. The value
 * specified must be greater than zero.
 */
class barrier final: public uv_type<uv_barrier_t> {
public:
    explicit barrier(loop::token token, std::shared_ptr<loop> ref, unsigned int count) noexcept;

    ~barrier() noexcept;

    /**
     * @brief Synchronizes at a barrier.
     * @return True in case of success, false otherwise.
     */
    bool wait() noexcept;
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "thread.cpp"
#endif

#endif // UVW_THREAD_INCLUDE_H
