#include "tcp.h"
#include "defines.h"

namespace uvw {

    UVW_INLINE_SPECIFIER bool TCPHandle::init() {
        return (tag == FLAGS) ? initialize(&uv_tcp_init_ex, flags) : initialize(&uv_tcp_init);
    }

    UVW_INLINE_SPECIFIER void TCPHandle::open(OSSocketHandle socket) {
        invoke(&uv_tcp_open, get(), socket);
    }

    UVW_INLINE_SPECIFIER bool TCPHandle::noDelay(bool value) {
        return (0 == uv_tcp_nodelay(get(), value));
    }

    UVW_INLINE_SPECIFIER bool TCPHandle::keepAlive(bool enable, TCPHandle::Time time) {
        return (0 == uv_tcp_keepalive(get(), enable, time.count()));
    }

    UVW_INLINE_SPECIFIER bool TCPHandle::simultaneousAccepts(bool enable) {
        return (0 == uv_tcp_simultaneous_accepts(get(), enable));
    }

    UVW_INLINE_SPECIFIER void TCPHandle::bind(const sockaddr &addr, Flags<Bind> opts) {
        invoke(&uv_tcp_bind, get(), &addr, opts);
    }

    UVW_INLINE_SPECIFIER void TCPHandle::connect(const sockaddr &addr) {
        auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
            ptr->publish(event);
        };

        auto req = loop().resource<details::ConnectReq>();
        req->once<ErrorEvent>(listener);
        req->once<ConnectEvent>(listener);
        req->connect(&uv_tcp_connect, get(), &addr);
    }

    UVW_INLINE_SPECIFIER void TCPHandle::closeReset() {
        invoke(&uv_tcp_close_reset, get(), &this->closeCallback);
    }
}
