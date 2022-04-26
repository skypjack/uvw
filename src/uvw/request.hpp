#ifndef UVW_REQUEST_INCLUDE_H
#define UVW_REQUEST_INCLUDE_H

#include <memory>
#include <type_traits>
#include <utility>
#include <uv.h>
#include "config.h"
#include "resource.hpp"

namespace uvw {

/**
 * @brief Request base class.
 *
 * Base type for all `uvw` request types.
 */
template<typename T, typename U, typename... E>
class request: public resource<T, U, E...> {
protected:
    static auto reserve(U *req) {
        auto ptr = static_cast<T *>(req->data)->shared_from_this();
        ptr->self_reset();
        return ptr;
    }

public:
    using resource<T, U, E...>::resource;

    /**
     * @brief Cancels a pending request.
     *
     * This method fails if the request is executing or has finished
     * executing.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/request.html#c.uv_cancel)
     * for further details.
     *
     * @return Underlying return value.
     */
    int cancel() {
        return uv_cancel(reinterpret_cast<uv_req_t *>(this->raw()));
    }

    /**
     * @brief Returns the size of the underlying request type.
     * @return The size of the underlying request type.
     */
    std::size_t size() const noexcept {
        return uv_req_size(reinterpret_cast<const uv_req_t *>(this->raw())->type);
    }
};

} // namespace uvw

#endif // UVW_REQUEST_INCLUDE_H
