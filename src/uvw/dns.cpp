#ifdef UVW_AS_LIB
#    include "dns.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE addr_info_event::addr_info_event(std::unique_ptr<addrinfo, deleter> addr)
    : data{std::move(addr)} {}

UVW_INLINE name_info_event::name_info_event(const char *host, const char *serv)
    : hostname{host}, service{serv} {}

UVW_INLINE void get_addr_info_req::addr_info_callback(uv_getaddrinfo_t *req, int status, addrinfo *res) {
    if(auto ptr = reserve(req); status) {
        ptr->publish(error_event{status});
    } else {
        auto data = std::unique_ptr<addrinfo, void (*)(addrinfo *)>{res, [](addrinfo *addr) { uv_freeaddrinfo(addr); }};
        ptr->publish(addr_info_event{std::move(data)});
    }
}

UVW_INLINE int get_addr_info_req::node_addr_info(const char *node, const char *service, addrinfo *hints) {
    return this->leak_if(uv_getaddrinfo(parent().raw(), raw(), &addr_info_callback, node, service, hints));
}

UVW_INLINE auto get_addr_info_req::node_addr_info_sync(const char *node, const char *service, addrinfo *hints) {
    auto req = raw();
    auto err = uv_getaddrinfo(parent().raw(), req, nullptr, node, service, hints);
    auto data = std::unique_ptr<addrinfo, void (*)(addrinfo *)>{req->addrinfo, [](addrinfo *addr) { uv_freeaddrinfo(addr); }};
    return std::make_pair(!err, std::move(data));
}

UVW_INLINE int get_addr_info_req::node_addr_info(const std::string &node, addrinfo *hints) {
    return node_addr_info(node.data(), nullptr, hints);
}

UVW_INLINE std::pair<bool, std::unique_ptr<addrinfo, get_addr_info_req::deleter>> get_addr_info_req::node_addr_info_sync(const std::string &node, addrinfo *hints) {
    return node_addr_info_sync(node.data(), nullptr, hints);
}

UVW_INLINE int get_addr_info_req::service_addr_info(const std::string &service, addrinfo *hints) {
    return node_addr_info(nullptr, service.data(), hints);
}

UVW_INLINE std::pair<bool, std::unique_ptr<addrinfo, get_addr_info_req::deleter>> get_addr_info_req::service_addr_info_sync(const std::string &service, addrinfo *hints) {
    return node_addr_info_sync(nullptr, service.data(), hints);
}

UVW_INLINE int get_addr_info_req::addr_info(const std::string &node, const std::string &service, addrinfo *hints) {
    return node_addr_info(node.data(), service.data(), hints);
}

UVW_INLINE std::pair<bool, std::unique_ptr<addrinfo, get_addr_info_req::deleter>> get_addr_info_req::addr_info_sync(const std::string &node, const std::string &service, addrinfo *hints) {
    return node_addr_info_sync(node.data(), service.data(), hints);
}

UVW_INLINE void get_name_info_req::name_info_callback(uv_getnameinfo_t *req, int status, const char *hostname, const char *service) {
    if(auto ptr = reserve(req); status) {
        ptr->publish(error_event{status});
    } else {
        ptr->publish(name_info_event{hostname, service});
    }
}

UVW_INLINE int get_name_info_req::name_info(const sockaddr &addr, int flags) {
    return this->leak_if(uv_getnameinfo(parent().raw(), raw(), &name_info_callback, &addr, flags));
}

UVW_INLINE int get_name_info_req::name_info(const std::string &ip, unsigned int port, int flags) {
    return name_info(details::ip_addr(ip.data(), port), flags);
}

UVW_INLINE int get_name_info_req::name_info(socket_address addr, int flags) {
    return name_info(std::move(addr.ip), addr.port, flags);
}

UVW_INLINE std::pair<bool, std::pair<const char *, const char *>> get_name_info_req::name_info_sync(const sockaddr &addr, int flags) {
    auto req = raw();
    auto err = uv_getnameinfo(parent().raw(), req, nullptr, &addr, flags);
    return std::make_pair(!err, std::make_pair(req->host, req->service));
}

UVW_INLINE std::pair<bool, std::pair<const char *, const char *>> get_name_info_req::name_info_sync(const std::string &ip, unsigned int port, int flags) {
    return name_info_sync(details::ip_addr(ip.data(), port), flags);
}

UVW_INLINE std::pair<bool, std::pair<const char *, const char *>> get_name_info_req::name_info_sync(socket_address addr, int flags) {
    return name_info_sync(addr.ip, addr.port, flags);
}

} // namespace uvw
