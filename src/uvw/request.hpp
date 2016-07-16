#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "resource.hpp"


namespace uvw {


template<typename T>
struct RequestType;

template<> struct RequestType<uv_shutdown_t> { };
template<> struct RequestType<uv_work_t> { };


template<typename T>
class Request: public Resource<T> {
protected:
    using Resource<T>::Resource;

public:
    void cancel() noexcept {
        invoke(&uv_cancel, this->template get<uv_req_t>());
    }

    std::size_t size() const noexcept {
        return uv_req_size(this->template get<uv_req_t>()->type);
    }
};


}
