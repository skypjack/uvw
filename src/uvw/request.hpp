#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "resource.hpp"


namespace uvw {


template<typename T>
class Request: public Resource<T> {
protected:
    template<typename U>
    explicit Request(ResourceType<U> rt, std::shared_ptr<Loop> ref)
        : Resource<T>{std::move(rt), std::move(ref)}
    { }

public:
    void cancel() noexcept {
        invoke(&uv_cancel, this->template get<uv_req_t>());
    }

    std::size_t size() const noexcept {
        return uv_req_size(this->template get<uv_req_t>()->type);
    }
};


}
