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
};


}
