#pragma once


#include <utility>
#include <memory>
#include <string>
#include <uv.h>
#include "request.hpp"
#include "util.hpp"
#include "loop.hpp"


namespace uvw {


/**
 * @brief AddrInfoEvent event.
 *
 * It will be emitted by GetAddrInfoReq according with its functionalities.
 */
struct AddrInfoEvent {
    using Deleter = void(*)(addrinfo *);

    AddrInfoEvent(std::unique_ptr<addrinfo, Deleter> addr)
        : data{std::move(addr)}
    {}

    /**
     * @brief An initialized instance of `addrinfo`.
     *
     * See [getaddrinfo](http://linux.die.net/man/3/getaddrinfo) for further
     * details.
     */
    std::unique_ptr<addrinfo, Deleter> data;
};


/**
 * @brief NameInfoEvent event.
 *
 * It will be emitted by GetNameInfoReq according with its functionalities.
 */
struct NameInfoEvent {
    NameInfoEvent(const char *host, const char *serv)
        : hostname{host}, service{serv}
    {}

    /**
     * @brief A valid hostname.
     *
     * See [getnameinfo](http://linux.die.net/man/3/getnameinfo) for further
     * details.
     */
    const char * hostname;

    /**
     * @brief A valid service name.
     *
     * See [getnameinfo](http://linux.die.net/man/3/getnameinfo) for further
     * details.
     */
    const char * service;
};


/**
 * @brief The GetAddrInfoReq request.
 *
 * Wrapper for [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).<br/>
 * It offers either asynchronous and synchronous access methods.
 *
 * To create a `GetAddrInfoReq` through a `Loop`, no arguments are required.
 */
class GetAddrInfoReq final: public Request<GetAddrInfoReq, uv_getaddrinfo_t> {
    static void addrInfoCallback(uv_getaddrinfo_t *req, int status, addrinfo *res) {
        auto ptr = reserve(req);

        if(status) {
            ptr->publish(ErrorEvent{status});
        } else {
            auto data = std::unique_ptr<addrinfo, void(*)(addrinfo *)>{
                res, [](addrinfo *addr){ uv_freeaddrinfo(addr); }};

            ptr->publish(AddrInfoEvent{std::move(data)});
        }
    }

    void nodeAddrInfo(const char *node, const char *service, addrinfo *hints = nullptr) {
        invoke(&uv_getaddrinfo, parent(), get(), &addrInfoCallback, node, service, hints);
    }

    auto nodeAddrInfoSync(const char *node, const char *service, addrinfo *hints = nullptr) {
        auto req = get();
        auto err = uv_getaddrinfo(parent(), req, nullptr, node, service, hints);
        auto data = std::unique_ptr<addrinfo, void(*)(addrinfo *)>{req->addrinfo, [](addrinfo *addr){ uv_freeaddrinfo(addr); }};
        return std::make_pair(!err, std::move(data));
    }

public:
    using Deleter = void(*)(addrinfo *);

    using Request::Request;

    /**
     * @brief Async [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     * @param node Either a numerical network address or a network hostname.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     */
    void nodeAddrInfo(std::string node, addrinfo *hints = nullptr) {
        nodeAddrInfo(node.data(), nullptr, hints);
    }

    /**
     * @brief Sync [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     *
     * @param node Either a numerical network address or a network hostname.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * A `std::unique_ptr<addrinfo, Deleter>` containing the data requested.
     */
    std::pair<bool, std::unique_ptr<addrinfo, Deleter>>
    nodeAddrInfoSync(std::string node, addrinfo *hints = nullptr) {
        return nodeAddrInfoSync(node.data(), nullptr, hints);
    }

    /**
     * @brief Async [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     * @param service Either a service name or a port number as a string.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     */
    void serviceAddrInfo(std::string service, addrinfo *hints = nullptr) {
        nodeAddrInfo(nullptr, service.data(), hints);
    }

    /**
     * @brief Sync [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     *
     * @param service Either a service name or a port number as a string.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * A `std::unique_ptr<addrinfo, Deleter>` containing the data requested.
     */
    std::pair<bool, std::unique_ptr<addrinfo, Deleter>>
    serviceAddrInfoSync(std::string service, addrinfo *hints = nullptr) {
        return nodeAddrInfoSync(nullptr, service.data(), hints);
    }

    /**
     * @brief Async [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     * @param node Either a numerical network address or a network hostname.
     * @param service Either a service name or a port number as a string.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     */
    void addrInfo(std::string node, std::string service, addrinfo *hints = nullptr) {
        nodeAddrInfo(node.data(), service.data(), hints);
    }

    /**
     * @brief Sync [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     *
     * @param node Either a numerical network address or a network hostname.
     * @param service Either a service name or a port number as a string.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * A `std::unique_ptr<addrinfo, Deleter>` containing the data requested.
     */
    std::pair<bool, std::unique_ptr<addrinfo, Deleter>>
    addrInfoSync(std::string node, std::string service, addrinfo *hints = nullptr) {
        return nodeAddrInfoSync(node.data(), service.data(), hints);
    }
};


/**
 * @brief The GetNameInfoReq request.
 *
 * Wrapper for [getnameinfo](http://linux.die.net/man/3/getnameinfo).<br/>
 * It offers either asynchronous and synchronous access methods.
 *
 * To create a `GetNameInfoReq` through a `Loop`, no arguments are required.
 */
class GetNameInfoReq final: public Request<GetNameInfoReq, uv_getnameinfo_t> {
    static void nameInfoCallback(uv_getnameinfo_t *req, int status, const char *hostname, const char *service) {
        auto ptr = reserve(req);
        if(status) { ptr->publish(ErrorEvent{status}); }
        else { ptr->publish(NameInfoEvent{hostname, service}); }
    }

public:
    using Request::Request;

    /**
     * @brief Async [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     */
    void nameInfo(const sockaddr &addr, int flags = 0) {
        invoke(&uv_getnameinfo, parent(), get(), &nameInfoCallback, &addr, flags);
    }

    /**
     * @brief Async [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     * @param ip A valid IP address.
     * @param port A valid port number.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     */
    template<typename I = IPv4>
    void nameInfo(std::string ip, unsigned int port, int flags = 0) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::addrFunc(ip.data(), port, &addr);
        nameInfo(reinterpret_cast<const sockaddr &>(addr), flags);
    }

    /**
     * @brief Async [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     * @param addr A valid instance of Addr.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     */
    template<typename I = IPv4>
    void nameInfo(Addr addr, int flags = 0) {
        nameInfo<I>(std::move(addr.ip), addr.port, flags);
    }

    /**
     * @brief Sync [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     *
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * A `std::pair` composed as it follows:
     *   * A `const char *` containing a valid hostname.
     *   * A `const char *` containing a valid service name.
     */
    std::pair<bool, std::pair<const char *, const char *>>
    nameInfoSync(const sockaddr &addr, int flags = 0) {
        auto req = get();
        auto err = uv_getnameinfo(parent(), req, nullptr, &addr, flags);
        return std::make_pair(!err, std::make_pair(req->host, req->service));
    }

    /**
     * @brief Sync [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     *
     * @param ip A valid IP address.
     * @param port A valid port number.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * A `std::pair` composed as it follows:
     *   * A `const char *` containing a valid hostname.
     *   * A `const char *` containing a valid service name.
     */
    template<typename I = IPv4>
    std::pair<bool, std::pair<const char *, const char *>>
    nameInfoSync(std::string ip, unsigned int port, int flags = 0) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::addrFunc(ip.data(), port, &addr);
        return nameInfoSync(reinterpret_cast<const sockaddr &>(addr), flags);
    }

    /**
     * @brief Sync [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     *
     * @param addr A valid instance of Addr.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * A `std::pair` composed as it follows:
     *   * A `const char *` containing a valid hostname.
     *   * A `const char *` containing a valid service name.
     */
    template<typename I = IPv4>
    std::pair<bool, std::pair<const char *, const char *>>
    nameInfoSync(Addr addr, int flags = 0) {
        return nameInfoSync<I>(std::move(addr.ip), addr.port, flags);
    }
};


}
