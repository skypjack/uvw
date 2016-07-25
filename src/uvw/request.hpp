#pragma once


#include <type_traits>
#include <utility>
#include <memory>
#include <uv.h>
#include "resource.hpp"


namespace uvw {


template<typename T, typename U>
class Request: public Resource<T, U> {
    template<typename R, typename E>
    static void execCallback(R *req, int status) {
        T &res = *(static_cast<T*>(req->data));

        auto ptr = res.shared_from_this();
        (void)ptr;

        res.reset();

        if(status) {
            res.publish(ErrorEvent{status});
        } else {
            res.publish(E{});
        }
    }

protected:
    using Resource<T, U>::Resource;

    template<typename R, typename E, typename F, typename... Args>
    auto exec(F &&f, Args&&... args)
    -> std::enable_if_t<not std::is_void<std::result_of_t<F(Args..., decltype(&execCallback<R, E>))>>::value, int> {
        auto ret = this->invoke(std::forward<F>(f), std::forward<Args>(args)..., &execCallback<R, E>);
        if(0 == ret) { this->leak(); }
        return ret;
    }

    template<typename R, typename E, typename F, typename... Args>
    auto exec(F &&f, Args&&... args)
    -> std::enable_if_t<std::is_void<std::result_of_t<F(Args..., decltype(&execCallback<R, E>))>>::value> {
        std::forward<F>(f)(std::forward<Args>(args)..., &execCallback<R, E>);
        this->leak();
    }

public:
    void cancel() {
        this->invoke(&uv_cancel, this->template get<uv_req_t>());
    }

    std::size_t size() const noexcept {
        return uv_req_size(this->template get<uv_req_t>()->type);
    }
};


}
