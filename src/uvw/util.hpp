#pragma once


#include <stdexcept>
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
    using FuncType = int(*)(const char *, int, sockaddr_in *);
    static const FuncType AddrFunc;
};

template<>
struct IpTraits<IPv6> {
    using Type = sockaddr_in6;
    using FuncType = int(*)(const char *, int, sockaddr_in6 *);
    static const FuncType AddrFunc;
};

const IpTraits<IPv4>::FuncType IpTraits<IPv4>::AddrFunc = uv_ip4_addr;
const IpTraits<IPv6>::FuncType IpTraits<IPv6>::AddrFunc = uv_ip6_addr;


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
    UVWOptionalData(UVWError e): err{e}, value{} { }
    UVWOptionalData(T t): err{}, value{std::move(t)} { }

    const UVWError & error() const noexcept { return err; }
    const T & data() const noexcept { return value; }

    operator const UVWError &() const noexcept { return error(); }
    operator const T &() const noexcept { return data(); }

    explicit operator bool() const noexcept { return static_cast<bool>(err); }

private:
    UVWError err;
    T value;
};


struct Addr {
    const std::string ip;
    const unsigned int port;
};


}
