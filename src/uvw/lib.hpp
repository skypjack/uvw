#pragma once


#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <uv.h>
#include "loop.hpp"
#include "underlying_type.hpp"


namespace uvw {


/**
 * @brief The SharedLib class.
 *
 * `uvw` provides cross platform utilities for loading shared libraries and
 * retrieving symbols from them, by means of the API offered by `libuv`.
 */
class SharedLib final: public UnderlyingType<SharedLib, uv_lib_t> {
public:
    explicit SharedLib(ConstructorAccess ca, std::shared_ptr<Loop> ref, std::string filename) noexcept
        : UnderlyingType{ca, std::move(ref)}
    {
        opened = (0 == uv_dlopen(filename.data(), get()));
    }

    ~SharedLib() noexcept {
        uv_dlclose(get());
    }

    /**
     * @brief Checks if the library has been correctly opened.
     * @return True if the library is opened, false otherwise.
     */
    explicit operator bool() const noexcept { return opened; }

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
    F * sym(std::string name) {
        static_assert(std::is_function<F>::value, "!");
        F *func;
        auto err = uv_dlsym(get(), name.data(), reinterpret_cast<void**>(&func));
        if(err) { func = nullptr; }
        return func;
    }

    /**
     * @brief Returns the last error message, if any.
     * @return The last error message, if any.
     */
    const char * error() const noexcept {
        return uv_dlerror(get());
    }

private:
    bool opened;
};


}
