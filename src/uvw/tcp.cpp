#include "tcp.h"
#include "config.h"


namespace uvw {


UVW_INLINE TCPHandle::TCPHandle(ConstructorAccess ca, std::shared_ptr<Loop> ref, unsigned int f)
    : StreamHandle{ca, std::move(ref)}, tag{f ? FLAGS : DEFAULT}, flags{f}
{}


UVW_INLINE bool TCPHandle::init() {
    return (tag == FLAGS) ? initialize(&uv_tcp_init_ex, flags) : initialize(&uv_tcp_init);
}


UVW_INLINE void TCPHandle::open(OSSocketHandle socket) {
    invoke(&uv_tcp_open, get(), socket);
}


UVW_INLINE bool TCPHandle::noDelay(bool value) {
    return (0 == uv_tcp_nodelay(get(), value));
}


UVW_INLINE bool TCPHandle::keepAlive(bool enable, TCPHandle::Time time) {
    return (0 == uv_tcp_keepalive(get(), enable, time.count()));
}


UVW_INLINE bool TCPHandle::simultaneousAccepts(bool enable) {
    return (0 == uv_tcp_simultaneous_accepts(get(), enable));
}


UVW_INLINE void TCPHandle::bind(const sockaddr &addr, Flags<Bind> opts) {
    invoke(&uv_tcp_bind, get(), &addr, opts);
}


UVW_INLINE void TCPHandle::connect(const sockaddr &addr) {
    auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
        ptr->publish(event);
    };

    auto req = loop().resource<details::ConnectReq>();
    req->once<ErrorEvent>(listener);
    req->once<ConnectEvent>(listener);
    req->connect(&uv_tcp_connect, get(), &addr);
}


UVW_INLINE void TCPHandle::closeReset() {
    invoke(&uv_tcp_close_reset, get(), &this->closeCallback);
}


}
