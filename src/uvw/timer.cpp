#ifdef UVW_AS_LIB
#    include "timer.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE void timer_handle::start_callback(uv_timer_t *hndl) {
    timer_handle &timer = *(static_cast<timer_handle *>(hndl->data));
    timer.publish(timer_event{});
}

UVW_INLINE int timer_handle::init() {
    return leak_if(uv_timer_init(parent().raw(), raw()));
}

UVW_INLINE void timer_handle::start(timer_handle::time timeout, timer_handle::time repeat) {
    if(auto err = uv_timer_start(raw(), &start_callback, timeout.count(), repeat.count()); err != 0) {
        publish(error_event{err});
    }
}

UVW_INLINE void timer_handle::stop() {
    // uv_timer_stop never returns a value other than 0 apparently
    uv_timer_stop(raw());
}

UVW_INLINE void timer_handle::again() {
    if(auto err = uv_timer_again(raw()); err != 0) {
        publish(error_event{err});
    }
}

UVW_INLINE void timer_handle::repeat(timer_handle::time repeat) {
    uv_timer_set_repeat(raw(), repeat.count());
}

UVW_INLINE timer_handle::time timer_handle::repeat() {
    return time{uv_timer_get_repeat(raw())};
}

UVW_INLINE timer_handle::time timer_handle::due_in() {
    return time{uv_timer_get_due_in(raw())};
}

} // namespace uvw
