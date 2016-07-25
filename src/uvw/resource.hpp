#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "emitter.hpp"
#include "self.hpp"
#include "loop.hpp"


namespace uvw {


template<typename T, typename U>
class Resource: public Emitter<T>, public Self<T> {
    template<typename, typename>
    friend class Resource;

protected:
    explicit Resource(std::shared_ptr<Loop> ref)
        : Emitter<T>{},
          Self<T>{},
          pLoop{std::move(ref)},
          resource{}
    {
        this->template get<U>()->data = static_cast<T*>(this);
    }

    auto parent() const noexcept { return pLoop->loop.get(); }

    template<typename R>
    auto get() noexcept { return reinterpret_cast<R*>(&resource); }

    template<typename R>
    auto get() const noexcept { return reinterpret_cast<const R*>(&resource); }

    template<typename F, typename... Args>
    auto invoke(F &&f, Args&&... args) {
        auto err = std::forward<F>(f)(std::forward<Args>(args)...);
        if(err) { Emitter<T>::publish(ErrorEvent{err}); }
        return err;
    }

public:
    Resource(const Resource &) = delete;
    Resource(Resource &&) = delete;

    virtual ~Resource() {
        static_assert(std::is_base_of<Resource<T, U>, T>::value, "!");
    }

    Resource& operator=(const Resource &) = delete;
    Resource& operator=(Resource &&) = delete;

    Loop& loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<Loop> pLoop;
    U resource;
};


}
