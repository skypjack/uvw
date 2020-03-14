#
# uvw
#

cmake_minimum_required(VERSION 3.15.3)

#
# Building in-tree is not allowed (we take care of your craziness).
#

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

#
# Project configuration
#

project(
    uvw
    VERSION 2.3.1
    DESCRIPTION "Header-only, event based, tiny and easy to use libuv wrapper in modern C++"
    HOMEPAGE_URL "https://github.com/skypjack/uvw"
    LANGUAGES C CXX
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message("*")
message("* ${PROJECT_NAME} v${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
message("* Copyright (c) 2016-2020 Michele Caini <michele.caini@gmail.com>")
message("*")

option(USE_LIBCPP "Use libc++ by adding -stdlib=libc++ flag if availbale." ON)
option(USE_ASAN "Use address sanitizer by adding -fsanitize=address -fno-omit-frame-pointer flags" OFF)
option(BUILD_UVW_LIBS "Prepare targets for static and shared libraries rather than for a header-only library." OFF)

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

function(fetch_libuv)
    include(FetchContent)

    FetchContent_Declare(
        libuv
        GIT_REPOSITORY https://github.com/libuv/libuv.git
        GIT_TAG v1.34.1
        GIT_SHALLOW 1
    )

    FetchContent_GetProperties(libuv)

    if(NOT libuv_POPULATED)
        FetchContent_Populate(libuv)
        add_subdirectory(${libuv_SOURCE_DIR} ${libuv_BINARY_DIR})
    endif()

    add_library(uv::uv-static ALIAS uv_a)
    set_target_properties(uv_a PROPERTIES POSITION_INDEPENDENT_CODE 1)
    add_library(uv::uv-shared ALIAS uv)
    set_target_properties(uv PROPERTIES POSITION_INDEPENDENT_CODE 1)
endfunction()

#
# Add uvw target
#

include(GNUInstallDirs)

if(BUILD_UVW_LIBS)
    fetch_libuv()
    add_subdirectory(src)
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
        target_link_libraries(uvw INTERFACE $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer>)
    endif()

    if(HAS_LIBCPP)
        target_compile_options(uvw BEFORE INTERFACE -stdlib=libc++)
    endif()
endif()

### Testing

option(BUILD_TESTING "Enable testing with ctest." OFF)

if(BUILD_TESTING)
    option(FIND_GTEST_PACKAGE "Enable finding gtest package." OFF)

    fetch_libuv()
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
