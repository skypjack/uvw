#ifdef UVW_AS_LIB
#    include "lib.h"
#endif

#include <utility>
#include "config.h"

namespace uvw {

UVW_INLINE shared_lib::shared_lib(loop::token token, std::shared_ptr<loop> ref, const std::string &filename) UVW_NOEXCEPT
    : uv_type{token, std::move(ref)} {
    opened = (0 == uv_dlopen(filename.data(), raw()));
}

UVW_INLINE shared_lib::~shared_lib() UVW_NOEXCEPT {
    uv_dlclose(raw());
}

UVW_INLINE shared_lib::operator bool() const UVW_NOEXCEPT {
    return opened;
}

UVW_INLINE const char *shared_lib::error() const UVW_NOEXCEPT {
    return uv_dlerror(raw());
}

} // namespace uvw
