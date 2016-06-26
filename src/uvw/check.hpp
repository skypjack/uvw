#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "resource.hpp"
#include "error.hpp"


namespace uvw {


class Check final: public Resource<Check> {
    static void startCallback(Check &ref, uv_check_t* h) {
        ref.startCb(UVWError{});
        ref.startCb = nullptr;
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

    void start(std::function<void(UVWError)> cb) noexcept {
        auto func = Callback<void(uv_check_t*)>::get<&Check::startCallback>(*this);
        auto err = uv_check_start(get<uv_check_t>(), func);

        if(err) {
            startCb(UVWError{err});
        } else {
            startCb = std::move(cb);
        }
    }

    UVWError stop() noexcept { return UVWError{uv_check_stop(get<uv_check_t>())}; }

    explicit operator bool() { return initialized; }

private:
    std::function<void(UVWError)> startCb;
    bool initialized;
};


}
