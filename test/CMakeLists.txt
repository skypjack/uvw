#
# Tests configuration
#

set(UVW_SRC_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_SRC_DIR})

# List of available targets

set(TARGET_FOOBAR foobar)

# Test TARGET_FOOBAR_SRC_DIR

set(
    TARGET_FOOBAR_SOURCES
    main.cpp
)

add_executable(
    ${TARGET_FOOBAR}
    ${TARGET_FOOBAR_SOURCES}
)

target_include_directories(
    ${TARGET_FOOBAR}
    PRIVATE ${UVW_SRC_DIR}
    PRIVATE ${GOOGLETEST_INCLUDE_DIRS}
    PRIVATE ${UV_INCLUDE_DIRS}
)

target_link_libraries(
    ${TARGET_FOOBAR}
    PRIVATE ${CMAKE_THREAD_LIBS_INIT}
    PRIVATE ${GOOGLETEST_LIBRARIES}
    PRIVATE ${UV_LIBRARIES}
)

add_test(
    NAME ${TARGET_FOOBAR}
    COMMAND ${TARGET_FOOBAR}
)
