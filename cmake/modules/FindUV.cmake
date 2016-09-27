# FindUV
# ---------
#
# Locate libuv
#
# This module defines:
#
# ::
#
#   UV_INCLUDE_DIRS, where to find the headers
#   UV_LIBRARIES, the libraries against which to link
#   UV_FOUND, if false, do not try to use the above mentioned vars
#

set(BUILD_DEPS_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_DEPS_DIR})
set(UV_DEPS_DIR libuv)

find_path(
    UV_INCLUDE_DIR NAMES uv.h
    PATHS ${BUILD_DEPS_DIR}/${UV_DEPS_DIR}/include/
    NO_DEFAULT_PATH
)

find_library(
    UV_STATIC_LIBRARY NAMES libuv.a libuv
    PATHS ${BUILD_DEPS_DIR}/${UV_DEPS_DIR}
    PATH_SUFFIXES .libs Release
    NO_DEFAULT_PATH
)

find_library(
    UV_SHARED_LIBRARY NAMES uv libuv
    PATHS ${BUILD_DEPS_DIR}/${UV_DEPS_DIR}
    PATH_SUFFIXES .libs Release
    NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    UV
    FOUND_VAR UV_FOUND
    REQUIRED_VARS
        UV_STATIC_LIBRARY
        UV_SHARED_LIBRARY
        UV_INCLUDE_DIR
)

if(UV_FOUND)
    set(
        UV_LIBRARIES
        ${UV_STATIC_LIBRARY}
        ${UV_SHARED_LIBRARY}
    )

    set(
        UV_INCLUDE_DIRS
        ${UV_INCLUDE_DIR}
    )
endif(UV_FOUND)


mark_as_advanced(
    UV_INCLUDE_DIR
    UV_STATIC_LIBRARY
    UV_SHARED_LIBRARY
)
