#ifdef UVW_AS_LIB
#    include "pipe.h"
#endif

#include <utility>
#include "config.h"

namespace uvw {

UVW_INLINE pipe_handle::pipe_handle(loop::token token, std::shared_ptr<loop> ref, bool pass)
    : stream_handle{token, std::move(ref)}, ipc{pass} {}

UVW_INLINE int pipe_handle::init() {
    return leak_if(uv_pipe_init(parent().raw(), raw(), ipc));
}

UVW_INLINE int pipe_handle::open(file_handle file) {
    return uv_pipe_open(raw(), file);
}

UVW_INLINE int pipe_handle::bind(const std::string &name) {
    return uv_pipe_bind(raw(), name.data());
}

UVW_INLINE void pipe_handle::connect(const std::string &name) {
    auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
        ptr->publish(event);
    };

    auto connect = parent().resource<details::connect_req>();
    connect->on<error_event>(listener);
    connect->on<connect_event>(listener);
    connect->connect(&uv_pipe_connect, raw(), name.data());
}

UVW_INLINE std::string pipe_handle::sock() const UVW_NOEXCEPT {
    return details::try_read(&uv_pipe_getsockname, raw());
}

UVW_INLINE std::string pipe_handle::peer() const UVW_NOEXCEPT {
    return details::try_read(&uv_pipe_getpeername, raw());
}

UVW_INLINE void pipe_handle::pending(int count) UVW_NOEXCEPT {
    uv_pipe_pending_instances(raw(), count);
}

UVW_INLINE int pipe_handle::pending() UVW_NOEXCEPT {
    return uv_pipe_pending_count(raw());
}

UVW_INLINE handle_type pipe_handle::receive() UVW_NOEXCEPT {
    handle_category category = uv_pipe_pending_type(raw());
    return utilities::guess_handle(category);
}

UVW_INLINE bool pipe_handle::chmod(chmod_flags flags) UVW_NOEXCEPT {
    return (0 == uv_pipe_chmod(raw(), static_cast<uv_poll_event>(flags)));
}

} // namespace uvw
