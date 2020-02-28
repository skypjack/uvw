#include "lib.h"
#include "defines.h"


namespace uvw {

    UVW_INLINE_SPECIFIER SharedLib::SharedLib(UnderlyingType<SharedLib, uv_lib_t>::ConstructorAccess ca,
                                              std::shared_ptr<Loop> ref, std::string filename) noexcept
            :
            UnderlyingType{ca, std::move(ref)} {
        opened = (0 == uv_dlopen(filename.data(), get()));
    }

    UVW_INLINE_SPECIFIER SharedLib::~SharedLib() noexcept {
        uv_dlclose(get());
    }

    UVW_INLINE_SPECIFIER SharedLib::operator bool() const noexcept {
        return opened;
    }

    UVW_INLINE_SPECIFIER const char *SharedLib::error() const noexcept {
        return uv_dlerror(get());
    }
}
