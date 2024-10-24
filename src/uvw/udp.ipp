#include <array>
#include "config.h"

namespace uvw {

UVW_INLINE udp_data_event::udp_data_event(socket_address sndr, std::unique_ptr<char[]> buf, std::size_t len, bool part) noexcept
    : data{std::move(buf)},
      length{len},
      sender{std::move(sndr)},
      partial{part} {}

UVW_INLINE void details::send_req::udp_send_callback(uv_udp_send_t *req, int status) {
    if(auto ptr = reserve(req); status) {
        ptr->publish(error_event{status});
    } else {
        ptr->publish(send_event{});
    }
}

UVW_INLINE details::send_req::send_req(loop::token token, std::shared_ptr<loop> parent, std::unique_ptr<char[], deleter> dt, unsigned int len)
    : request{token, std::move(parent)},
      data{std::move(dt)},
      buf{uv_buf_init(data.get(), len)} {}

UVW_INLINE int details::send_req::send(uv_udp_t *hndl, const struct sockaddr *addr) {
    return this->leak_if(uv_udp_send(raw(), hndl, &buf, 1, addr, &udp_send_callback));
}

UVW_INLINE void udp_handle::recv_callback(uv_udp_t *hndl, ssize_t nread, const uv_buf_t *buf, const sockaddr *addr, unsigned flags) {
    udp_handle &udp = *(static_cast<udp_handle *>(hndl->data));
    // data will be destroyed no matter of what the value of nread is
    std::unique_ptr<char[]> data{buf->base};

    if(nread > 0) {
        // data available (can be truncated)
        udp.publish(udp_data_event{details::sock_addr(*addr), std::move(data), static_cast<std::size_t>(nread), !(0 == (flags & UV_UDP_PARTIAL))});
    } else if(nread == 0 && addr == nullptr) {
        // no more data to be read, doing nothing is fine
    } else if(nread == 0 && addr != nullptr) {
        // empty udp packet
        udp.publish(udp_data_event{details::sock_addr(*addr), std::move(data), static_cast<std::size_t>(nread), false});
    } else {
        // transmission error
        udp.publish(error_event(nread));
    }
}

UVW_INLINE udp_handle::udp_handle(loop::token token, std::shared_ptr<loop> ref, unsigned int f)
    : handle{token, std::move(ref)}, tag{FLAGS}, flags{f} {}

UVW_INLINE int udp_handle::init() {
    if(tag == FLAGS) {
        return leak_if(uv_udp_init_ex(parent().raw(), raw(), flags));
    } else {
        return leak_if(uv_udp_init(parent().raw(), raw()));
    }
}

UVW_INLINE int udp_handle::open(os_socket_handle socket) {
    return uv_udp_open(raw(), socket);
}

UVW_INLINE int udp_handle::connect(const sockaddr &addr) {
    return uv_udp_connect(raw(), &addr);
}

UVW_INLINE int udp_handle::connect(const std::string &ip, unsigned int port) {
    return connect(details::ip_addr(ip.data(), port));
}

UVW_INLINE int udp_handle::connect(const socket_address &addr) {
    return connect(addr.ip, addr.port);
}

UVW_INLINE int udp_handle::disconnect() {
    return uv_udp_connect(raw(), nullptr);
}

UVW_INLINE socket_address udp_handle::peer() const noexcept {
    sockaddr_storage storage;
    int len = sizeof(sockaddr_storage);
    uv_udp_getpeername(raw(), reinterpret_cast<sockaddr *>(&storage), &len);
    return details::sock_addr(storage);
}

UVW_INLINE int udp_handle::bind(const sockaddr &addr, udp_handle::udp_flags opts) {
    return uv_udp_bind(raw(), &addr, static_cast<uv_udp_flags>(opts));
}

UVW_INLINE int udp_handle::bind(const std::string &ip, unsigned int port, udp_flags opts) {
    return bind(details::ip_addr(ip.data(), port), opts);
}

UVW_INLINE int udp_handle::bind(const socket_address &addr, udp_flags opts) {
    return bind(addr.ip, addr.port, opts);
}

UVW_INLINE socket_address udp_handle::sock() const noexcept {
    sockaddr_storage storage;
    int len = sizeof(sockaddr_storage);
    uv_udp_getsockname(raw(), reinterpret_cast<sockaddr *>(&storage), &len);
    return details::sock_addr(storage);
}

UVW_INLINE bool udp_handle::multicast_membership(const std::string &multicast, const std::string &iface, membership ms) {
    return (0 == uv_udp_set_membership(raw(), multicast.data(), iface.data(), static_cast<uv_membership>(ms)));
}

UVW_INLINE bool udp_handle::multicast_loop(bool enable) {
    return (0 == uv_udp_set_multicast_loop(raw(), enable));
}

UVW_INLINE bool udp_handle::multicast_ttl(int val) {
    return (0 == uv_udp_set_multicast_ttl(raw(), val > 255 ? 255 : val));
}

UVW_INLINE bool udp_handle::multicast_interface(const std::string &iface) {
    return (0 == uv_udp_set_multicast_interface(raw(), iface.data()));
}

UVW_INLINE bool udp_handle::broadcast(bool enable) {
    return (0 == uv_udp_set_broadcast(raw(), enable));
}

UVW_INLINE bool udp_handle::ttl(int val) {
    return (0 == uv_udp_set_ttl(raw(), val > 255 ? 255 : val));
}

UVW_INLINE int udp_handle::send(const sockaddr &addr, std::unique_ptr<char[]> data, unsigned int len) {
    auto req = parent().resource<details::send_req>(std::unique_ptr<char[], details::send_req::deleter>{data.release(), [](char *ptr) { delete[] ptr; }}, len);

    auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
        ptr->publish(event);
    };

    req->on<error_event>(listener);
    req->on<send_event>(listener);

    return req->send(raw(), &addr);
}

UVW_INLINE int udp_handle::send(const std::string &ip, unsigned int port, std::unique_ptr<char[]> data, unsigned int len) {
    return send(details::ip_addr(ip.data(), port), std::move(data), len);
}

UVW_INLINE int udp_handle::send(const socket_address &addr, std::unique_ptr<char[]> data, unsigned int len) {
    return send(addr.ip, addr.port, std::move(data), len);
}

UVW_INLINE int udp_handle::send(const sockaddr &addr, char *data, unsigned int len) {
    auto req = parent().resource<details::send_req>(std::unique_ptr<char[], details::send_req::deleter>{data, [](char *) {}}, len);

    auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
        ptr->publish(event);
    };

    req->on<error_event>(listener);
    req->on<send_event>(listener);

    return req->send(raw(), &addr);
}

UVW_INLINE int udp_handle::send(const std::string &ip, unsigned int port, char *data, unsigned int len) {
    return send(details::ip_addr(ip.data(), port), data, len);
}

UVW_INLINE int udp_handle::send(const socket_address &addr, char *data, unsigned int len) {
    return send(addr.ip, addr.port, data, len);
}

UVW_INLINE int udp_handle::try_send(const sockaddr &addr, std::unique_ptr<char[]> data, unsigned int len) {
    std::array bufs{uv_buf_init(data.get(), len)};
    return uv_udp_try_send(raw(), bufs.data(), 1, &addr);
}

UVW_INLINE int udp_handle::try_send(const std::string &ip, unsigned int port, std::unique_ptr<char[]> data, unsigned int len) {
    return try_send(details::ip_addr(ip.data(), port), std::move(data), len);
}

UVW_INLINE int udp_handle::try_send(const socket_address &addr, std::unique_ptr<char[]> data, unsigned int len) {
    return try_send(addr.ip, addr.port, std::move(data), len);
}

UVW_INLINE int udp_handle::try_send(const sockaddr &addr, char *data, unsigned int len) {
    std::array bufs{uv_buf_init(data, len)};
    return uv_udp_try_send(raw(), bufs.data(), 1, &addr);
}

UVW_INLINE int udp_handle::try_send(const std::string &ip, unsigned int port, char *data, unsigned int len) {
    return try_send(details::ip_addr(ip.data(), port), data, len);
}

UVW_INLINE int udp_handle::try_send(const socket_address &addr, char *data, unsigned int len) {
    return try_send(addr.ip, addr.port, data, len);
}

UVW_INLINE int udp_handle::recv() {
    return uv_udp_recv_start(raw(), &details::common_alloc_callback, &recv_callback);
}

UVW_INLINE int udp_handle::stop() {
    return uv_udp_recv_stop(raw());
}

UVW_INLINE size_t udp_handle::send_queue_size() const noexcept {
    return uv_udp_get_send_queue_size(raw());
}

UVW_INLINE size_t udp_handle::send_queue_count() const noexcept {
    return uv_udp_get_send_queue_count(raw());
}

} // namespace uvw
