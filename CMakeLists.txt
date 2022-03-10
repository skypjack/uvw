#
# uvw
#

cmake_minimum_required(VERSION 3.13)

#
# Building in-tree is not allowed (we take care of your craziness).
#

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

#
# Project configuration
#
set(UVW_VERSION_MAJOR 2)
set(UVW_VERSION_MINOR 12)
set(UVW_VERSION_PATCH 1)

project(
    uvw
    VERSION ${UVW_VERSION_MAJOR}.${UVW_VERSION_MINOR}.${UVW_VERSION_PATCH}
    DESCRIPTION "Header-only, event based, tiny and easy to use libuv wrapper in modern C++ - now available also as static library!"
    HOMEPAGE_URL "https://github.com/skypjack/uvw"
    LANGUAGES C CXX
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

option(USE_LIBCPP "Use libc++ by adding -stdlib=libc++ flag if availbale." ON)
option(USE_ASAN "Use address sanitizer by adding -fsanitize=address -fno-omit-frame-pointer flags" OFF)
option(USE_UBSAN "Use address sanitizer by adding -fsanitize=undefined -fno-sanitize-recover=all -fno-omit-frame-pointer flags" OFF)
option(BUILD_UVW_LIBS "Prepare targets for static library rather than for a header-only library." OFF)
option(BUILD_UVW_SHARED_LIB "Prepare targets for shared library rather than for a header-only library." OFF)
option(FIND_LIBUV "Try finding libuv library development files in the system" OFF)

if(BUILD_UVW_SHARED_LIB)
    set(BUILD_UVW_LIBS BOOL:ON)
endif()

#
# Compiler stuff
#

if(NOT WIN32 AND USE_LIBCPP)
    include(CheckCXXSourceCompiles)
    include(CMakePushCheckState)

    cmake_push_check_state()

    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -stdlib=libc++")

    check_cxx_source_compiles("
        #include<type_traits>
        int main() { return std::is_same_v<int, char>; }
    " HAS_LIBCPP)

    if(NOT HAS_LIBCPP)
        message(WARNING "The option USE_LIBCPP is set (by default) but libc++ is not available. The flag will not be added to the target.")
    endif()

    cmake_pop_check_state()
endif()


# Required minimal libuv version
set(LIBUV_VERSION 1.44.1)

function(fetch_libuv)
    if (FETCH_LIBUV)
        include(FetchContent)

        FetchContent_Declare(
            libuv
            GIT_REPOSITORY https://github.com/libuv/libuv.git
            GIT_TAG "v${LIBUV_VERSION}"
            GIT_SHALLOW 1
        )

        FetchContent_GetProperties(libuv)

        if(NOT libuv_POPULATED)
            FetchContent_Populate(libuv)
            add_subdirectory(${libuv_SOURCE_DIR} ${libuv_BINARY_DIR} EXCLUDE_FROM_ALL)
        endif()

        if(BUILD_UVW_SHARED_LIB)
            add_library(uv::uv-shared ALIAS uv)
            set_target_properties(uv PROPERTIES POSITION_INDEPENDENT_CODE 1)
        else()
            add_library(uv::uv-static ALIAS uv_a)
            set_target_properties(uv_a PROPERTIES POSITION_INDEPENDENT_CODE 1)
        endif()
    endif(FETCH_LIBUV)
endfunction()

function(use_libuv)
    set(FETCH_LIBUV_DEFAULT ON)
    if (FIND_LIBUV)
        find_package(libuv ${LIBUV_VERSION} QUIET)
        if (libuv_FOUND)
            add_library(uv::uv-shared ALIAS uv)
            set(FETCH_LIBUV_DEFAULT OFF)
            message(STATUS "libuv ${libuv_VERSION} found via cmake")
        else(libuv_FOUND)
            find_package(PkgConfig QUIET)
            if (PkgConfig_FOUND)
                pkg_check_modules(libuv IMPORTED_TARGET libuv>=${LIBUV_VERSION})
                if (libuv_FOUND)
                    add_library(uv::uv-shared ALIAS PkgConfig::libuv)
                    set(FETCH_LIBUV_DEFAULT OFF)
                    message(STATUS "libuv ${libuv_VERSION} found via pkg-config")
                endif(libuv_FOUND)
            endif(PkgConfig_FOUND)
        endif(libuv_FOUND)
    endif(FIND_LIBUV)

    option(FETCH_LIBUV "Fetch the libuv repo using CMake FetchContent facility" ${FETCH_LIBUV_DEFAULT})
    fetch_libuv()
endfunction()

#
# Add uvw target
#

include(GNUInstallDirs)


if(BUILD_UVW_LIBS)
    use_libuv()

    add_subdirectory(src)
    file(GLOB HEADERS src/uvw/*.h src/uvw/*.hpp)
else()
    add_library(uvw INTERFACE)
    add_library(uvw::uvw ALIAS uvw)

    target_compile_features(uvw INTERFACE cxx_std_17)

    target_include_directories(
        uvw
        INTERFACE
            $<BUILD_INTERFACE:${uvw_SOURCE_DIR}/src>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    if(USE_ASAN)
        target_compile_options(uvw INTERFACE $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer>)
        target_link_libraries(uvw INTERFACE $<$<CONFIG:Debug>:-fsanitize=address>)
    endif()

    if(USE_UBSAN)
        target_compile_options(uvw INTERFACE $<$<CONFIG:Debug>:-fsanitize=undefined -fno-sanitize-recover=all -fno-omit-frame-pointer>)
        target_link_libraries(uvw INTERFACE $<$<CONFIG:Debug>:-fsanitize=undefined>)
    endif()

    if(HAS_LIBCPP)
        target_compile_options(uvw BEFORE INTERFACE -stdlib=libc++)
    endif()

    file(GLOB HEADERS src/uvw/*.h src/uvw/*.hpp src/uvw/*.cpp)
endif()

#
# Install targets
#

install(
    FILES ${HEADERS}
    COMPONENT ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/uvw
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

install(
    FILES src/uvw.hpp
    COMPONENT ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

#
# Install targets
#

install(EXPORT uvwConfig NAMESPACE uvw:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/uvw)
install(TARGETS uvw EXPORT uvwConfig ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
if(FETCH_LIBUV AND BUILD_UVW_LIBS)
    # libuv is only fetched when both above conditions are true
    install(DIRECTORY ${libuv_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/uvw/uv/include)
    if (BUILD_UVW_SHARED_LIB)
        install(TARGETS uv EXPORT uvwConfig LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/uvw)
    else()
        install(TARGETS uv_a EXPORT uvwConfig ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/uvw)
    endif()
    
endif(FETCH_LIBUV AND BUILD_UVW_LIBS)

### Testing

option(BUILD_TESTING "Enable testing with ctest." OFF)

if(BUILD_TESTING)
    option(FIND_GTEST_PACKAGE "Enable finding gtest package." OFF)

    if (NOT BUILD_UVW_LIBS)
        use_libuv()
    endif()

    enable_testing()
    add_subdirectory(test)
endif()

#
# Documentation
#

option(BUILD_DOCS "Enable building with documentation." OFF)

if(BUILD_DOCS)
    find_package(Doxygen 1.8)

    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()

#
# AOB
#

add_custom_target(
    uvw_aob
    SOURCES
        .github/workflows/build.yml
        .github/workflows/coverage.yml
        .github/workflows/deploy.yml
        .github/FUNDING.yml
        AUTHORS
        LICENSE
        README.md
)
