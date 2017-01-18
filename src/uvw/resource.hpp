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
    struct ConstructorAccess { explicit ConstructorAccess(int) {} };

    auto parent() const noexcept {
        return pLoop->loop.get();
    }

    auto get() noexcept {
        return &resource;
    }

    auto get() const noexcept {
        return const_cast<const U *>(&resource);
    }

    template<typename R>
    auto get() noexcept {
        static_assert(not std::is_same<R, U>::value, "!");
        return reinterpret_cast<R *>(&resource);
    }

    template<typename R, typename... P>
    auto get(Resource<P...> &res) noexcept {
        return reinterpret_cast<R *>(&res.resource);
    }

    template<typename R>
    auto get() const noexcept {
        static_assert(not std::is_same<R, U>::value, "!");
        return reinterpret_cast<const R *>(&resource);
    }

    template<typename R, typename... P>
    auto get(Resource<P...> &res) const noexcept {
        return reinterpret_cast<const R *>(&res.resource);
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
    explicit Resource(ConstructorAccess, std::shared_ptr<Loop> ref)
        : Emitter<T>{},
          std::enable_shared_from_this<T>{},
          pLoop{std::move(ref)},
          resource{}
    {
        resource.data = static_cast<T*>(this);
    }

    Resource(const Resource &) = delete;
    Resource(Resource &&) = delete;

    virtual ~Resource() {
        static_assert(std::is_base_of<Resource<T, U>, T>::value, "!");
    }

    Resource& operator=(const Resource &) = delete;
    Resource& operator=(Resource &&) = delete;

    /**
     * @brief Creates a new resource of the given type.
     * @param args Arguments to be forwarded to the actual constructor (if any).
     * @return A pointer to the newly created resource.
     */
    template<typename... Args>
    static std::shared_ptr<T> create(Args&&... args) {
        return std::make_shared<T>(ConstructorAccess{0}, std::forward<Args>(args)...);
    }

    /**
     * @brief Gets the loop from which the resource was originated.
     * @return A reference to a loop instance.
     */
    Loop& loop() const noexcept { return *pLoop; }

    /**
     * @brief Gets user-defined data. `uvw` won't use this field in any case.
     * @return User-defined data if any, an invalid pointer otherwise.
     */
    template<typename R = void>
    std::shared_ptr<R> data() const {
        return std::static_pointer_cast<R>(userData);
    }

    /**
     * @brief Sets arbitrary data. `uvw` won't use this field in any case.
     * @param uData User-defined arbitrary data.
     */
    void data(std::shared_ptr<void> uData) {
        userData = std::move(uData);
    }

private:
    std::shared_ptr<void> userData{nullptr};
    std::shared_ptr<void> ptr{nullptr};
    std::shared_ptr<Loop> pLoop;
    U resource;
};


}
