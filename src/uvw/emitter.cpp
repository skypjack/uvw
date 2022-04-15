#ifdef UVW_AS_LIB
#    include "emitter.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE int error_event::translate(int sys) noexcept {
    return uv_translate_sys_error(sys);
}

UVW_INLINE const char *error_event::what() const noexcept {
    return uv_strerror(ec);
}

UVW_INLINE const char *error_event::name() const noexcept {
    return uv_err_name(ec);
}

UVW_INLINE int error_event::code() const noexcept {
    return ec;
}

UVW_INLINE error_event::operator bool() const noexcept {
    return ec < 0;
}

} // namespace uvw
