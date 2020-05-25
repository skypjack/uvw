#ifndef UVW_CONFIG_H
#define UVW_CONFIG_H

#if defined(BUILDING_UVW_SHARED) && defined(USING_UVW_SHARED)
#error "Define either BUILDING_UVW_SHARED or USING_UVW_SHARED, not both."
#endif

#ifndef UVW_AS_LIB
#define UVW_INLINE inline
#define UVW_EXTERN /* nothing */
#else /* UVW_AS_LIB */
#define UVW_INLINE /* nothing */

#ifdef _WIN32
/* Windows - set up dll import/export decorators. */
#if defined(BUILDING_UVW_SHARED)
/* Building shared library. */
#define UVW_EXTERN __declspec(dllexport)
#elif defined(USING_UVW_SHARED)
/* Using shared library. */
#define UVW_EXTERN __declspec(dllimport)
#else
/* Building static library. */
#define UVW_EXTERN /* nothing */
#endif
#elif __GNUC__ >= 4
#define UVW_EXTERN __attribute__((visibility("default")))
#else
#define UVW_EXTERN /* nothing */
#endif

#endif /* UVW_AS_LIB */

#if defined(_MSC_VER) && defined(UVW_AS_LIB)
/*
 * C4251: 'type' : class 'type1' needs to have dll-interface to be used by clients of class 'type2'
 */
#define UVW_MSVC_WARNING_PUSH_DISABLE_DLLINTERFACE() \
    __pragma(warning(push)) \
    __pragma(warning(disable: 4251))
#define UVW_MSVC_WARNING_POP() \
    __pragma(warning(pop))
#else
#define UVW_MSVC_WARNING_PUSH_DISABLE_DLLINTERFACE()
#define UVW_MSVC_WARNING_POP()
#endif

#endif
