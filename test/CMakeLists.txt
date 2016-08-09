#
# Tests configuration
#

set(UVW_SRC_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_SRC_DIR})

set(
    COMMON_INCLUDE_DIRS
    ${UVW_SRC_DIR}
    ${GOOGLETEST_INCLUDE_DIRS}
    ${UV_INCLUDE_DIRS}
)

set(
    COMMON_LINK_LIBS
    ${CMAKE_THREAD_LIBS_INIT}
    ${GOOGLETEST_LIBRARIES}
    ${UV_LIBRARIES}
)

# List of available targets

set(TARGET_MAIN main)
set(TARGET_ASYNC async)
set(TARGET_CHECK check)
set(TARGET_IDLE idle)
set(TARGET_LOOP loop)
set(TARGET_PREPARE prepare)
set(TARGET_WORK work)

# Test TARGET_MAIN

set(TARGET_MAIN_SOURCES main.cpp)
add_executable(${TARGET_MAIN} ${TARGET_MAIN_SOURCES})
target_include_directories(${TARGET_MAIN} PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(${TARGET_MAIN} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_MAIN} COMMAND ${TARGET_MAIN})

# Test TARGET_ASYNC

set(TARGET_ASYNC_SOURCES uvw/async.cpp)
add_executable(${TARGET_ASYNC} ${TARGET_ASYNC_SOURCES})
target_include_directories(${TARGET_ASYNC} PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(${TARGET_ASYNC} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_ASYNC} COMMAND ${TARGET_ASYNC})

# Test TARGET_CHECK

set(TARGET_CHECK_SOURCES uvw/check.cpp)
add_executable(${TARGET_CHECK} ${TARGET_CHECK_SOURCES})
target_include_directories(${TARGET_CHECK} PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(${TARGET_CHECK} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_CHECK} COMMAND ${TARGET_CHECK})

# Test TARGET_IDLE

set(TARGET_IDLE_SOURCES uvw/idle.cpp)
add_executable(${TARGET_IDLE} ${TARGET_IDLE_SOURCES})
target_include_directories(${TARGET_IDLE} PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(${TARGET_IDLE} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_IDLE} COMMAND ${TARGET_IDLE})

# Test TARGET_LOOP

set(TARGET_LOOP_SOURCES uvw/loop.cpp)
add_executable(${TARGET_LOOP} ${TARGET_LOOP_SOURCES})
target_include_directories(${TARGET_LOOP} PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(${TARGET_LOOP} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_LOOP} COMMAND ${TARGET_LOOP})

# Test TARGET_PREPARE

set(TARGET_PREPARE_SOURCES uvw/prepare.cpp)
add_executable(${TARGET_PREPARE} ${TARGET_PREPARE_SOURCES})
target_include_directories(${TARGET_PREPARE} PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(${TARGET_PREPARE} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_PREPARE} COMMAND ${TARGET_PREPARE})

# Test TARGET_WORK

set(TARGET_WORK_SOURCES uvw/work.cpp)
add_executable(${TARGET_WORK} ${TARGET_WORK_SOURCES})
target_include_directories(${TARGET_WORK} PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(${TARGET_WORK} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_WORK} COMMAND ${TARGET_WORK})
