#
# Tests configuration
#

set(UVW_SRC_DIR ${PROJECT_SRC_DIR})

if(WIN32)
    set(WINSOCK2 ws2_32)
elseif(NOT APPLE)
    find_library(LIBRT rt)
endif()

set(
    COMMON_LINK_LIBS
    gtest_main
    ${LIBRT}
    ${libuv_static}
    Threads::Threads
    ${WINSOCK2}
)

add_library(odr OBJECT odr.cpp)
target_include_directories(odr PRIVATE ${UVW_SRC_DIR})

macro(ADD_UVW_TEST TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} $<TARGET_OBJECTS:odr> ${TEST_SOURCE})
    target_include_directories(${TEST_NAME} PRIVATE ${UVW_SRC_DIR})
    target_link_libraries(${TEST_NAME} PRIVATE ${COMMON_LINK_LIBS})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro()

macro(ADD_UVW_DIR_TEST TEST_NAME TEST_SOURCE)
    ADD_UVW_TEST(${TEST_NAME} ${TEST_SOURCE})
    string(TOUPPER "${TEST_NAME}" TEST_NAME_UPPER)
    set(TEST_DIR_VAR "TARGET_${TEST_NAME_UPPER}_DIR")
    set(TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME})
    file(MAKE_DIRECTORY ${TEST_DIR})
    target_compile_definitions(${TEST_NAME} PRIVATE ${TEST_DIR_VAR}="${TEST_DIR}")
endmacro()

macro(ADD_UVW_LIB_TEST TEST_NAME TEST_SOURCE)
    ADD_UVW_TEST(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE ${CMAKE_DL_LIBS})
    target_compile_definitions(${TEST_NAME} PRIVATE TARGET_LIB_SO="${libuv_shared}")
endmacro()

# List of available targets

ADD_UVW_TEST(main main.cpp)
ADD_UVW_TEST(async uvw/async.cpp)
ADD_UVW_TEST(check uvw/check.cpp)
ADD_UVW_TEST(dns uvw/dns.cpp)
ADD_UVW_TEST(emitter uvw/emitter.cpp)
ADD_UVW_DIR_TEST(file_req uvw/file_req.cpp)
ADD_UVW_DIR_TEST(fs_event uvw/fs_event.cpp)
ADD_UVW_DIR_TEST(fs_poll uvw/fs_poll.cpp)
ADD_UVW_DIR_TEST(fs_req uvw/fs_req.cpp)
ADD_UVW_TEST(handle uvw/handle.cpp)
ADD_UVW_TEST(idle uvw/idle.cpp)
ADD_UVW_LIB_TEST(lib uvw/lib.cpp)
ADD_UVW_TEST(loop uvw/loop.cpp)
ADD_UVW_DIR_TEST(pipe uvw/pipe.cpp)
ADD_UVW_TEST(poll uvw/poll.cpp)
ADD_UVW_TEST(prepare uvw/prepare.cpp)
ADD_UVW_TEST(process uvw/process.cpp)
ADD_UVW_TEST(request uvw/request.cpp)
ADD_UVW_TEST(resource uvw/resource.cpp)
ADD_UVW_TEST(signal uvw/signal.cpp)
ADD_UVW_TEST(stream uvw/stream.cpp)
ADD_UVW_TEST(tcp uvw/tcp.cpp)
ADD_UVW_TEST(thread uvw/thread.cpp)
ADD_UVW_TEST(timer uvw/timer.cpp)
ADD_UVW_TEST(tty uvw/tty.cpp)
ADD_UVW_TEST(udp uvw/udp.cpp)
ADD_UVW_TEST(underlying_type uvw/underlying_type.cpp)
ADD_UVW_TEST(util uvw/util.cpp)
ADD_UVW_TEST(work uvw/work.cpp)
