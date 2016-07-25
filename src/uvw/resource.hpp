#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "emitter.hpp"
#include "self.hpp"
#include "loop.hpp"


namespace uvw {


template<typename T>
class Resource: public Emitter<T>, public Self<T> {
    using Deleter = void(*)(void *);

    template<typename U>
    friend class Resource;

protected:
    template<typename U, template<typename> class R>
    explicit Resource(R<U>, std::shared_ptr<Loop> ref)
        : Emitter<T>{},
          Self<T>{},
          resource{new U, [](void *res){ delete static_cast<U*>(res); }},
          pLoop{std::move(ref)}
    {
        this->template get<U>()->data = static_cast<T*>(this);
    }

    uv_loop_t* parent() const noexcept { return pLoop->loop.get(); }

    template<typename U>
    U* get() const noexcept { return reinterpret_cast<U*>(resource.get()); }

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
        static_assert(std::is_base_of<Resource<T>, T>::value, "!");
    }

    Resource& operator=(const Resource &) = delete;
    Resource& operator=(Resource &&) = delete;

    Loop& loop() const noexcept { return *pLoop; }

private:
    std::unique_ptr<void, Deleter> resource;
    std::shared_ptr<Loop> pLoop;
};


}
