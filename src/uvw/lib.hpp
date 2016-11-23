#pragma once


#include <utility>
#include <memory>
#include <string>
#include <type_traits>
#include <uv.h>
#include "loop.hpp"


namespace uvw {


/**
 * @brief The SharedLib class.
 *
 * `uvw` provides cross platform utilities for loading shared libraries and
 * retrieving symbols from them, by means of the API offered by libuv.
 */
class SharedLib final {
    explicit SharedLib(std::shared_ptr<Loop> ref, std::string filename) noexcept
        : pLoop{std::move(ref)}, lib{}
    {
        opened = (0 == uv_dlopen(filename.data(), &lib));
    }

public:
    /**
     * @brief Creates a new shared library object.
     * @param loop A pointer to the loop from which the handle generated.
     * @param filename The filename of the library in UTF8.
     * @return A pointer to the newly created handle.
     */
    static std::shared_ptr<SharedLib> create(std::shared_ptr<Loop> loop, std::string filename) noexcept {
        return std::shared_ptr<SharedLib>{new SharedLib{std::move(loop), std::move(filename)}};
    }

    SharedLib(const SharedLib &) = delete;
    SharedLib(SharedLib &&) = delete;

    ~SharedLib() noexcept {
        uv_dlclose(&lib);
    }

    SharedLib& operator=(const SharedLib &) = delete;
    SharedLib& operator=(SharedLib &&) = delete;

    /**
     * @brief Checks if the library has been correctly opened.
     * @return True if the library is opened, false otherwise.
     */
    explicit operator bool() const noexcept { return !opened; }

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
        auto err = uv_dlsym(&lib, name.data(), reinterpret_cast<void**>(&func));
        if(err) { func = nullptr; }
        return func;
    }

    /**
     * @brief Returns the last error message, if any.
     * @return The last error message, if any.
     */
    const char * error() const noexcept {
        return uv_dlerror(&lib);
    }

    /**
     * @brief Gets the loop from which the object was originated.
     * @return A reference to a loop instance.
     */
    Loop& loop() const noexcept { return *pLoop; }

private:
    std::shared_ptr<Loop> pLoop;
    uv_lib_t lib;
    bool opened;
};


}
