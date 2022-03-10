#ifndef UVW_CONFIG_H
#define UVW_CONFIG_H

#ifndef UVW_AS_LIB
#    define UVW_INLINE inline
#else
#    define UVW_INLINE
#endif

#if defined(UVW_NOEXCEPTION)
#    define UVW_NOEXCEPT
#else
#    define UVW_NOEXCEPT noexcept
#endif

#endif
