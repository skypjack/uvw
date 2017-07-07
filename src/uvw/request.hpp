#pragma once


#include <type_traits>
#include <utility>
#include <memory>
#include <uv.h>
#include "resource.hpp"


namespace uvw {


template<typename T, typename U>
class Request: public Resource<T, U> {
protected:
    static auto reserve(U *req) {
        auto ptr = static_cast<T*>(req->data)->shared_from_this();
        ptr->reset();
        return ptr;
    }

    template<typename E>
    static void defaultCallback(U *req, int status) {
        auto ptr = reserve(req);
        if(status) { ptr->publish(ErrorEvent{status}); }
        else { ptr->publish(E{}); }
    }

    template<typename F, typename... Args>
    auto invoke(F &&f, Args&&... args)
    -> std::enable_if_t<not std::is_void<std::result_of_t<F(Args...)>>::value> {
        auto err = std::forward<F>(f)(std::forward<Args>(args)...);
        if(err) { Emitter<T>::publish(ErrorEvent{err}); }
        else { this->leak(); }
    }

    template<typename F, typename... Args>
    auto invoke(F &&f, Args&&... args)
    -> std::enable_if_t<std::is_void<std::result_of_t<F(Args...)>>::value> {
        std::forward<F>(f)(std::forward<Args>(args)...);
        this->leak();
    }

public:
    using Resource<T, U>::Resource;

    /**
    * @brief Cancels a pending request.
    *
    * This method fails if the request is executing or has finished
    * executing.<br/>
    * It can emit an ErrorEvent event in case of errors.
    *
    * See the official
    * [documentation](http://docs.libuv.org/en/v1.x/request.html#c.uv_cancel)
    * for further details.
    *
    * @return True in case of success, false otherwise.
    */
    bool cancel() {
        return (0 == uv_cancel(this->template get<uv_req_t>()));
    }

    /**
    * @brief Returns the size of the underlying request type.
    * @return The size of the underlying request type.
    */
    std::size_t size() const noexcept {
        return uv_req_size(this->template get<uv_req_t>()->type);
    }
};


}
