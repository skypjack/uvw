#
# uvw
#

cmake_minimum_required(VERSION 3.2)

#
# Building in-tree is not allowed (we take care of your craziness).
#

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

#
# Project configuration
#

project(uvw VERSION 1.11.2)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(PROJECT_AUTHOR "Michele Caini")
set(PROJECT_AUTHOR_EMAIL "michele.caini@gmail.com")

message("*")
message("* ${PROJECT_NAME} v${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
message("* Copyright (c) 2016-2018 ${PROJECT_AUTHOR} <${PROJECT_AUTHOR_EMAIL}>")
message("*")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -DRELEASE")
endif()

#
# CMake configuration
#

set(PROJECT_CMAKE_IN ${uvw_SOURCE_DIR}/cmake/in)
set(PROJECT_DEPS_DIR ${uvw_SOURCE_DIR}/deps)
set(PROJECT_SRC_DIR ${uvw_SOURCE_DIR}/src)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

#
# Referenced packages
#

set(THREADS_PREFER_PTHREAD_FLAG ON)

include(FindThreads)
find_package(Doxygen 1.8)

#
# Referenced directories and targets
#

if(DOXYGEN_FOUND)
    add_subdirectory(docs)
endif()

if(BUILD_TESTING)
    enable_testing()
    set(GOOGLETEST_DEPS_DIR ${PROJECT_DEPS_DIR}/googletest)
    set(LIBUV_DEPS_DIR ${PROJECT_DEPS_DIR}/libuv)

    configure_file(${PROJECT_CMAKE_IN}/deps.in ${PROJECT_DEPS_DIR}/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${PROJECT_DEPS_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${PROJECT_DEPS_DIR})

    # gtest, gtest_main, gmock and gmock_main targets are available from now on
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(${GOOGLETEST_DEPS_DIR})

    # uv and uv_a targets are available from now on
    add_subdirectory(${LIBUV_DEPS_DIR})
    include_directories(${LIBUV_DEPS_DIR}/include)

    add_subdirectory(test)
endif()

#
# Keep your stuff and go further away, foolish.
#

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR ${PROJECT_AUTHOR})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_NAME})
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION})
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME}-src)
set(CPACK_RESOURCE_FILE_LICENSE ${uvw_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${uvw_SOURCE_DIR}/README.md)
set(CPACK_GENERATOR TGZ)
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_PACKAGING_INSTALL_DIRECTORY "uvw-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

include(CPack)

#
# Install headers
#

INCLUDE(GNUInstallDirs)

IF(NOT DEFINED INCLUDE_INSTALL_DIR)
    SET(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
ENDIF()

file(GLOB HEADERS src/uvw/*.hpp)

install(
    FILES ${HEADERS}
    COMPONENT ${PROJECT_NAME}
    DESTINATION ${INCLUDE_INSTALL_DIR}/uvw
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

install(
    FILES src/uvw.hpp
    COMPONENT ${PROJECT_NAME}
    DESTINATION ${INCLUDE_INSTALL_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

#
# AOB
#

add_custom_target(
    uvw_aob
    SOURCES
        appveyor.yml
        AUTHORS
        LICENSE
        README.md
        .travis.yml
)
