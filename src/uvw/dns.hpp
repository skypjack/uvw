#pragma once


#include <utility>
#include <memory>
#include <string>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "util.hpp"


namespace uvw {


struct AddrInfoEvent: Event<AddrInfoEvent> {
    AddrInfoEvent(std::unique_ptr<addrinfo, void(*)(addrinfo *)> ptr)
        : dt{std::move(ptr)}
    { }

    addrinfo& data() const noexcept { return *dt; }

private:
    std::unique_ptr<addrinfo, void(*)(addrinfo *)> dt;
};


struct NameInfoEvent: Event<NameInfoEvent> {
    NameInfoEvent(const char *h, const char *s)
        : host{h}, serv{s}
    { }

    const char* hostname() const noexcept { return host; }
    const char* service() const noexcept { return serv; }

private:
    const char *host;
    const char *serv;
};


class GetAddrInfoReq final: public Request<GetAddrInfoReq, uv_getaddrinfo_t> {
    static void getAddrInfoCallback(uv_getaddrinfo_t *req, int status, addrinfo *res) {
        auto ptr = reserve(reinterpret_cast<uv_req_t*>(req));

        if(status) {
            ptr->publish(ErrorEvent{status});
        } else {
            auto data = std::unique_ptr<addrinfo, void(*)(addrinfo *)>{
                res, [](addrinfo *res){ uv_freeaddrinfo(res); }};

            ptr->publish(AddrInfoEvent{std::move(data)});
        }
    }

    using Request::Request;

    void getNodeAddrInfo(const char *node, const char *service, addrinfo *hints = nullptr) {
        invoke(&uv_getaddrinfo, parent(), get<uv_getaddrinfo_t>(), &getAddrInfoCallback, node, service, hints);
    }

    auto getNodeAddrInfoSync(const char *node, const char *service, addrinfo *hints = nullptr) {
        auto req = get<uv_getaddrinfo_t>();
        auto err = uv_getaddrinfo(parent(), req, nullptr, node, service, hints);
        auto ptr = std::unique_ptr<addrinfo, void(*)(addrinfo *)>{req->addrinfo, [](addrinfo *res){ uv_freeaddrinfo(res); }};
        return std::make_pair(ErrorEvent{err}, AddrInfoEvent{std::move(ptr)});
    }

public:
    template<typename... Args>
    static std::shared_ptr<GetAddrInfoReq> create(Args&&... args) {
        return std::shared_ptr<GetAddrInfoReq>{new GetAddrInfoReq{std::forward<Args>(args)...}};
    }

    void getNodeAddrInfo(std::string node, addrinfo *hints = nullptr) {
        getNodeAddrInfo(node.data(), nullptr, hints);
    }

    auto getNodeAddrInfoSync(std::string node, addrinfo *hints = nullptr) {
        return getNodeAddrInfoSync(node.data(), nullptr, hints);
    }

    void getServiceAddrInfo(std::string service, addrinfo *hints = nullptr) {
        getNodeAddrInfo(nullptr, service.data(), hints);
    }

    auto getServiceAddrInfoSync(std::string service, addrinfo *hints = nullptr) {
        return getNodeAddrInfo(nullptr, service.data(), hints);
    }

    void getAddrInfo(std::string node, std::string service, addrinfo *hints = nullptr) {
        getNodeAddrInfo(node.data(), service.data(), hints);
    }

    auto getAddrInfoSync(std::string node, std::string service, addrinfo *hints = nullptr) {
        return getNodeAddrInfo(node.data(), service.data(), hints);
    }
};


class GetNameInfoReq final: public Request<GetNameInfoReq, uv_getnameinfo_t> {
    static void getNameInfoCallback(uv_getnameinfo_t *req, int status, const char *hostname, const char *service) {
        auto ptr = reserve(reinterpret_cast<uv_req_t*>(req));
        if(status) { ptr->publish(ErrorEvent{status}); }
        else { ptr->publish(NameInfoEvent{hostname, service}); }
    }

    using Request::Request;

public:
    using IPv4 = details::IPv4;
    using IPv6 = details::IPv6;

    template<typename... Args>
    static std::shared_ptr<GetNameInfoReq> create(Args&&... args) {
        return std::shared_ptr<GetNameInfoReq>{new GetNameInfoReq{std::forward<Args>(args)...}};
    }

    template<typename I = IPv4>
    void getNameInfo(std::string ip, unsigned int port, int flags = 0) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::AddrFunc(ip.data(), port, &addr);
        invoke(&uv_getnameinfo, parent(), get<uv_getnameinfo_t>(), &getNameInfoCallback, &addr, flags);
    }

    template<typename I = IPv4>
    void getNameInfo(Addr addr, int flags = 0) {
        getNameInfo<I>(addr.ip, addr.port, flags);
    }

    template<typename I = IPv4>
    auto getNameInfoSync(std::string ip, unsigned int port, int flags = 0) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::AddrFunc(ip.data(), port, &addr);
        auto req = get<uv_getnameinfo_t>();
        auto err = uv_getnameinfo(parent(), req, nullptr, &addr, flags);
        return std::make_pair(ErrorEvent{err}, NameInfoEvent{req->host, req->service});
    }

    template<typename I = IPv4>
    auto getNameInfoSync(Addr addr, int flags = 0) {
        getNameInfoSync<I>(addr.ip, addr.port, flags);
    }
};


}
