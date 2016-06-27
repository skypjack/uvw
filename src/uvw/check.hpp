#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "resource.hpp"
#include "error.hpp"


namespace uvw {


class Check final: public Resource<Check> {
    static void startCallback(Check &check, std::function<void(UVWError, Check &)> &cb, uv_check_t *) {
        cb(UVWError{}, check);
    }

    explicit Check(std::shared_ptr<Loop> ref)
        : Resource{HandleType<uv_check_t>{}, std::move(ref)}
    {
        initialized = (uv_check_init(parent(), get<uv_check_t>()) == 0);
    }

public:
    template<typename... Args>
    static std::shared_ptr<Check> create(Args&&... args) {
        return std::shared_ptr<Check>{new Check{std::forward<Args>(args)...}};
    }

    void start(std::function<void(UVWError, Check &)> cb) noexcept {
        using CB = Callback<void(uv_check_t *)>;
        auto func = CB::on<&Check::startCallback>(*this, cb);
        auto err = uv_check_start(get<uv_check_t>(), func);
        if(err) { cb(UVWError{err}, *this); }
    }

    UVWError stop() noexcept { return UVWError{uv_check_stop(get<uv_check_t>())}; }

    explicit operator bool() { return initialized; }

private:
    bool initialized;
};


}
