#ifndef UVW_CONFIG_H
#define UVW_CONFIG_H


#ifndef UVW_AS_LIB
#define UVW_INLINE inline
#else
#define UVW_INLINE
#endif


#define LIBUV_VERSION_AT_LEAST(a, b, c) (UV_VERSION_HEX >= (((a) << 16) | ((b) << 8) | (c)))


#endif
