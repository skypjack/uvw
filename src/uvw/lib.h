#ifndef UVW_LIB_INCLUDE_H
#define UVW_LIB_INCLUDE_H

#include <memory>
#include <string>
#include <type_traits>
#include <uv.h>
#include "config.h"
#include "loop.h"
#include "uv_type.hpp"

namespace uvw {

/**
 * @brief The shared lib class.
 *
 * `uvw` provides cross platform utilities for loading shared libraries and
 * retrieving symbols from them, by means of the API offered by `libuv`.
 */
class shared_lib final: public uv_type<uv_lib_t> {
public:
    explicit shared_lib(loop::token token, std::shared_ptr<loop> ref, const std::string &filename) noexcept;

    ~shared_lib() noexcept;

    /**
     * @brief Checks if the library has been correctly opened.
     * @return True if the library is opened, false otherwise.
     */
    explicit operator bool() const noexcept;

    /**
     * @brief Retrieves a data pointer from a dynamic library.
     *
     * `F` shall be a valid function type (as an example, `void(int)`).<br/>
     * It is legal for a symbol to map to `nullptr`.
     *
     * @param name The symbol to be retrieved.
     * @return A valid function pointer in case of success, `nullptr` otherwise.
     */
    template<typename F>
    F *sym(const std::string &name) {
        static_assert(std::is_function_v<F>);
        F *func;
        auto err = uv_dlsym(raw(), name.data(), reinterpret_cast<void **>(&func));
        if(err) { func = nullptr; }
        return func;
    }

    /**
     * @brief Returns the last error message, if any.
     * @return The last error message, if any.
     */
    const char *error() const noexcept;

private:
    bool opened;
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "lib.cpp"
#endif

#endif // UVW_LIB_INCLUDE_H
