#pragma once


#include <memory>
#include <type_traits>
#include <utility>
#include "loop.hpp"


namespace uvw {


/**
 * @brief Wrapper class for underlying types.
 *
 * It acts mainly as a wrapper around data structures of the underlying library.
 */
template<typename T, typename U>
class UnderlyingType {
    template<typename, typename>
    friend class UnderlyingType;

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

    template<typename R, typename... P>
    auto get(UnderlyingType<P...> &other) noexcept {
        return reinterpret_cast<R *>(&other.resource);
    }

    template<typename R>
    auto get() const noexcept {
        static_assert(not std::is_same<R, U>::value, "!");
        return reinterpret_cast<const R *>(&resource);
    }

    template<typename R, typename... P>
    auto get(const UnderlyingType<P...> &other) const noexcept {
        return reinterpret_cast<const R *>(&other.resource);
    }

public:
    explicit UnderlyingType(ConstructorAccess, std::shared_ptr<Loop> ref) noexcept
        : pLoop{std::move(ref)}, resource{}
    {}

    UnderlyingType(const UnderlyingType &) = delete;
    UnderlyingType(UnderlyingType &&) = delete;

    virtual ~UnderlyingType() {
        static_assert(std::is_base_of<UnderlyingType<T, U>, T>::value, "!");
    }

    UnderlyingType & operator=(const UnderlyingType &) = delete;
    UnderlyingType & operator=(UnderlyingType &&) = delete;

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
