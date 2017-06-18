#
# uvw
#

#
# Building in-tree is not allowed (we take care of your craziness).
#

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

#
# Project configuration
#

project(uvw)
cmake_minimum_required(VERSION 3.4)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

set(PROJECT_NAME "uvw")
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
set(SETTINGS_ORGANIZATION "Michele Caini")
set(SETTINGS_APPLICATION ${PROJECT_NAME})
set(PROJECT_AUTHOR "Michele Caini")
set(PROJECT_YEAR_COPYRIGHT "2017")
set(PROJECT_AUTHOR_EMAIL "michele.caini@gmail.com")

set(PROJECT_BUILD_MESSAGE ${PROJECT_NAME} " v" ${PROJECT_VERSION} " (" ${CMAKE_BUILD_TYPE} ")")
set(COPYRIGHT_BUILD_MESSAGE "Copyright (c) " ${PROJECT_YEAR_COPYRIGHT} " " ${PROJECT_AUTHOR}  " <" ${PROJECT_AUTHOR_EMAIL} ">")

message("*")
message("* " ${PROJECT_BUILD_MESSAGE})
message("* " ${COPYRIGHT_BUILD_MESSAGE})
message("*")

if(APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined,dynamic_lookup")
else(APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif(APPLE)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif(NOT WIN32)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -DRELEASE")

add_definitions(
    -DPROJECT_NAME=${PROJECT_NAME}
    -DPROJECT_VERSION=${PROJECT_VERSION}
)

#
# CMake configuration
#

set(PROJECT_CMAKE_MODULES cmake/modules)
set(PROJECT_CMAKE_IN cmake/in)
set(PROJECT_BUILD_DIR build)
set(PROJECT_DEPS_DIR deps)
set(PROJECT_DOCS_DIR docs)
set(PROJECT_SRC_DIR src)
set(PROJECT_TEST_DIR test)

set(PROJECT_RUNTIME_OUTPUT_DIRECTORY bin)

set(
    CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/${PROJECT_CMAKE_MODULES}
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_RUNTIME_OUTPUT_DIRECTORY})

#
# Enable test support using ctest
#

enable_testing()

#
# Referenced packages
#

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(Doxygen 1.8)
find_package(LibRt)

#
# Download and unpack dependencies
#

set(GOOGLETEST_DEPS_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_DEPS_DIR}/googletest)
set(LIBUV_DEPS_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_DEPS_DIR}/libuv)

# gtest, gtest_main, gmock and gmock_main targets are available from now on
configure_file(${PROJECT_CMAKE_IN}/googletest.in ${GOOGLETEST_DEPS_DIR}/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${GOOGLETEST_DEPS_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${GOOGLETEST_DEPS_DIR})
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(${GOOGLETEST_DEPS_DIR}/src ${GOOGLETEST_DEPS_DIR}/build)

configure_file(${PROJECT_CMAKE_IN}/libuv.in ${LIBUV_DEPS_DIR}/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${LIBUV_DEPS_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${LIBUV_DEPS_DIR})
include_directories(${LIBUV_DEPS_DIR}/src/include)
find_library(libuv_static NAMES libuv.a libuv PATHS ${LIBUV_DEPS_DIR}/src PATH_SUFFIXES .libs Release NO_DEFAULT_PATH)
find_library(libuv_shared NAMES uv libuv PATHS ${LIBUV_DEPS_DIR}/src PATH_SUFFIXES .libs Release NO_DEFAULT_PATH)

#
# Referenced directories and targets
#

if(${DOXYGEN_FOUND})
    add_subdirectory(${PROJECT_DOCS_DIR})
endif(${DOXYGEN_FOUND})

add_subdirectory(${PROJECT_TEST_DIR})

#
# Keep your stuff and go further away, foolish.
#

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR ${PROJECT_AUTHOR})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "uvw")
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION})
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME}-src)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/README.md)
set(CPACK_GENERATOR TGZ)
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_PACKAGING_INSTALL_DIRECTORY "uvw-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")

include(CPack)

#
# I use QtCreator and I need the lines below, so do not ask. :-)
#

file(
    GLOB_RECURSE PROJECT_FILES FOLLOW_SYMLINKS
    *.txt *.c *.cpp *.hpp *.h *.in *.dox *.cmake *.sh *.md *.yml *.bat
    AUTHORS LICENSE TODO
)

add_custom_target(
    QTCREATOR_FALLBACK ALL
    COMMENT "Feel free to ignore this target, but please do not remove it."
    SOURCES ${PROJECT_FILES}
)
