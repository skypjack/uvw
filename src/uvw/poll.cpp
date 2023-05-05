#ifdef UVW_AS_LIB
#    include "poll.h"
#endif

#include <utility>
#include "config.h"

namespace uvw {

UVW_INLINE poll_event::poll_event(details::uvw_poll_event events) noexcept
    : flags{std::move(events)} {}

UVW_INLINE poll_handle::poll_handle(loop::token token, std::shared_ptr<loop> ref, int desc)
    : handle{token, std::move(ref)}, tag{FD}, file_desc{desc} {}

UVW_INLINE poll_handle::poll_handle(loop::token token, std::shared_ptr<loop> ref, os_socket_handle sock)
    : handle{token, std::move(ref)}, tag{SOCKET}, socket{sock} {}

UVW_INLINE void poll_handle::start_callback(uv_poll_t *hndl, int status, int events) {
    if(poll_handle &poll = *(static_cast<poll_handle *>(hndl->data)); status) {
        poll.publish(error_event{status});
    } else {
        poll.publish(poll_event{poll_event_flags(events)});
    }
}

UVW_INLINE int poll_handle::init() {
    if(tag == SOCKET) {
        return leak_if(uv_poll_init_socket(parent().raw(), raw(), socket));
    } else {
        return leak_if(uv_poll_init(parent().raw(), raw(), file_desc));
    }
}

UVW_INLINE int poll_handle::start(poll_event_flags flags) {
    return uv_poll_start(raw(), static_cast<uv_poll_event>(flags), &start_callback);
}

UVW_INLINE int poll_handle::stop() {
    return uv_poll_stop(raw());
}

} // namespace uvw
