#pragma once


#include <stdexcept>
#include <cstddef>
#include <utility>
#include <string>
#include <uv.h>


namespace uvw {


namespace details {


struct IPv4 { };
struct IPv6 { };


template<typename>
struct IpTraits;

template<>
struct IpTraits<IPv4> {
    using Type = sockaddr_in;
    using AddrFuncType = int(*)(const char *, int, sockaddr_in *);
    using NameFuncType = int(*)(const sockaddr_in *, char *, std::size_t);
    static const AddrFuncType AddrFunc;
    static const NameFuncType NameFunc;
};

template<>
struct IpTraits<IPv6> {
    using Type = sockaddr_in6;
    using AddrFuncType = int(*)(const char *, int, sockaddr_in6 *);
    using NameFuncType = int(*)(const sockaddr_in6 *, char *, std::size_t);
    static const AddrFuncType AddrFunc;
    static const NameFuncType NameFunc;
};

const IpTraits<IPv4>::AddrFuncType IpTraits<IPv4>::AddrFunc = uv_ip4_addr;
const IpTraits<IPv6>::AddrFuncType IpTraits<IPv6>::AddrFunc = uv_ip6_addr;
const IpTraits<IPv4>::NameFuncType IpTraits<IPv4>::NameFunc = uv_ip4_name;
const IpTraits<IPv6>::NameFuncType IpTraits<IPv6>::NameFunc = uv_ip6_name;


}


class UVWException final: std::runtime_error {
public:
    explicit UVWException(int code)
        : runtime_error{uv_strerror(code)}, ec{code}
    { }

    const char* name() const noexcept {
        return uv_err_name(ec);
    }

    const char* error() const noexcept {
        return uv_strerror(ec);
    }

private:
    int ec;
};


class UVWError {
public:
    explicit UVWError(int code = 0): ec(code) { }

public:
    explicit operator bool() const noexcept { return !(ec == 0); }
    operator const char *() const noexcept { return uv_strerror(ec); }
    operator int() const noexcept { return ec; }

private:
    int ec;
};


template<typename T>
class UVWOptionalData {
public:
    UVWOptionalData(UVWError e): err{e}, value{} { }
    UVWOptionalData(T t): err{}, value{std::move(t)} { }

    const UVWError & error() const noexcept { return err; }
    const T & data() const noexcept { return value; }

    operator const UVWError &() const noexcept { return error(); }
    operator const T &() const noexcept { return data(); }

    explicit operator bool() const noexcept { return !err; }

private:
    UVWError err;
    T value;
};


using Addr = std::pair<std::string, unsigned int>;


}
