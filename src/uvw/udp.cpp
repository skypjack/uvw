#include "udp.h"
#include "config.h"


namespace uvw {


UVW_INLINE void details::SendReq::send(uv_udp_t *handle, const struct sockaddr *addr) {
    invoke(&uv_udp_send, get(), handle, &buf, 1, addr, &defaultCallback<SendEvent>);
}


UVW_INLINE bool UDPHandle::init() {
    return (tag == FLAGS) ? initialize(&uv_udp_init_ex, flags) : initialize(&uv_udp_init);
}


UVW_INLINE void UDPHandle::open(OSSocketHandle socket) {
    invoke(&uv_udp_open, get(), socket);
}


UVW_INLINE void UDPHandle::bind(const sockaddr &addr, Flags<UDPHandle::Bind> opts) {
    invoke(&uv_udp_bind, get(), &addr, opts);
}


UVW_INLINE void UDPHandle::connect(const sockaddr &addr) {
    invoke(&uv_udp_connect, get(), &addr);
}


UVW_INLINE void UDPHandle::disconnect() {
    invoke(&uv_udp_connect, get(), nullptr);
}


UVW_INLINE bool UDPHandle::multicastLoop(bool enable) {
    return (0 == uv_udp_set_multicast_loop(get(), enable));
}


UVW_INLINE bool UDPHandle::multicastTtl(int val) {
    return (0 == uv_udp_set_multicast_ttl(get(), val > 255 ? 255 : val));
}


UVW_INLINE bool UDPHandle::broadcast(bool enable) {
    return (0 == uv_udp_set_broadcast(get(), enable));
}


UVW_INLINE bool UDPHandle::ttl(int val) {
    return (0 == uv_udp_set_ttl(get(), val > 255 ? 255 : val));
}


UVW_INLINE void UDPHandle::send(const sockaddr &addr, std::unique_ptr<char[]> data, unsigned int len) {
    auto req = loop().resource<details::SendReq>(
            std::unique_ptr<char[], details::SendReq::Deleter>{data.release(), [](char *ptr) {
                delete[] ptr;
            }}, len);

    auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
        ptr->publish(event);
    };

    req->once<ErrorEvent>(listener);
    req->once<SendEvent>(listener);
    req->send(get(), &addr);
}


UVW_INLINE void UDPHandle::send(const sockaddr &addr, char *data, unsigned int len) {
    auto req = loop().resource<details::SendReq>(
            std::unique_ptr<char[], details::SendReq::Deleter>{data, [](char *) {
            }}, len);

    auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
        ptr->publish(event);
    };

    req->once<ErrorEvent>(listener);
    req->once<SendEvent>(listener);
    req->send(get(), &addr);
}


UVW_INLINE void UDPHandle::stop() {
    invoke(&uv_udp_recv_stop, get());
}


UVW_INLINE size_t UDPHandle::sendQueueSize() const noexcept {
    return uv_udp_get_send_queue_size(get());
}


UVW_INLINE size_t UDPHandle::sendQueueCount() const noexcept {
    return uv_udp_get_send_queue_count(get());
}


}
