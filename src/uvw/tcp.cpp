#ifdef UVW_AS_LIB
#    include "tcp.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE tcp_handle::tcp_handle(loop::token token, std::shared_ptr<loop> ref, unsigned int f)
    : stream_handle{token, std::move(ref)}, tag{f ? FLAGS : DEFAULT}, flags{f} {}

UVW_INLINE int tcp_handle::init() {
    if(tag == FLAGS) {
        return leak_if(uv_tcp_init_ex(parent().raw(), raw(), flags));
    } else {
        return leak_if(uv_tcp_init(parent().raw(), raw()));
    }
}

UVW_INLINE int tcp_handle::open(os_socket_handle socket) {
    return uv_tcp_open(raw(), socket);
}

UVW_INLINE bool tcp_handle::no_delay(bool value) {
    return (0 == uv_tcp_nodelay(raw(), value));
}

UVW_INLINE bool tcp_handle::keep_alive(bool enable, tcp_handle::time val) {
    return (0 == uv_tcp_keepalive(raw(), enable, val.count()));
}

UVW_INLINE bool tcp_handle::simultaneous_accepts(bool enable) {
    return (0 == uv_tcp_simultaneous_accepts(raw(), enable));
}

UVW_INLINE int tcp_handle::bind(const sockaddr &addr, tcp_flags opts) {
    return uv_tcp_bind(raw(), &addr, static_cast<uv_tcp_flags>(opts));
}

UVW_INLINE int tcp_handle::bind(const std::string &ip, unsigned int port, tcp_flags opts) {
    return bind(details::ip_addr(ip.data(), port), opts);
}

UVW_INLINE int tcp_handle::bind(socket_address addr, tcp_flags opts) {
    return bind(addr.ip, addr.port, opts);
}

UVW_INLINE socket_address tcp_handle::sock() const noexcept {
    sockaddr_storage storage;
    int len = sizeof(sockaddr_storage);
    uv_tcp_getsockname(raw(), reinterpret_cast<sockaddr *>(&storage), &len);
    return details::sock_addr(storage);
}

UVW_INLINE socket_address tcp_handle::peer() const noexcept {
    sockaddr_storage storage;
    int len = sizeof(sockaddr_storage);
    uv_tcp_getpeername(raw(), reinterpret_cast<sockaddr *>(&storage), &len);
    return details::sock_addr(storage);
}

UVW_INLINE int tcp_handle::connect(const std::string &ip, unsigned int port) {
    return connect(details::ip_addr(ip.data(), port));
}

UVW_INLINE int tcp_handle::connect(socket_address addr) {
    return connect(addr.ip, addr.port);
}

UVW_INLINE int tcp_handle::connect(const sockaddr &addr) {
    auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
        ptr->publish(event);
    };

    auto req = parent().resource<details::connect_req>();
    req->on<error_event>(listener);
    req->on<connect_event>(listener);

    return req->connect(&uv_tcp_connect, raw(), &addr);
}

UVW_INLINE int tcp_handle::close_reset() {
    return uv_tcp_close_reset(raw(), &this->close_callback);
}

} // namespace uvw
