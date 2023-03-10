#ifndef UVW_RESOURCE_INCLUDE_H
#define UVW_RESOURCE_INCLUDE_H

#include <memory>
#include <utility>
#include "config.h"
#include "emitter.h"
#include "uv_type.hpp"

namespace uvw {

/**
 * @brief Common class for almost all the resources available in `uvw`.
 *
 * This is the base class for handles and requests.
 */
template<typename T, typename U, typename... E>
class resource: public uv_type<U>, public emitter<T, E...>, public std::enable_shared_from_this<T> {
protected:
    int leak_if(int err) noexcept {
        if(err == 0) {
            self_ptr = this->shared_from_this();
        }

        return err;
    }

    void self_reset() noexcept {
        self_ptr.reset();
    }

    bool has_self() const noexcept {
        return static_cast<bool>(self_ptr);
    }

public:
    explicit resource(loop::token token, std::shared_ptr<loop> ref)
        : uv_type<U>{token, std::move(ref)} {
        this->raw()->data = this;
    }

    /**
     * @brief Gets user-defined data. `uvw` won't use this field in any case.
     * @return User-defined data if any, an invalid pointer otherwise.
     */
    template<typename R = void>
    std::shared_ptr<R> data() const {
        return std::static_pointer_cast<R>(user_data);
    }

    /**
     * @brief Sets arbitrary data. `uvw` won't use this field in any case.
     * @param udata User-defined arbitrary data.
     */
    void data(std::shared_ptr<void> udata) {
        user_data = std::move(udata);
    }

private:
    std::shared_ptr<void> user_data{nullptr};
    std::shared_ptr<void> self_ptr{nullptr};
};

} // namespace uvw

#endif // UVW_RESOURCE_INCLUDE_H
