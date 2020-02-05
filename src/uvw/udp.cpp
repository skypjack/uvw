#include "udp.h"

namespace uvw {

    void details::SendReq::send(uv_udp_t *handle, const struct sockaddr *addr) {
        invoke(&uv_udp_send, get(), handle, &buf, 1, addr, &defaultCallback<SendEvent>);
    }

    bool UDPHandle::init() {
        return (tag == FLAGS)
               ? initialize(&uv_udp_init_ex, flags)
               : initialize(&uv_udp_init);
    }

    void UDPHandle::open(OSSocketHandle socket) {
        invoke(&uv_udp_open, get(), socket);
    }

    void UDPHandle::bind(const sockaddr &addr, Flags<UDPHandle::Bind> opts) {
        invoke(&uv_udp_bind, get(), &addr, opts);
    }

    void UDPHandle::connect(const sockaddr &addr) {
        invoke(&uv_udp_connect, get(), &addr);
    }

    void UDPHandle::disconnect() {
        invoke(&uv_udp_connect, get(), nullptr);
    }

    bool UDPHandle::multicastLoop(bool enable) {
        return (0 == uv_udp_set_multicast_loop(get(), enable));
    }

    bool UDPHandle::multicastTtl(int val) {
        return (0 == uv_udp_set_multicast_ttl(get(), val > 255 ? 255 : val));
    }

    bool UDPHandle::broadcast(bool enable) {
        return (0 == uv_udp_set_broadcast(get(), enable));
    }

    bool UDPHandle::ttl(int val) {
        return (0 == uv_udp_set_ttl(get(), val > 255 ? 255 : val));
    }

    void UDPHandle::send(const sockaddr &addr, std::unique_ptr<char[]> data, unsigned int len) {
        auto req = loop().resource<details::SendReq>(
                std::unique_ptr<char[], details::SendReq::Deleter>{
                        data.release(), [](char *ptr) { delete[] ptr; }
                }, len);

        auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
            ptr->publish(event);
        };

        req->once<ErrorEvent>(listener);
        req->once<SendEvent>(listener);
        req->send(get(), &addr);
    }

    void UDPHandle::send(const sockaddr &addr, char *data, unsigned int len) {
        auto req = loop().resource<details::SendReq>(
                std::unique_ptr<char[], details::SendReq::Deleter>{
                        data, [](char *) {}
                }, len);

        auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
            ptr->publish(event);
        };

        req->once<ErrorEvent>(listener);
        req->once<SendEvent>(listener);
        req->send(get(), &addr);
    }

    void UDPHandle::stop() {
        invoke(&uv_udp_recv_stop, get());
    }

    size_t UDPHandle::sendQueueSize() const noexcept {
        return uv_udp_get_send_queue_size(get());
    }

    size_t UDPHandle::sendQueueCount() const noexcept {
        return uv_udp_get_send_queue_count(get());
    }
}
