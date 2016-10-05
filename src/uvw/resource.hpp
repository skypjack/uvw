#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "emitter.hpp"
#include "loop.hpp"


namespace uvw {


/**
 * @brief Common class for almost all the resources available in `uvw`.
 *
 * This is the base class for handles and requests.<br/>
 * It mainly acts as a wrapper around a libuv's data structure.
 */
template<typename T, typename U>
class Resource: public Emitter<T>, public std::enable_shared_from_this<T> {
    template<typename, typename>
    friend class Resource;

protected:
    explicit Resource(std::shared_ptr<Loop> ref)
        : Emitter<T>{},
          std::enable_shared_from_this<T>{},
          pLoop{std::move(ref)},
          resource{}
    {
        resource.data = static_cast<T*>(this);
    }

    auto parent() const noexcept {
        return pLoop->loop.get();
    }

    auto get() noexcept {
        return &resource;
    }

    const auto get() const noexcept {
        return &resource;
    }

    template<typename R>
    auto get() noexcept {
        static_assert(not std::is_same<R, U>::value, "!");
        return reinterpret_cast<R*>(&resource);
    }

    template<typename R>
    auto get() const noexcept {
        static_assert(not std::is_same<R, U>::value, "!");
        return reinterpret_cast<const R*>(&resource);
    }

    void leak() noexcept {
        ptr = this->shared_from_this();
    }

    void reset() noexcept {
        ptr.reset();
    }

    bool self() const noexcept {
        return static_cast<bool>(ptr);
    }

public:
    Resource(const Resource &) = delete;
    Resource(Resource &&) = delete;

    virtual ~Resource() {
        static_assert(std::is_base_of<Resource<T, U>, T>::value, "!");
    }

    Resource& operator=(const Resource &) = delete;
    Resource& operator=(Resource &&) = delete;

    /**
     * @brief Gets the loop from which the resource was originated.
     * @return A reference to a loop instance.
     */
    Loop& loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<void> ptr{nullptr};
    std::shared_ptr<Loop> pLoop;
    U resource;
};


}
