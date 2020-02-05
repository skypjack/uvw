#include "lib.h"

namespace uvw {

    SharedLib::SharedLib(UnderlyingType<SharedLib, uv_lib_t>::ConstructorAccess ca, std::shared_ptr<Loop> ref,
                         std::string filename) noexcept
            : UnderlyingType{ca, std::move(ref)}
    {
        opened = (0 == uv_dlopen(filename.data(), get()));
    }

    SharedLib::~SharedLib() noexcept {
        uv_dlclose(get());
    }

    SharedLib::operator bool() const noexcept { return opened; }

    const char *SharedLib::error() const noexcept {
        return uv_dlerror(get());
    }
}
