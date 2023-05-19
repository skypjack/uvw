#ifndef UVW_LOOP_INCLUDE_H
#define UVW_LOOP_INCLUDE_H

#ifdef _WIN32
#    include <ciso646>
#endif

#include <chrono>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <uv.h>
#include "config.h"
#include "emitter.h"
#include "util.h"

namespace uvw {

class async_handle;
class check_handle;
class fs_event_handle;
class fs_poll_handle;
class idle_handle;
class pipe_handle;
class poll_handle;
class prepare_handle;
class process_handle;
class signal_handle;
class tcp_handle;
class timer_handle;
class tty_handle;
class udp_handle;

namespace details {

enum class uvw_loop_option : std::underlying_type_t<uv_loop_option> {
    BLOCK_SIGNAL = UV_LOOP_BLOCK_SIGNAL,
    IDLE_TIME = UV_METRICS_IDLE_TIME
};

enum class uvw_run_mode : std::underlying_type_t<uv_run_mode> {
    DEFAULT = UV_RUN_DEFAULT,
    ONCE = UV_RUN_ONCE,
    NOWAIT = UV_RUN_NOWAIT
};

} // namespace details

using metrics_type = uv_metrics_t; /*!< Library equivalent for uv_metrics_t. */

/**
 * @brief The loop class.
 *
 * The event loop is the central part of `uvw`'s functionalities, as well as
 * `libuv`'s ones.<br/>
 * It takes care of polling for I/O and scheduling callbacks to be run based on
 * different sources of events.
 */
class loop final: public emitter<loop>, public std::enable_shared_from_this<loop> {
    using deleter = void (*)(uv_loop_t *);

    template<typename, typename, typename...>
    friend class resource;

    class uv_token {
        friend class loop;
        explicit uv_token(int) {}
    };

    loop(std::unique_ptr<uv_loop_t, deleter> ptr) noexcept;

public:
    using token = uv_token;
    using time = std::chrono::duration<uint64_t, std::milli>;
    using option = details::uvw_loop_option;
    using run_mode = details::uvw_run_mode;

    /**
     * @brief Initializes a new loop instance.
     * @return A pointer to the newly created loop.
     */
    static std::shared_ptr<loop> create();

    /**
     * @brief Initializes a new loop instance from an existing resource.
     *
     * The lifetime of the resource must exceed that of the instance to which
     * it's associated. Management of the memory associated with the resource is
     * in charge of the user.
     *
     * @param res A valid pointer to a correctly initialized resource.
     * @return A pointer to the newly created loop.
     */
    static std::shared_ptr<loop> create(uv_loop_t *res);

    /**
     * @brief Gets the initialized default loop.
     *
     * It may return an empty pointer in case of failure.<br>
     * This function is just a convenient way for having a global loop
     * throughout an application, the default loop is in no way different than
     * the ones initialized with `create()`.<br>
     * As such, the default loop can be closed with `close()` so the resources
     * associated with it are freed (even if it is not strictly necessary).
     *
     * @return The initialized default loop.
     */
    static std::shared_ptr<loop> get_default();

    loop(const loop &) = delete;
    loop(loop &&other) = delete;

    loop &operator=(const loop &) = delete;
    loop &operator=(loop &&other) = delete;

    ~loop() noexcept;

    /**
     * @brief Sets additional loop options.
     *
     * You should normally call this before the first call to uv_run() unless
     * mentioned otherwise.<br/>
     * Supported options:
     *
     * * `loop::option::BLOCK_SIGNAL`: Block a signal when polling for new
     * events. A second argument is required and it is the signal number.
     * * `loop::option::IDLE_TIME`: Accumulate the amount of idle time the event
     * loop spends in the event provider. This option is necessary to use
     * `idle_time()`.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/loop.html#c.uv_loop_configure)
     * for further details.
     *
     * @return Underlying return value.
     */
    template<typename... Args>
    int configure(option flag, Args &&...args) {
        return uv_loop_configure(uv_loop.get(), static_cast<uv_loop_option>(flag), std::forward<Args>(args)...);
    }

    /**
     * @brief Creates resources of any type.
     *
     * This should be used as a default method to create resources.<br/>
     * The arguments are the ones required for the specific resource.
     *
     * Use it as `loop->resource<uvw::timer_handle>()`.
     *
     * @return A pointer to the newly created resource.
     */
    template<typename R, typename... Args>
    std::shared_ptr<R> resource(Args &&...args) {
        auto ptr = uninitialized_resource<R>(std::forward<Args>(args)...);
        ptr = (ptr->init() == 0) ? ptr : nullptr;
        return ptr;
    }

    /**
     * @brief Creates uninitialized resources of any type.
     * @return A pointer to the newly created resource.
     */
    template<typename R, typename... Args>
    std::shared_ptr<R> uninitialized_resource(Args &&...args) {
        return std::make_shared<R>(token{0}, shared_from_this(), std::forward<Args>(args)...);
    }

    /**
     * @brief Releases all internal loop resources.
     *
     * Call this function only when the loop has finished executing and all open
     * handles and requests have been closed, or the loop will error.
     *
     * @return Underlying return value.
     */
    int close();

    /**
     * @brief Runs the event loop.
     *
     * Available modes are:
     *
     * * `loop::run_mode::DEFAULT`: Runs the event loop until there are no more
     * active and referenced handles or requests.
     * * `loop::run_mode::ONCE`: Poll for i/o once. Note that this function
     * blocks if there are no pending callbacks.
     * * `loop::run_mode::NOWAIT`: Poll for i/o once but don’t block if there
     * are no pending callbacks.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/loop.html#c.uv_run)
     * for further details.
     *
     * @return Underlying return value.
     */
    int run(run_mode mode = run_mode::DEFAULT) noexcept;

    /**
     * @brief Checks if there are active resources.
     * @return True if there are active resources in the loop.
     */
    bool alive() const noexcept;

    /**
     * @brief Stops the event loop.
     *
     * It causes `run()` to end as soon as possible.<br/>
     * This will happen not sooner than the next loop iteration.<br/>
     * If this function was called before blocking for I/O, the loop won’t block
     * for I/O on this iteration.
     */
    void stop() noexcept;

    /**
     * @brief Get backend file descriptor.
     *
     * Only kqueue, epoll and event ports are supported.<br/>
     * This can be used in conjunction with `run(loop::run_mode::NOWAIT)` to
     * poll in one thread and run the event loop’s callbacks in another.
     *
     * @return The backend file descriptor.
     */
    int descriptor() const noexcept;

    /**
     * @brief Gets the poll timeout.
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of valid timeout, false otherwise.
     * * Milliseconds (`std::chrono::duration<uint64_t, std::milli>`).
     */
    std::pair<bool, time> timeout() const noexcept;

    /**
     * @brief Returns the amount of time the event loop has been idle. The call
     * is thread safe.
     * @return The accumulated time spent idle.
     */
    time idle_time() const noexcept;

    /**
     * @brief Tracks various internal operations of the event loop.
     * @return Event loop metrics.
     */
    metrics_type metrics() const noexcept;

    /**
     * @brief Returns the current timestamp in milliseconds.
     *
     * The timestamp is cached at the start of the event loop tick.<br/>
     * The timestamp increases monotonically from some arbitrary point in
     * time.<br/>
     * Don’t make assumptions about the starting point, you will only get
     * disappointed.
     *
     * @return The current timestamp in milliseconds (actual type is
     * `std::chrono::duration<uint64_t, std::milli>`).
     */
    time now() const noexcept;

    /**
     * @brief Updates the event loop’s concept of _now_.
     *
     * The current time is cached at the start of the event loop tick in order
     * to reduce the number of time-related system calls.<br/>
     * You won’t normally need to call this function unless you have callbacks
     * that block the event loop for longer periods of time, where _longer_ is
     * somewhat subjective but probably on the order of a millisecond or more.
     */
    void update() const noexcept;

    /**
     * @brief Walks the list of handles.
     *
     * The callback is invoked once for each handle that is still active.
     *
     * @param callback A function to invoke once for each active handle.
     */
    template<typename Func>
    void walk(Func callback) {
        auto func = [](uv_handle_t *hndl, void *func) {
            if(hndl->data) {
                auto &cb = *static_cast<Func *>(func);

                switch(utilities::guess_handle(handle_category{hndl->type})) {
                case handle_type::ASYNC:
                    cb(*static_cast<async_handle *>(hndl->data));
                    break;
                case handle_type::CHECK:
                    cb(*static_cast<check_handle *>(hndl->data));
                    break;
                case handle_type::FS_EVENT:
                    cb(*static_cast<fs_event_handle *>(hndl->data));
                    break;
                case handle_type::FS_POLL:
                    cb(*static_cast<fs_poll_handle *>(hndl->data));
                    break;
                case handle_type::IDLE:
                    cb(*static_cast<idle_handle *>(hndl->data));
                    break;
                case handle_type::PIPE:
                    cb(*static_cast<pipe_handle *>(hndl->data));
                    break;
                case handle_type::POLL:
                    cb(*static_cast<poll_handle *>(hndl->data));
                    break;
                case handle_type::PREPARE:
                    cb(*static_cast<prepare_handle *>(hndl->data));
                    break;
                case handle_type::PROCESS:
                    cb(*static_cast<process_handle *>(hndl->data));
                    break;
                case handle_type::SIGNAL:
                    cb(*static_cast<signal_handle *>(hndl->data));
                    break;
                case handle_type::TCP:
                    cb(*static_cast<tcp_handle *>(hndl->data));
                    break;
                case handle_type::TIMER:
                    cb(*static_cast<timer_handle *>(hndl->data));
                    break;
                case handle_type::TTY:
                    cb(*static_cast<tty_handle *>(hndl->data));
                    break;
                case handle_type::UDP:
                    cb(*static_cast<udp_handle *>(hndl->data));
                    break;
                default:
                    // this handle isn't managed by uvw, let it be...
                    break;
                }
            }
        };

        uv_walk(uv_loop.get(), func, &callback);
    }

    /**
     * @brief Reinitialize any kernel state necessary in the child process after
     * a fork(2) system call.
     *
     * Previously started watchers will continue to be started in the child
     * process.
     *
     * It is necessary to explicitly call this function on every event loop
     * created in the parent process that you plan to continue to use in the
     * child, including the default loop (even if you don’t continue to use it
     * in the parent). This function must be called before calling any API
     * function using the loop in the child. Failure to do so will result in
     * undefined behaviour, possibly including duplicate events delivered to
     * both parent and child or aborting the child process.
     *
     * When possible, it is preferred to create a new loop in the child process
     * instead of reusing a loop created in the parent. New loops created in the
     * child process after the fork should not use this function.
     *
     * Note that this function is not implemented on Windows.<br/>
     * Note also that this function is experimental in `libuv`. It may contain
     * bugs, and is subject to change or removal. API and ABI stability is not
     * guaranteed.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/loop.html#c.uv_loop_fork)
     * for further details.
     *
     * @return Underlying return value.
     */
    int fork() noexcept;

    /**
     * @brief Gets user-defined data. `uvw` won't use this field in any case.
     * @return User-defined data if any, an invalid pointer otherwise.
     */
    template<typename R = void>
    std::shared_ptr<R> data() const {
        return std::static_pointer_cast<R>(user_data);
    }

    /**
     * @brief Sets arbitrary data. `uvw` won't use this field in any case.
     * @param ud User-defined arbitrary data.
     */
    void data(std::shared_ptr<void> ud);

    /**
     * @brief Gets the underlying raw data structure.
     *
     * This function should not be used, unless you know exactly what you are
     * doing and what are the risks.<br/>
     * Going raw is dangerous, mainly because the lifetime management of a loop,
     * a handle or a request is in charge to the library itself and users should
     * not work around it.
     *
     * @warning
     * Use this function at your own risk, but do not expect any support in case
     * of bugs.
     *
     * @return The underlying raw data structure.
     */
    const uv_loop_t *raw() const noexcept;

    /**
     * @brief Gets the underlying raw data structure.
     *
     * This function should not be used, unless you know exactly what you are
     * doing and what are the risks.<br/>
     * Going raw is dangerous, mainly because the lifetime management of a loop,
     * a handle or a request is in charge to the library itself and users should
     * not work around it.
     *
     * @warning
     * Use this function at your own risk, but do not expect any support in case
     * of bugs.
     *
     * @return The underlying raw data structure.
     */
    uv_loop_t *raw() noexcept;

private:
    std::unique_ptr<uv_loop_t, deleter> uv_loop;
    std::shared_ptr<void> user_data{nullptr};
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "loop.cpp"
#endif

#endif // UVW_LOOP_INCLUDE_H
