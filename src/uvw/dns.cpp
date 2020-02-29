#include "dns.h"
#include "config.h"


namespace uvw {


UVW_INLINE void GetAddrInfoReq::addrInfoCallback(uv_getaddrinfo_t *req, int status, addrinfo *res) {
    auto ptr = reserve(req);

    if(status) {
        ptr->publish(ErrorEvent{status});
    } else {
        auto data = std::unique_ptr<addrinfo, void (*)(addrinfo *)>{res, [](addrinfo *addr) {
            uv_freeaddrinfo(addr);
        }};

        ptr->publish(AddrInfoEvent{std::move(data)});
    }
}


UVW_INLINE void GetAddrInfoReq::nodeAddrInfo(const char *node, const char *service, addrinfo *hints) {
    invoke(&uv_getaddrinfo, parent(), get(), &addrInfoCallback, node, service, hints);
}


UVW_INLINE auto GetAddrInfoReq::nodeAddrInfoSync(const char *node, const char *service, addrinfo *hints) {
    auto req = get();
    auto err = uv_getaddrinfo(parent(), req, nullptr, node, service, hints);
    auto data = std::unique_ptr<addrinfo, void (*)(addrinfo *)>{req->addrinfo, [](addrinfo *addr) {
        uv_freeaddrinfo(addr);
    }};

    return std::make_pair(!err, std::move(data));
}


UVW_INLINE void GetAddrInfoReq::nodeAddrInfo(std::string node, addrinfo *hints) {
    nodeAddrInfo(node.data(), nullptr, hints);
}


UVW_INLINE std::pair<bool, std::unique_ptr<addrinfo, GetAddrInfoReq::Deleter>> GetAddrInfoReq::nodeAddrInfoSync(std::string node, addrinfo *hints) {
    return nodeAddrInfoSync(node.data(), nullptr, hints);
}


UVW_INLINE void GetAddrInfoReq::serviceAddrInfo(std::string service, addrinfo *hints) {
    nodeAddrInfo(nullptr, service.data(), hints);
}


UVW_INLINE std::pair<bool, std::unique_ptr<addrinfo, GetAddrInfoReq::Deleter>> GetAddrInfoReq::serviceAddrInfoSync(std::string service, addrinfo *hints) {
    return nodeAddrInfoSync(nullptr, service.data(), hints);
}


UVW_INLINE void GetAddrInfoReq::addrInfo(std::string node, std::string service, addrinfo *hints) {
    nodeAddrInfo(node.data(), service.data(), hints);
}


UVW_INLINE std::pair<bool, std::unique_ptr<addrinfo, GetAddrInfoReq::Deleter>> GetAddrInfoReq::addrInfoSync(std::string node, std::string service, addrinfo *hints) {
    return nodeAddrInfoSync(node.data(), service.data(), hints);
}


UVW_INLINE void GetNameInfoReq::nameInfoCallback(uv_getnameinfo_t *req, int status, const char *hostname, const char *service) {
    auto ptr = reserve(req);

    if(status) {
        ptr->publish(ErrorEvent{status});
    } else {
        ptr->publish(NameInfoEvent{hostname, service});
    }
}


UVW_INLINE void GetNameInfoReq::nameInfo(const sockaddr &addr, int flags) {
    invoke(&uv_getnameinfo, parent(), get(), &nameInfoCallback, &addr, flags);
}


UVW_INLINE std::pair<bool, std::pair<const char *, const char *>> GetNameInfoReq::nameInfoSync(const sockaddr &addr, int flags) {
    auto req = get();
    auto err = uv_getnameinfo(parent(), req, nullptr, &addr, flags);
    return std::make_pair(!err, std::make_pair(req->host, req->service));
}


}
