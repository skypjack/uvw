#
# Tests configuration
#

set(UVW_SRC_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_SRC_DIR})

set(
    COMMON_LINK_LIBS
    gtest
    gtest_main
    ${libuv_static}
    Threads::Threads
)

if(${LIBRT_FOUND})
    list(APPEND COMMON_LINK_LIBS ${LIBRT_LIBRARIES})
endif(${LIBRT_FOUND})

if(WIN32)
    list(APPEND COMMON_LINK_LIBS ws2_32)
endif(WIN32)

add_library(odr OBJECT odr.cpp)
target_include_directories(odr PRIVATE ${UVW_SRC_DIR})

# List of available targets

set(TARGET_MAIN main)
set(TARGET_ASYNC async)
set(TARGET_CHECK check)
set(TARGET_DNS dns)
set(TARGET_EMITTER emitter)
set(TARGET_FILE_REQ file_req)
set(TARGET_FS_EVENT fs_event)
set(TARGET_FS_POLL fs_poll)
set(TARGET_FS_REQ fs_req)
set(TARGET_HANDLE handle)
set(TARGET_IDLE idle)
set(TARGET_LIB lib)
set(TARGET_LOOP loop)
set(TARGET_PIPE pipe)
set(TARGET_POLL poll)
set(TARGET_PREPARE prepare)
set(TARGET_PROCESS process)
set(TARGET_REQUEST request)
set(TARGET_RESOURCE resource)
set(TARGET_SIGNAL signal)
set(TARGET_STREAM stream)
set(TARGET_TCP tcp)
set(TARGET_THREAD thread)
set(TARGET_TIMER timer)
set(TARGET_TTY tty)
set(TARGET_UDP udp)
set(TARGET_UTIL util)
set(TARGET_WORK work)

# Test TARGET_MAIN

set(TARGET_MAIN_SOURCES $<TARGET_OBJECTS:odr> main.cpp)
add_executable(${TARGET_MAIN} ${TARGET_MAIN_SOURCES})
target_include_directories(${TARGET_MAIN} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_MAIN} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_MAIN} COMMAND ${TARGET_MAIN})

# Test TARGET_ASYNC

set(TARGET_ASYNC_SOURCES $<TARGET_OBJECTS:odr> uvw/async.cpp)
add_executable(${TARGET_ASYNC} ${TARGET_ASYNC_SOURCES})
target_include_directories(${TARGET_ASYNC} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_ASYNC} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_ASYNC} COMMAND ${TARGET_ASYNC})

# Test TARGET_CHECK

set(TARGET_CHECK_SOURCES $<TARGET_OBJECTS:odr> uvw/check.cpp)
add_executable(${TARGET_CHECK} ${TARGET_CHECK_SOURCES})
target_include_directories(${TARGET_CHECK} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_CHECK} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_CHECK} COMMAND ${TARGET_CHECK})

# Test TARGET_DNS

set(TARGET_DNS_SOURCES $<TARGET_OBJECTS:odr> uvw/dns.cpp)
add_executable(${TARGET_DNS} ${TARGET_DNS_SOURCES})
target_include_directories(${TARGET_DNS} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_DNS} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_DNS} COMMAND ${TARGET_DNS})

# Test TARGET_EMITTER

set(TARGET_EMITTER_SOURCES $<TARGET_OBJECTS:odr> uvw/emitter.cpp)
add_executable(${TARGET_EMITTER} ${TARGET_EMITTER_SOURCES})
target_include_directories(${TARGET_EMITTER} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_EMITTER} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_EMITTER} COMMAND ${TARGET_EMITTER})

# Test TARGET_FILE_REQ

set(TARGET_FILE_REQ_SOURCES $<TARGET_OBJECTS:odr> uvw/file_req.cpp)
set(TARGET_FILE_REQ_DIR ${CMAKE_CURRENT_BINARY_DIR}/file_req)
file(MAKE_DIRECTORY ${TARGET_FILE_REQ_DIR})
add_executable(${TARGET_FILE_REQ} ${TARGET_FILE_REQ_SOURCES})
target_include_directories(${TARGET_FILE_REQ} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_FILE_REQ} PRIVATE ${COMMON_LINK_LIBS})
target_compile_definitions(${TARGET_FILE_REQ} PRIVATE TARGET_FILE_REQ_DIR="${TARGET_FILE_REQ_DIR}")
add_test(NAME ${TARGET_FILE_REQ} COMMAND ${TARGET_FILE_REQ})

# Test TARGET_FS_EVENT

set(TARGET_FS_EVENT_SOURCES $<TARGET_OBJECTS:odr> uvw/fs_event.cpp)
set(TARGET_FS_EVENT_DIR ${CMAKE_CURRENT_BINARY_DIR}/fs_event)
file(MAKE_DIRECTORY ${TARGET_FS_EVENT_DIR})
add_executable(${TARGET_FS_EVENT} ${TARGET_FS_EVENT_SOURCES})
target_include_directories(${TARGET_FS_EVENT} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_FS_EVENT} PRIVATE ${COMMON_LINK_LIBS})
target_compile_definitions(${TARGET_FS_EVENT} PRIVATE TARGET_FS_EVENT_DIR="${TARGET_FS_EVENT_DIR}")
add_test(NAME ${TARGET_FS_EVENT} COMMAND ${TARGET_FS_EVENT})

# Test TARGET_FS_POLL

set(TARGET_FS_POLL_SOURCES $<TARGET_OBJECTS:odr> uvw/fs_poll.cpp)
set(TARGET_FS_POLL_DIR ${CMAKE_CURRENT_BINARY_DIR}/fs_poll)
file(MAKE_DIRECTORY ${TARGET_FS_POLL_DIR})
add_executable(${TARGET_FS_POLL} ${TARGET_FS_POLL_SOURCES})
target_include_directories(${TARGET_FS_POLL} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_FS_POLL} PRIVATE ${COMMON_LINK_LIBS})
target_compile_definitions(${TARGET_FS_POLL} PRIVATE TARGET_FS_POLL_DIR="${TARGET_FS_POLL_DIR}")
add_test(NAME ${TARGET_FS_POLL} COMMAND ${TARGET_FS_POLL})

# Test TARGET_FS_REQ

set(TARGET_FS_REQ_SOURCES $<TARGET_OBJECTS:odr> uvw/fs_req.cpp)
set(TARGET_FS_REQ_DIR ${CMAKE_CURRENT_BINARY_DIR}/fs_req)
file(MAKE_DIRECTORY ${TARGET_FS_REQ_DIR})
add_executable(${TARGET_FS_REQ} ${TARGET_FS_REQ_SOURCES})
target_include_directories(${TARGET_FS_REQ} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_FS_REQ} PRIVATE ${COMMON_LINK_LIBS})
target_compile_definitions(${TARGET_FS_REQ} PRIVATE TARGET_FS_REQ_DIR="${TARGET_FS_REQ_DIR}")
add_test(NAME ${TARGET_FS_REQ} COMMAND ${TARGET_FS_REQ})

# Test TARGET_HANDLE

set(TARGET_HANDLE_SOURCES $<TARGET_OBJECTS:odr> uvw/handle.cpp)
add_executable(${TARGET_HANDLE} ${TARGET_HANDLE_SOURCES})
target_include_directories(${TARGET_HANDLE} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_HANDLE} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_HANDLE} COMMAND ${TARGET_HANDLE})

# Test TARGET_IDLE

set(TARGET_IDLE_SOURCES $<TARGET_OBJECTS:odr> uvw/idle.cpp)
add_executable(${TARGET_IDLE} ${TARGET_IDLE_SOURCES})
target_include_directories(${TARGET_IDLE} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_IDLE} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_IDLE} COMMAND ${TARGET_IDLE})

# Test TARGET_LIB

set(TARGET_LIB_SOURCES $<TARGET_OBJECTS:odr> uvw/lib.cpp)
add_executable(${TARGET_LIB} ${TARGET_LIB_SOURCES})
target_include_directories(${TARGET_LIB} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_LIB} PRIVATE ${COMMON_LINK_LIBS} ${CMAKE_DL_LIBS})
target_compile_definitions(${TARGET_LIB} PRIVATE TARGET_LIB_SO="${libuv_shared}")
add_test(NAME ${TARGET_LIB} COMMAND ${TARGET_LIB})

# Test TARGET_LOOP

set(TARGET_LOOP_SOURCES $<TARGET_OBJECTS:odr> uvw/loop.cpp)
add_executable(${TARGET_LOOP} ${TARGET_LOOP_SOURCES})
target_include_directories(${TARGET_LOOP} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_LOOP} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_LOOP} COMMAND ${TARGET_LOOP})

# Test TARGET_PIPE

set(TARGET_PIPE_SOURCES $<TARGET_OBJECTS:odr> uvw/pipe.cpp)
add_executable(${TARGET_PIPE} ${TARGET_PIPE_SOURCES})
target_include_directories(${TARGET_PIPE} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_PIPE} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_PIPE} COMMAND ${TARGET_PIPE})

# Test TARGET_POLL

set(TARGET_POLL_SOURCES $<TARGET_OBJECTS:odr> uvw/poll.cpp)
add_executable(${TARGET_POLL} ${TARGET_POLL_SOURCES})
target_include_directories(${TARGET_POLL} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_POLL} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_POLL} COMMAND ${TARGET_POLL})

# Test TARGET_PREPARE

set(TARGET_PREPARE_SOURCES $<TARGET_OBJECTS:odr> uvw/prepare.cpp)
add_executable(${TARGET_PREPARE} ${TARGET_PREPARE_SOURCES})
target_include_directories(${TARGET_PREPARE} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_PREPARE} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_PREPARE} COMMAND ${TARGET_PREPARE})

# Test TARGET_PROCESS

set(TARGET_PROCESS_SOURCES $<TARGET_OBJECTS:odr> uvw/process.cpp)
add_executable(${TARGET_PROCESS} ${TARGET_PROCESS_SOURCES})
target_include_directories(${TARGET_PROCESS} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_PROCESS} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_PROCESS} COMMAND ${TARGET_PROCESS})

# Test TARGET_REQUEST

set(TARGET_REQUEST_SOURCES $<TARGET_OBJECTS:odr> uvw/request.cpp)
add_executable(${TARGET_REQUEST} ${TARGET_REQUEST_SOURCES})
target_include_directories(${TARGET_REQUEST} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_REQUEST} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_REQUEST} COMMAND ${TARGET_REQUEST})

# Test TARGET_RESOURCE
set(TARGET_RESOURCE_SOURCES $<TARGET_OBJECTS:odr> uvw/resource.cpp)
add_executable(${TARGET_RESOURCE} ${TARGET_RESOURCE_SOURCES})
target_include_directories(${TARGET_RESOURCE} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_RESOURCE} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_RESOURCE} COMMAND ${TARGET_RESOURCE})

# Test TARGET_SIGNAL

set(TARGET_SIGNAL_SOURCES $<TARGET_OBJECTS:odr> uvw/signal.cpp)
add_executable(${TARGET_SIGNAL} ${TARGET_SIGNAL_SOURCES})
target_include_directories(${TARGET_SIGNAL} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_SIGNAL} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_SIGNAL} COMMAND ${TARGET_SIGNAL})

# Test TARGET_STREAM

set(TARGET_STREAM_SOURCES $<TARGET_OBJECTS:odr> uvw/stream.cpp)
add_executable(${TARGET_STREAM} ${TARGET_STREAM_SOURCES})
target_include_directories(${TARGET_STREAM} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_STREAM} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_STREAM} COMMAND ${TARGET_STREAM})

# Test TARGET_TCP

set(TARGET_TCP_SOURCES $<TARGET_OBJECTS:odr> uvw/tcp.cpp)
add_executable(${TARGET_TCP} ${TARGET_TCP_SOURCES})
target_include_directories(${TARGET_TCP} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_TCP} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_TCP} COMMAND ${TARGET_TCP})

# Test TARGET_THREAD

set(TARGET_THREAD_SOURCES $<TARGET_OBJECTS:odr> uvw/thread.cpp)
add_executable(${TARGET_THREAD} ${TARGET_THREAD_SOURCES})
target_include_directories(${TARGET_THREAD} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_THREAD} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_THREAD} COMMAND ${TARGET_THREAD})

# Test TARGET_TIMER

set(TARGET_TIMER_SOURCES $<TARGET_OBJECTS:odr> uvw/timer.cpp)
add_executable(${TARGET_TIMER} ${TARGET_TIMER_SOURCES})
target_include_directories(${TARGET_TIMER} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_TIMER} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_TIMER} COMMAND ${TARGET_TIMER})

# Test TARGET_TTY

set(TARGET_TTY_SOURCES $<TARGET_OBJECTS:odr> uvw/tty.cpp)
add_executable(${TARGET_TTY} ${TARGET_TTY_SOURCES})
target_include_directories(${TARGET_TTY} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_TTY} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_TTY} COMMAND ${TARGET_TTY})

# Test TARGET_UDP

set(TARGET_UDP_SOURCES $<TARGET_OBJECTS:odr> uvw/udp.cpp)
add_executable(${TARGET_UDP} ${TARGET_UDP_SOURCES})
target_include_directories(${TARGET_UDP} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_UDP} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_UDP} COMMAND ${TARGET_UDP})

# Test TARGET_UTIL

set(TARGET_UTIL_SOURCES $<TARGET_OBJECTS:odr> uvw/util.cpp)
add_executable(${TARGET_UTIL} ${TARGET_UTIL_SOURCES})
target_include_directories(${TARGET_UTIL} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_UTIL} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_UTIL} COMMAND ${TARGET_UTIL})

# Test TARGET_WORK

set(TARGET_WORK_SOURCES $<TARGET_OBJECTS:odr> uvw/work.cpp)
add_executable(${TARGET_WORK} ${TARGET_WORK_SOURCES})
target_include_directories(${TARGET_WORK} PRIVATE ${UVW_SRC_DIR})
target_link_libraries(${TARGET_WORK} PRIVATE ${COMMON_LINK_LIBS})
add_test(NAME ${TARGET_WORK} COMMAND ${TARGET_WORK})
