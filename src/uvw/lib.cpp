#ifdef UVW_AS_LIB
#    include "lib.h"
#endif

#include <utility>
#include "config.h"

namespace uvw {

UVW_INLINE shared_lib::shared_lib(loop::token token, std::shared_ptr<loop> ref, const std::string &filename) noexcept
    : uv_type{token, std::move(ref)} {
    opened = (0 == uv_dlopen(filename.data(), raw()));
}

UVW_INLINE shared_lib::~shared_lib() noexcept {
    uv_dlclose(raw());
}

UVW_INLINE shared_lib::operator bool() const noexcept {
    return opened;
}

UVW_INLINE const char *shared_lib::error() const noexcept {
    return uv_dlerror(raw());
}

} // namespace uvw
