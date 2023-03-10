#ifdef UVW_AS_LIB
#    include "stream.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE data_event::data_event(std::unique_ptr<char[]> buf, std::size_t len) noexcept
    : data{std::move(buf)},
      length{len} {}

UVW_INLINE void details::connect_req::connect_callback(uv_connect_t *req, int status) {
    if(auto ptr = reserve(req); status) {
        ptr->publish(error_event{status});
    } else {
        ptr->publish(connect_event{});
    }
}

UVW_INLINE void details::shutdown_req::shoutdown_callback(uv_shutdown_t *req, int status) {
    if(auto ptr = reserve(req); status) {
        ptr->publish(error_event{status});
    } else {
        ptr->publish(shutdown_event{});
    }
}

UVW_INLINE int details::shutdown_req::shutdown(uv_stream_t *hndl) {
    return this->leak_if(uv_shutdown(raw(), hndl, &shoutdown_callback));
}

} // namespace uvw
