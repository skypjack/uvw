#include "udp.h"
#include "config.h"


namespace uvw {


UVW_INLINE UDPDataEvent::UDPDataEvent(Addr sndr, std::unique_ptr<const char[]> buf, std::size_t len, bool part) noexcept
    : data{std::move(buf)}, length{len}, sender{std::move(sndr)}, partial{part}
{}


UVW_INLINE details::SendReq::SendReq(ConstructorAccess ca, std::shared_ptr<Loop> loop, std::unique_ptr<char[], Deleter> dt, unsigned int len)
    : Request<SendReq, uv_udp_send_t>{ca, std::move(loop)},
      data{std::move(dt)},
      buf{uv_buf_init(data.get(), len)}
{}


UVW_INLINE void details::SendReq::send(uv_udp_t *handle, const struct sockaddr *addr) {
    invoke(&uv_udp_send, get(), handle, &buf, 1, addr, &defaultCallback<SendEvent>);
}


UVW_INLINE UDPHandle::UDPHandle(ConstructorAccess ca, std::shared_ptr<Loop> ref, unsigned int f)
    : Handle{ca, std::move(ref)}, tag{FLAGS}, flags{f}
{}


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


// explicit instantiation definitions

template void UDPHandle::connect<IPv4>(std::string, unsigned int);
template void UDPHandle::connect<IPv6>(std::string, unsigned int);

template void UDPHandle::connect<IPv4>(Addr);
template void UDPHandle::connect<IPv6>(Addr);

template Addr UDPHandle::peer<IPv4>() const noexcept;
template Addr UDPHandle::peer<IPv6>() const noexcept;

template void UDPHandle::bind<IPv4>(std::string, unsigned int, Flags<Bind>);
template void UDPHandle::bind<IPv6>(std::string, unsigned int, Flags<Bind>);

template void UDPHandle::bind<IPv4>(Addr, Flags<Bind>);
template void UDPHandle::bind<IPv6>(Addr, Flags<Bind>);

template Addr UDPHandle::sock<IPv4>() const noexcept;
template Addr UDPHandle::sock<IPv6>() const noexcept;

template bool UDPHandle::multicastMembership<IPv4>(std::string, std::string, Membership);
template bool UDPHandle::multicastMembership<IPv6>(std::string, std::string, Membership);

template bool UDPHandle::multicastInterface<IPv4>(std::string);
template bool UDPHandle::multicastInterface<IPv6>(std::string);

template void UDPHandle::send<IPv4>(std::string, unsigned int, std::unique_ptr<char[]>, unsigned int);
template void UDPHandle::send<IPv6>(std::string, unsigned int, std::unique_ptr<char[]>, unsigned int);

template void UDPHandle::send<IPv4>(Addr, std::unique_ptr<char[]>, unsigned int);
template void UDPHandle::send<IPv6>(Addr, std::unique_ptr<char[]>, unsigned int);

template void UDPHandle::send<IPv4>(std::string, unsigned int, char *, unsigned int);
template void UDPHandle::send<IPv6>(std::string, unsigned int, char *, unsigned int);

template void UDPHandle::send<IPv4>(Addr, char *, unsigned int);
template void UDPHandle::send<IPv6>(Addr, char *, unsigned int);

template int UDPHandle::trySend<IPv4>(const sockaddr &, std::unique_ptr<char[]>, unsigned int);
template int UDPHandle::trySend<IPv6>(const sockaddr &, std::unique_ptr<char[]>, unsigned int);

template int UDPHandle::trySend<IPv4>(std::string, unsigned int, std::unique_ptr<char[]>, unsigned int);
template int UDPHandle::trySend<IPv6>(std::string, unsigned int, std::unique_ptr<char[]>, unsigned int);

template int UDPHandle::trySend<IPv4>(Addr, std::unique_ptr<char[]>, unsigned int);
template int UDPHandle::trySend<IPv6>(Addr, std::unique_ptr<char[]>, unsigned int);

template int UDPHandle::trySend<IPv4>(const sockaddr &, char *, unsigned int);
template int UDPHandle::trySend<IPv6>(const sockaddr &, char *, unsigned int);

template int UDPHandle::trySend<IPv4>(std::string, unsigned int, char *, unsigned int);
template int UDPHandle::trySend<IPv6>(std::string, unsigned int, char *, unsigned int);

template int UDPHandle::trySend<IPv4>(Addr, char *, unsigned int);
template int UDPHandle::trySend<IPv6>(Addr, char *, unsigned int);

template void UDPHandle::recv<IPv4>();
template void UDPHandle::recv<IPv6>();


}
