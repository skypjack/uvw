#pragma once


#include <utility>
#include <memory>
#include <string>
#include <uv.h>
#include "loop.hpp"


namespace uvw {


namespace details {


template<typename T> struct IsFunc: std::false_type { };
template<typename R, typename... A> struct IsFunc<R(A...)>: std::true_type { };


}


class SharedLib final {
    explicit SharedLib(std::shared_ptr<Loop> ref, std::string filename) noexcept
        : pLoop{std::move(ref)},
          lib{}
    {
        opened = (0 == uv_dlopen(filename.data(), &lib));
    }

public:
    template<typename... Args>
    static std::shared_ptr<SharedLib> create(Args&&... args) noexcept {
        return std::shared_ptr<SharedLib>{new SharedLib{std::forward<Args>(args)...}};
    }

    SharedLib(const SharedLib &) = delete;
    SharedLib(SharedLib &&) = delete;

    ~SharedLib() noexcept {
        uv_dlclose(&lib);
    }

    SharedLib& operator=(const SharedLib &) = delete;
    SharedLib& operator=(SharedLib &&) = delete;

    explicit operator bool() const noexcept { return !opened; }

    template<typename F>
    F * sym(std::string name) {
        static_assert(details::IsFunc<F>::value, "!");
        F *func;
        auto err = uv_dlsym(&lib, name.data(), reinterpret_cast<void**>(&func));
        if(err) { func = nullptr; }
        return func;
    }

    const char * error() const noexcept {
        return uv_dlerror(&lib);
    }

    Loop& loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<Loop> pLoop;
    uv_lib_t lib;
    bool opened;
};


}
