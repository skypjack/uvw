#ifndef UVW_DNS_INCLUDE_H
#define UVW_DNS_INCLUDE_H

#include <memory>
#include <string>
#include <utility>
#include <uv.h>
#include "loop.h"
#include "request.hpp"
#include "util.h"

namespace uvw {

/*! @brief The addrinfo event. */
struct addr_info_event {
    using deleter = void (*)(addrinfo *);

    addr_info_event(std::unique_ptr<addrinfo, deleter> addr);

    /**
     * @brief An initialized instance of `addrinfo`.
     *
     * See [getaddrinfo](http://linux.die.net/man/3/getaddrinfo) for further
     * details.
     */
    std::unique_ptr<addrinfo, deleter> data;
};

/*! @brief The nameinfo event. */
struct name_info_event {
    name_info_event(const char *host, const char *serv);

    /**
     * @brief A valid hostname.
     *
     * See [getnameinfo](http://linux.die.net/man/3/getnameinfo) for further
     * details.
     */
    const char *hostname;

    /**
     * @brief A valid service name.
     *
     * See [getnameinfo](http://linux.die.net/man/3/getnameinfo) for further
     * details.
     */
    const char *service;
};

/**
 * @brief The getaddrinfo request.
 *
 * Wrapper for [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).<br/>
 * It offers either asynchronous and synchronous access methods.
 *
 * To create a `get_addr_info_req` through a `loop`, no arguments are required.
 */
class get_addr_info_req final: public request<get_addr_info_req, uv_getaddrinfo_t, addr_info_event> {
    static void addr_info_callback(uv_getaddrinfo_t *req, int status, addrinfo *res);
    int node_addr_info(const char *node, const char *service, addrinfo *hints = nullptr);
    auto node_addr_info_sync(const char *node, const char *service, addrinfo *hints = nullptr);

public:
    using deleter = void (*)(addrinfo *);

    using request::request;

    /**
     * @brief Async [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     * @param node Either a numerical network address or a network hostname.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     * @return Underlying return value.
     */
    int node_addr_info(const std::string &node, addrinfo *hints = nullptr);

    /**
     * @brief Sync [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     *
     * @param node Either a numerical network address or a network hostname.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * A `std::unique_ptr<addrinfo, deleter>` containing the data requested.
     */
    std::pair<bool, std::unique_ptr<addrinfo, deleter>> node_addr_info_sync(const std::string &node, addrinfo *hints = nullptr);

    /**
     * @brief Async [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     * @param service Either a service name or a port number as a string.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     * @return Underlying return value.
     */
    int service_addr_info(const std::string &service, addrinfo *hints = nullptr);

    /**
     * @brief Sync [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     *
     * @param service Either a service name or a port number as a string.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * A `std::unique_ptr<addrinfo, deleter>` containing the data requested.
     */
    std::pair<bool, std::unique_ptr<addrinfo, deleter>> service_addr_info_sync(const std::string &service, addrinfo *hints = nullptr);

    /**
     * @brief Async [getaddrinfo](http://linux.die.net/man/3/getaddrinfo).
     * @param node Either a numerical network address or a network hostname.
     * @param service Either a service name or a port number as a string.
     * @param hints Optional `addrinfo` data structure with additional address
     * type constraints.
     * @return Underlying return value.
     */
    int addr_info(const std::string &node, const std::string &service, addrinfo *hints = nullptr);

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
     * * A `std::unique_ptr<addrinfo, deleter>` containing the data requested.
     */
    std::pair<bool, std::unique_ptr<addrinfo, deleter>> addr_info_sync(const std::string &node, const std::string &service, addrinfo *hints = nullptr);
};

/**
 * @brief The getnameinfo request.
 *
 * Wrapper for [getnameinfo](http://linux.die.net/man/3/getnameinfo).<br/>
 * It offers either asynchronous and synchronous access methods.
 *
 * To create a `get_name_info_req` through a `loop`, no arguments are required.
 */
class get_name_info_req final: public request<get_name_info_req, uv_getnameinfo_t, name_info_event> {
    static void name_info_callback(uv_getnameinfo_t *req, int status, const char *hostname, const char *service);

public:
    using request::request;

    /**
     * @brief Async [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     * @param addr Initialized `sockaddr_in` or `sockaddr_in6` data structure.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     * @return Underlying return value.
     */
    int name_info(const sockaddr &addr, int flags = 0);

    /**
     * @brief Async [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     * @param ip A valid IP address.
     * @param port A valid port number.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     * @return Underlying return value.
     */
    int name_info(const std::string &ip, unsigned int port, int flags = 0);

    /**
     * @brief Async [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     * @param addr A valid instance of socket_address.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     * @return Underlying return value.
     */
    int name_info(socket_address addr, int flags = 0);

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
    std::pair<bool, std::pair<const char *, const char *>> name_info_sync(const sockaddr &addr, int flags = 0);

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
    std::pair<bool, std::pair<const char *, const char *>> name_info_sync(const std::string &ip, unsigned int port, int flags = 0);

    /**
     * @brief Sync [getnameinfo](http://linux.die.net/man/3/getnameinfo).
     *
     * @param addr A valid instance of socket_address.
     * @param flags Optional flags that modify the behavior of `getnameinfo`.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * A `std::pair` composed as it follows:
     *   * A `const char *` containing a valid hostname.
     *   * A `const char *` containing a valid service name.
     */
    std::pair<bool, std::pair<const char *, const char *>> name_info_sync(socket_address addr, int flags = 0);
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "dns.cpp"
#endif

#endif // UVW_DNS_INCLUDE_H
