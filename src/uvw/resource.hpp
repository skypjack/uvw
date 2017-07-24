#pragma once


#include <memory>
#include <utility>
#include "emitter.hpp"
#include "resource_base.hpp"


namespace uvw {


/**
 * @brief Common class for almost all the resources available in `uvw`.
 *
 * This is the base class for handles and requests.<br/>
 * Beyond `ResourceBase`, it stores a UserData and a self-referencing shared_ptr.
 */
template<typename T, typename U>
class Resource: public ResourceBase<T, U>, public Emitter<T>,
                public std::enable_shared_from_this<T> {

protected:
    auto parent() const noexcept {
        return this->loop().loop.get();
    }

    void leak() noexcept {
        sPtr = this->shared_from_this();
    }

    void reset() noexcept {
        sPtr.reset();
    }

    bool self() const noexcept {
        return static_cast<bool>(sPtr);
    }

public:
    explicit Resource(typename ResourceBase<T, U>::ConstructorAccess ca,
                      std::shared_ptr<Loop> ref)
        : ResourceBase<T, U>{std::move(ca), std::move(ref)},
          Emitter<T>{},
          std::enable_shared_from_this<T>{}
    {
        this->get()->data = static_cast<T*>(this);
    }

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
    std::shared_ptr<void> sPtr{nullptr};
};


}
