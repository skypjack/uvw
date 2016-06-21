#pragma once


#include <stdexcept>
#include <uv.h>


namespace uvw {


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


}
