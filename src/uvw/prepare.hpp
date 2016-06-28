#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "resource.hpp"
#include "util.hpp"


namespace uvw {


class Prepare final: public Resource<Prepare> {
    static void startCallback(Prepare &prepare, std::function<void(UVWError, Prepare &)> &cb, uv_prepare_t *) {
        cb(UVWError{}, prepare);
    }

    explicit Prepare(std::shared_ptr<Loop> ref)
        : Resource{HandleType<uv_prepare_t>{}, std::move(ref)}
    {
        initialized = (uv_prepare_init(parent(), get<uv_prepare_t>()) == 0);
    }

public:
    template<typename... Args>
    static std::shared_ptr<Prepare> create(Args&&... args) {
        return std::shared_ptr<Prepare>{new Prepare{std::forward<Args>(args)...}};
    }

    void start(std::function<void(UVWError, Prepare &)> cb) noexcept {
        using CBF = CallbackFactory<void(uv_prepare_t *)>;
        auto func = CBF::on<&Prepare::startCallback>(*this, cb);
        auto err = uv_prepare_start(get<uv_prepare_t>(), func);
        if(err) { error(err); }
    }

    UVWError stop() noexcept { return UVWError{uv_prepare_stop(get<uv_prepare_t>())}; }

    explicit operator bool() const noexcept { return initialized; }

private:
    bool initialized;
};


}
