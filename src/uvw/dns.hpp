#pragma once


#include <utility>
#include <memory>
#include <string>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "util.hpp"
#include "loop.hpp"


namespace uvw {


/**
 * @brief AddrInfoEvent event.
 *
 * It will be emitted by GetAddrInfoReq according with its functionalities.
 */
struct AddrInfoEvent: Event<AddrInfoEvent> {
    AddrInfoEvent(std::unique_ptr<addrinfo, void(*)(addrinfo *)> ptr)
        : dt{std::move(ptr)}
    { }

    /**
     * @brief Gets an instance of `addrinfo`.
     *
     * See [getaddrinfo](http://linux.die.net/man/3/getaddrinfo) for further
     * details.
     *
     * @return An initialized instance of `addrinfo`.
     */
    addrinfo& data() const noexcept { return *dt; }

private:
    std::unique_ptr<addrinfo, void(*)(addrinfo *)> dt;
};


/**
 * @brief NameInfoEvent event.
 *
 * It will be emitted by GetNameInfoReq according with its functionalities.
 */
struct NameInfoEvent: Event<NameInfoEvent> {
    NameInfoEvent(const char *h, const char *s)
        : host{h}, serv{s}
    { }

    /**
     * @brief Gets the returned hostname.
     *
     * See [getnameinfo](http://linux.die.net/man/3/getnameinfo) for further
     * details.
     *
     * @return A valid hostname.
     */
    const char* hostname() const noexcept { return host; }

    /**
     * @brief Gets the returned service name.
     *
     * See [getnameinfo](http://linux.die.net/man/3/getnameinfo) for further
     * details.
     *
     * @return A valid service name.
     */
    const char* service() const noexcept { return serv; }

private:
    const char *host;
    const char *serv;
};


/**
 * @brief The GetAddrInfoReq request.
 *
 * Wrapper for [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).<br/>
 * It offers either asynchronous and synchronous access methods.
 */
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
        invoke(&uv_getaddrinfo, parent(), get(), &getAddrInfoCallback, node, service, hints);
    }

    auto getNodeAddrInfoSync(const char *node, const char *service, addrinfo *hints = nullptr) {
        auto req = get();
        auto err = uv_getaddrinfo(parent(), req, nullptr, node, service, hints);
        auto ptr = std::unique_ptr<addrinfo, void(*)(addrinfo *)>{req->addrinfo, [](addrinfo *res){ uv_freeaddrinfo(res); }};
        return std::make_pair(ErrorEvent{err}, AddrInfoEvent{std::move(ptr)});
    }

public:
    /**
     * @brief Creates a new `getaddrinfo` wrapper request.
     * @param loop A pointer to the loop from which the handle generated.
     * @return A pointer to the newly created handle.
     */
    static std::shared_ptr<GetAddrInfoReq> create(std::shared_ptr<Loop> loop) {
        return std::shared_ptr<GetAddrInfoReq>{new GetAddrInfoReq{std::move(loop)}};
    }

    /**
     * @brief Async [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     * @param node Either a numerical network address or a network hostname.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     */
    void getNodeAddrInfo(std::string node, addrinfo *hints = nullptr) {
        getNodeAddrInfo(node.data(), nullptr, hints);
    }

    /**
     * @brief Sync [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     * @param node Either a numerical network address or a network hostname.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     */
    auto getNodeAddrInfoSync(std::string node, addrinfo *hints = nullptr) {
        return getNodeAddrInfoSync(node.data(), nullptr, hints);
    }

    /**
     * @brief Async [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     * @param service Either a service name or a port number as a string.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     */
    void getServiceAddrInfo(std::string service, addrinfo *hints = nullptr) {
        getNodeAddrInfo(nullptr, service.data(), hints);
    }

    /**
     * @brief Sync [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     * @param service Either a service name or a port number as a string.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     */
    auto getServiceAddrInfoSync(std::string service, addrinfo *hints = nullptr) {
        return getNodeAddrInfo(nullptr, service.data(), hints);
    }

    /**
     * @brief Async [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     * @param node Either a numerical network address or a network hostname.
     * @param service Either a service name or a port number as a string.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     */
    void getAddrInfo(std::string node, std::string service, addrinfo *hints = nullptr) {
        getNodeAddrInfo(node.data(), service.data(), hints);
    }

    /**
     * @brief Sync [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     * @param node Either a numerical network address or a network hostname.
     * @param service Either a service name or a port number as a string.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     */
    auto getAddrInfoSync(std::string node, std::string service, addrinfo *hints = nullptr) {
        return getNodeAddrInfo(node.data(), service.data(), hints);
    }
};


/**
 * @brief The GetNameInfoReq request.
 *
 * Wrapper for [getnameinfo](http://linux.die.net/man/3/getnameinfo).<br/>
 * It offers either asynchronous and synchronous access methods.
 */
class GetNameInfoReq final: public Request<GetNameInfoReq, uv_getnameinfo_t> {
    static void getNameInfoCallback(uv_getnameinfo_t *req, int status, const char *hostname, const char *service) {
        auto ptr = reserve(reinterpret_cast<uv_req_t*>(req));
        if(status) { ptr->publish(ErrorEvent{status}); }
        else { ptr->publish(NameInfoEvent{hostname, service}); }
    }

    using Request::Request;

public:
    /**
     * @brief Creates a new `getnameinfo` wrapper request.
     * @param loop A pointer to the loop from which the handle generated.
     * @return A pointer to the newly created handle.
     */
    static std::shared_ptr<GetNameInfoReq> create(std::shared_ptr<Loop> loop) {
        return std::shared_ptr<GetNameInfoReq>{new GetNameInfoReq{std::move(loop)}};
    }

    /**
     * @brief Async [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     * @param ip A valid IP address.
     * @param port A valid port number.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     */
    template<typename I = IPv4>
    void getNameInfo(std::string ip, unsigned int port, int flags = 0) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::AddrFunc(ip.data(), port, &addr);
        invoke(&uv_getnameinfo, parent(), get(), &getNameInfoCallback, &addr, flags);
    }

    /**
     * @brief Async [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     * @param addr A valid instance of Addr.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     */
    template<typename I = IPv4>
    void getNameInfo(Addr addr, int flags = 0) {
        getNameInfo<I>(addr.ip, addr.port, flags);
    }

    /**
     * @brief Sync [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     * @param ip A valid IP address.
     * @param port A valid port number.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     */
    template<typename I = IPv4>
    auto getNameInfoSync(std::string ip, unsigned int port, int flags = 0) {
        typename details::IpTraits<I>::Type addr;
        details::IpTraits<I>::AddrFunc(ip.data(), port, &addr);
        auto req = get();
        auto err = uv_getnameinfo(parent(), req, nullptr, &addr, flags);
        return std::make_pair(ErrorEvent{err}, NameInfoEvent{req->host, req->service});
    }

    /**
     * @brief Sync [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     * @param addr A valid instance of Addr.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     */
    template<typename I = IPv4>
    auto getNameInfoSync(Addr addr, int flags = 0) {
        getNameInfoSync<I>(addr.ip, addr.port, flags);
    }
};


}
