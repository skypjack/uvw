#
# Tests configuration
#

include(FetchContent)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(FIND_GTEST_PACKAGE)
    find_package(GTest REQUIRED)
else()
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
        GIT_SHALLOW 1
    )

    FetchContent_GetProperties(googletest)

    if(NOT googletest_POPULATED)
        FetchContent_Populate(googletest)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()

    add_library(GTest::Main ALIAS gtest_main)

    target_compile_features(gtest PUBLIC cxx_std_17)
    target_compile_features(gtest_main PUBLIC cxx_std_17)
    target_compile_features(gmock PUBLIC cxx_std_17)
    target_compile_features(gmock_main PUBLIC cxx_std_17)

    if(CMAKE_SYSTEM_NAME MATCHES OpenBSD)
        target_compile_options(gtest PRIVATE -Wno-error)
    endif()
endif()

if(WIN32)
    set(WINSOCK2 ws2_32)
endif()

function(ADD_UVW_TEST TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})

    target_link_libraries(
        ${TEST_NAME}
        PRIVATE
            $<$<TARGET_EXISTS:uvw::uvw>:uvw::uvw>
            $<$<TARGET_EXISTS:uv::uv-static>:uv::uv-static>
            $<$<TARGET_EXISTS:uvw::uvw-static>:uvw::uvw-static>            
            $<$<TARGET_EXISTS:uv::uv-shared>:uv::uv-shared>
            $<$<TARGET_EXISTS:uvw::uvw-shared>:uvw::uvw-shared>
            GTest::Main
            Threads::Threads
            ${LIBRT}
            ${WINSOCK2}
        )

    target_compile_options(
        ${TEST_NAME}
        PRIVATE
            $<$<NOT:$<PLATFORM_ID:Windows>>:-Wall>
            $<$<PLATFORM_ID:Windows>:/EHsc>
    )

    target_compile_definitions(${TEST_NAME} PRIVATE $<$<NOT:$<TARGET_EXISTS:uvw::uvw>>:UVW_AS_LIB>)
    add_test(NAME uvw_${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)
endfunction()

function(ADD_UVW_DIR_TEST TEST_NAME TEST_SOURCE)
    ADD_UVW_TEST(${TEST_NAME} ${TEST_SOURCE})
    string(TOUPPER "${TEST_NAME}" TEST_NAME_UPPER)
    set(TEST_DIR_VAR "TARGET_${TEST_NAME_UPPER}_DIR")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}_data)
    target_compile_definitions(${TEST_NAME} PRIVATE ${TEST_DIR_VAR}="${TEST_NAME}_data")
endfunction()

function(ADD_UVW_LIB_TEST TEST_NAME TEST_SOURCE)
    ADD_UVW_TEST(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE ${CMAKE_DL_LIBS})
    target_compile_definitions(${TEST_NAME} PRIVATE TARGET_LIB_SO="$<TARGET_FILE:fake>")
endfunction()

add_library(fake SHARED fake.cpp)

# List of available targets

option(BUILD_DNS_TEST "Build DNS test." OFF)

ADD_UVW_TEST(main main.cpp)
ADD_UVW_TEST(async uvw/async.cpp)
ADD_UVW_TEST(check uvw/check.cpp)
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

if(NOT CMAKE_SYSTEM_NAME MATCHES OpenBSD)
    ADD_UVW_DIR_TEST(file_req_sendfile uvw/file_req_sendfile.cpp)
endif()

if(BUILD_DNS_TEST)
    ADD_UVW_TEST(dns uvw/dns.cpp)
endif()
