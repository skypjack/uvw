#ifdef UVW_AS_LIB
#    include "thread.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE thread::thread(loop::token token, std::shared_ptr<loop> ref, task t, std::shared_ptr<void> d) noexcept
    : uv_type{token, std::move(ref)},
      data{std::move(d)},
      func{std::move(t)} {}

UVW_INLINE void thread::create_callback(void *arg) {
    thread &curr = *(static_cast<thread *>(arg));
    curr.func(curr.data);
}

UVW_INLINE thread::type thread::self() noexcept {
    return uv_thread_self();
}

UVW_INLINE int thread::getcpu() noexcept {
    return uv_thread_getcpu();
}

UVW_INLINE bool thread::equal(const thread &tl, const thread &tr) noexcept {
    return !(0 == uv_thread_equal(tl.raw(), tr.raw()));
}

UVW_INLINE thread::~thread() noexcept {
    join();
}

UVW_INLINE bool thread::run() noexcept {
    return (0 == uv_thread_create(raw(), &create_callback, this));
}

UVW_INLINE bool thread::run(create_flags opts, std::size_t stack) noexcept {
    uv_thread_options_t params{static_cast<unsigned int>(opts), stack};
    return (0 == uv_thread_create_ex(raw(), &params, &create_callback, this));
}

UVW_INLINE bool thread::join() noexcept {
    return (0 == uv_thread_join(raw()));
}

UVW_INLINE thread_local_storage::thread_local_storage(loop::token token, std::shared_ptr<loop> ref) noexcept
    : uv_type{token, std::move(ref)} {
    uv_key_create(uv_type::raw());
}

UVW_INLINE thread_local_storage::~thread_local_storage() noexcept {
    uv_key_delete(uv_type::raw());
}

UVW_INLINE uv_once_t *once::guard() noexcept {
    static uv_once_t once = UV_ONCE_INIT;
    return &once;
}

UVW_INLINE mutex::mutex(loop::token token, std::shared_ptr<loop> ref, bool recursive) noexcept
    : uv_type{token, std::move(ref)} {
    if(recursive) {
        uv_mutex_init_recursive(raw());
    } else {
        uv_mutex_init(raw());
    }
}

UVW_INLINE mutex::~mutex() noexcept {
    uv_mutex_destroy(raw());
}

UVW_INLINE void mutex::lock() noexcept {
    uv_mutex_lock(raw());
}

UVW_INLINE bool mutex::try_lock() noexcept {
    return (0 == uv_mutex_trylock(raw()));
}

UVW_INLINE void mutex::unlock() noexcept {
    uv_mutex_unlock(raw());
}

UVW_INLINE rwlock::rwlock(loop::token token, std::shared_ptr<loop> ref) noexcept
    : uv_type{token, std::move(ref)} {
    uv_rwlock_init(raw());
}

UVW_INLINE rwlock::~rwlock() noexcept {
    uv_rwlock_destroy(raw());
}

UVW_INLINE void rwlock::rdlock() noexcept {
    uv_rwlock_rdlock(raw());
}

UVW_INLINE bool rwlock::try_rdlock() noexcept {
    return (0 == uv_rwlock_tryrdlock(raw()));
}

UVW_INLINE void rwlock::rdunlock() noexcept {
    uv_rwlock_rdunlock(raw());
}

UVW_INLINE void rwlock::wrlock() noexcept {
    uv_rwlock_wrlock(raw());
}

UVW_INLINE bool rwlock::try_wrlock() noexcept {
    return (0 == uv_rwlock_trywrlock(raw()));
}

UVW_INLINE void rwlock::wrunlock() noexcept {
    uv_rwlock_wrunlock(raw());
}

UVW_INLINE semaphore::semaphore(loop::token token, std::shared_ptr<loop> ref, unsigned int value) noexcept
    : uv_type{token, std::move(ref)} {
    uv_sem_init(raw(), value);
}

UVW_INLINE semaphore::~semaphore() noexcept {
    uv_sem_destroy(raw());
}

UVW_INLINE void semaphore::post() noexcept {
    uv_sem_post(raw());
}

UVW_INLINE void semaphore::wait() noexcept {
    uv_sem_wait(raw());
}

UVW_INLINE bool semaphore::try_wait() noexcept {
    return (0 == uv_sem_trywait(raw()));
}

UVW_INLINE condition::condition(loop::token token, std::shared_ptr<loop> ref) noexcept
    : uv_type{token, std::move(ref)} {
    uv_cond_init(raw());
}

UVW_INLINE condition::~condition() noexcept {
    uv_cond_destroy(raw());
}

UVW_INLINE void condition::signal() noexcept {
    uv_cond_signal(raw());
}

UVW_INLINE void condition::broadcast() noexcept {
    uv_cond_broadcast(raw());
}

UVW_INLINE void condition::wait(mutex &mtx) noexcept {
    uv_cond_wait(raw(), mtx.raw());
}

UVW_INLINE bool condition::timed_wait(mutex &mtx, uint64_t timeout) noexcept {
    return (0 == uv_cond_timedwait(raw(), mtx.raw(), timeout));
}

UVW_INLINE barrier::barrier(loop::token token, std::shared_ptr<loop> ref, unsigned int count) noexcept
    : uv_type{token, std::move(ref)} {
    uv_barrier_init(raw(), count);
}

UVW_INLINE barrier::~barrier() noexcept {
    uv_barrier_destroy(raw());
}

UVW_INLINE bool barrier::wait() noexcept {
    return (0 == uv_barrier_wait(raw()));
}

} // namespace uvw
