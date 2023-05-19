#ifdef UVW_AS_LIB
#    include "loop.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE loop::loop(std::unique_ptr<uv_loop_t, deleter> ptr) noexcept
    : uv_loop{std::move(ptr)} {}

UVW_INLINE std::shared_ptr<loop> loop::create() {
    auto ptr = std::unique_ptr<uv_loop_t, deleter>{new uv_loop_t, [](uv_loop_t *l) { delete l; }};
    auto curr = std::shared_ptr<loop>{new loop{std::move(ptr)}};

    if(uv_loop_init(curr->uv_loop.get())) {
        curr = nullptr;
    }

    return curr;
}

UVW_INLINE std::shared_ptr<loop> loop::create(uv_loop_t *res) {
    auto ptr = std::unique_ptr<uv_loop_t, deleter>{res, [](uv_loop_t *) {}};
    return std::shared_ptr<loop>{new loop{std::move(ptr)}};
}

UVW_INLINE std::shared_ptr<loop> loop::get_default() {
    static std::weak_ptr<loop> ref;
    std::shared_ptr<loop> curr;

    if(ref.expired()) {
        auto def = uv_default_loop();

        if(def) {
            auto ptr = std::unique_ptr<uv_loop_t, deleter>(def, [](uv_loop_t *) {});
            curr = std::shared_ptr<loop>{new loop{std::move(ptr)}};
        }

        ref = curr;
    } else {
        curr = ref.lock();
    }

    return curr;
}

UVW_INLINE loop::~loop() noexcept {
    if(uv_loop) {
        close();
    }
}

UVW_INLINE int loop::close() {
    int ret = 0;

    if(uv_loop) {
        ret = uv_loop_close(uv_loop.get());
        uv_loop.reset();
    }

    return ret;
}

UVW_INLINE int loop::run(run_mode mode) noexcept {
    return uv_run(uv_loop.get(), static_cast<uv_run_mode>(mode));
}

UVW_INLINE bool loop::alive() const noexcept {
    return !!uv_loop_alive(uv_loop.get());
}

UVW_INLINE void loop::stop() noexcept {
    uv_stop(uv_loop.get());
}

UVW_INLINE int loop::descriptor() const noexcept {
    return uv_backend_fd(uv_loop.get());
}

UVW_INLINE std::pair<bool, loop::time> loop::timeout() const noexcept {
    auto to = uv_backend_timeout(uv_loop.get());
    return std::make_pair(to == -1, time{to});
}

UVW_INLINE loop::time loop::idle_time() const noexcept {
    return time{uv_metrics_idle_time(uv_loop.get())};
}

UVW_INLINE metrics_type loop::metrics() const noexcept {
    metrics_type res{};
    uv_metrics_info(uv_loop.get(), &res);
    return res;
}

UVW_INLINE loop::time loop::now() const noexcept {
    return time{uv_now(uv_loop.get())};
}

UVW_INLINE void loop::update() const noexcept {
    return uv_update_time(uv_loop.get());
}

UVW_INLINE int loop::fork() noexcept {
    return uv_loop_fork(uv_loop.get());
}

UVW_INLINE void loop::data(std::shared_ptr<void> ud) {
    user_data = std::move(ud);
}

UVW_INLINE const uv_loop_t *loop::raw() const noexcept {
    return uv_loop.get();
}

UVW_INLINE uv_loop_t *loop::raw() noexcept {
    return const_cast<uv_loop_t *>(const_cast<const loop *>(this)->raw());
}

} // namespace uvw
