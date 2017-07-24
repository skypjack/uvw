#pragma once


#include <memory>
#include <type_traits>
#include <utility>
#include "loop.hpp"


namespace uvw {


/**
 * @brief Internal `class uvw::ResourceBase`
 *
 * A base resource class to define the `create` method in one place.<br/>
 * It mainly acts as a wrapper around a data structure of `libuv`.
 */
template<typename T, typename U>
class ResourceBase {
protected:
    struct ConstructorAccess { explicit ConstructorAccess(int) {} };

    auto get() noexcept {
        return &resource;
    }

    auto get() const noexcept {
        return &resource;
    }

    template<typename R>
    auto get() noexcept {
        static_assert(not std::is_same<R, U>::value, "!");
        return reinterpret_cast<R *>(&resource);
    }

    template<typename R>
    auto get() const noexcept {
        static_assert(not std::is_same<R, U>::value, "!");
        return reinterpret_cast<const R *>(&resource);
    }

public:
    explicit ResourceBase(ConstructorAccess, std::shared_ptr<Loop> ref) noexcept
        : pLoop{std::move(ref)}, resource{}
    {}

    ResourceBase(const ResourceBase &) = delete;
    ResourceBase(ResourceBase &&) = delete;

    virtual ~ResourceBase() {
        static_assert(std::is_base_of<ResourceBase<T, U>, T>::value, "!");
    }

    ResourceBase & operator=(const ResourceBase &) = delete;
    ResourceBase & operator=(ResourceBase &&) = delete;

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
    Loop & loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<Loop> pLoop;
    U resource;
};


}
