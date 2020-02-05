#include "thread.h"

namespace uvw {

    void Thread::createCallback(void *arg) {
        Thread &thread = *(static_cast<Thread*>(arg));
        thread.task(thread.data);
    }

    Thread::Type Thread::self() noexcept {
        return uv_thread_self();
    }

    bool Thread::equal(const Thread &tl, const Thread &tr) noexcept {
        return !(0 == uv_thread_equal(tl.get(), tr.get()));
    }

    Thread::~Thread() noexcept {
        join();
    }

    bool Thread::run() noexcept {
        return (0 == uv_thread_create(get(), &createCallback, this));
    }

    bool Thread::run(Flags<Options> opts, std::size_t stack) noexcept {
        uv_thread_options_t params{opts, stack};
        return (0 == uv_thread_create_ex(get(), &params, &createCallback, this));
    }

    bool Thread::join() noexcept {
        return (0 == uv_thread_join(get()));
    }

    ThreadLocalStorage::ThreadLocalStorage(UnderlyingType<ThreadLocalStorage, uv_key_t>::ConstructorAccess ca,
                                           std::shared_ptr<Loop> ref) noexcept
            : UnderlyingType{ca, std::move(ref)}
    {
        uv_key_create(UnderlyingType::get());
    }

    ThreadLocalStorage::~ThreadLocalStorage() noexcept {
        uv_key_delete(UnderlyingType::get());
    }

    uv_once_t *Once::guard() noexcept {
        static uv_once_t once = UV_ONCE_INIT;
        return &once;
    }

    Mutex::Mutex(UnderlyingType<Mutex, uv_mutex_t>::ConstructorAccess ca, std::shared_ptr<Loop> ref,
                 bool recursive) noexcept
            : UnderlyingType{ca, std::move(ref)}
    {
        if(recursive) {
            uv_mutex_init_recursive(get());
        } else {
            uv_mutex_init(get());
        }
    }

    Mutex::~Mutex() noexcept {
        uv_mutex_destroy(get());
    }

    void Mutex::lock() noexcept {
        uv_mutex_lock(get());
    }

    bool Mutex::tryLock() noexcept {
        return (0 == uv_mutex_trylock(get()));
    }

    void Mutex::unlock() noexcept {
        uv_mutex_unlock(get());
    }


    RWLock::RWLock(UnderlyingType<RWLock, uv_rwlock_t>::ConstructorAccess ca, std::shared_ptr<Loop> ref) noexcept
            : UnderlyingType{ca, std::move(ref)}
    {
        uv_rwlock_init(get());
    }

    RWLock::~RWLock() noexcept {
        uv_rwlock_destroy(get());
    }

    void RWLock::rdLock() noexcept {
        uv_rwlock_rdlock(get());
    }

    bool RWLock::tryRdLock() noexcept {
        return (0 == uv_rwlock_tryrdlock(get()));
    }

    void RWLock::rdUnlock() noexcept {
        uv_rwlock_rdunlock(get());
    }

    void RWLock::wrLock() noexcept {
        uv_rwlock_wrlock(get());
    }

    bool RWLock::tryWrLock() noexcept {
        return (0 == uv_rwlock_trywrlock(get()));
    }

    void RWLock::wrUnlock() noexcept {
        uv_rwlock_wrunlock(get());
    }

    Semaphore::Semaphore(UnderlyingType<Semaphore, uv_sem_t>::ConstructorAccess ca, std::shared_ptr<Loop> ref,
                         unsigned int value) noexcept
            : UnderlyingType{ca, std::move(ref)}
    {
        uv_sem_init(get(), value);
    }

    void Semaphore::post() noexcept {
        uv_sem_post(get());
    }

    void Semaphore::wait() noexcept {
        uv_sem_wait(get());
    }

    bool Semaphore::tryWait() noexcept {
        return (0 == uv_sem_trywait(get()));
    }

    Condition::Condition(UnderlyingType<Condition, uv_cond_t>::ConstructorAccess ca, std::shared_ptr<Loop> ref) noexcept
            : UnderlyingType{ca, std::move(ref)}
    {
        uv_cond_init(get());
    }

    Condition::~Condition() noexcept {
        uv_cond_destroy(get());
    }

    void Condition::signal() noexcept {
        uv_cond_signal(get());
    }

    void Condition::broadcast() noexcept {
        uv_cond_broadcast(get());
    }

    void Condition::wait(Mutex &mutex) noexcept {
        uv_cond_wait(get(), mutex.get());
    }

    bool Condition::timedWait(Mutex &mutex, uint64_t timeout) noexcept {
        return (0 == uv_cond_timedwait(get(), mutex.get(), timeout));
    }

    Barrier::Barrier(UnderlyingType<Barrier, uv_barrier_t>::ConstructorAccess ca, std::shared_ptr<Loop> ref,
                     unsigned int count) noexcept
            : UnderlyingType{ca, std::move(ref)}
    {
        uv_barrier_init(get(), count);
    }

    Barrier::~Barrier() noexcept {
        uv_barrier_destroy(get());
    }

    bool Barrier::wait() noexcept {
        return (0 == uv_barrier_wait(get()));
    }
}
