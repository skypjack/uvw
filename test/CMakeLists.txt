#
# Tests configuration
#

include(FetchContent)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(UVW_FIND_GTEST_PACKAGE)
    find_package(GTest REQUIRED)
else()
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
        GIT_SHALLOW 1
    )

    FetchContent_GetProperties(googletest)

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_library(GTest::Main ALIAS gtest_main)

    target_compile_features(gtest PUBLIC cxx_std_17)
    set_target_properties(gtest PROPERTIES CXX_CLANG_TIDY "")

    target_compile_features(gtest_main PUBLIC cxx_std_17)
    set_target_properties(gtest_main PROPERTIES CXX_CLANG_TIDY "")

    target_compile_features(gmock PUBLIC cxx_std_17)
    set_target_properties(gmock PROPERTIES CXX_CLANG_TIDY "")

    target_compile_features(gmock_main PUBLIC cxx_std_17)
    set_target_properties(gmock_main PROPERTIES CXX_CLANG_TIDY "")

    if(CMAKE_SYSTEM_NAME MATCHES OpenBSD)
        target_compile_options(gtest PRIVATE -Wno-error)
    endif()
endif()

if(WIN32)
    set(WINSOCK2 ws2_32)
endif()

function(UVW_ADD_TEST TEST_NAME TEST_SOURCE)
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

function(UVW_ADD_DIR_TEST TEST_NAME TEST_SOURCE)
    UVW_ADD_TEST(${TEST_NAME} ${TEST_SOURCE})
    string(TOUPPER "${TEST_NAME}" TEST_NAME_UPPER)
    set(TEST_DIR_VAR "TARGET_${TEST_NAME_UPPER}_DIR")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}_data)
    target_compile_definitions(${TEST_NAME} PRIVATE ${TEST_DIR_VAR}="${TEST_NAME}_data")
endfunction()

function(UVW_ADD_LIB_TEST TEST_NAME TEST_SOURCE)
    UVW_ADD_TEST(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE ${CMAKE_DL_LIBS})
    target_compile_definitions(${TEST_NAME} PRIVATE TARGET_LIB_SO="$<TARGET_FILE:fake>")
endfunction()

add_library(fake SHARED fake.cpp)

# List of available targets

option(UVW_BUILD_DNS_TEST "Build DNS test." OFF)

UVW_ADD_TEST(main main.cpp)
UVW_ADD_TEST(async uvw/async.cpp)
UVW_ADD_TEST(check uvw/check.cpp)
UVW_ADD_TEST(emitter uvw/emitter.cpp)
UVW_ADD_DIR_TEST(file_req uvw/file_req.cpp)
UVW_ADD_DIR_TEST(fs_event uvw/fs_event.cpp)
UVW_ADD_DIR_TEST(fs_req uvw/fs_req.cpp)
UVW_ADD_TEST(handle uvw/handle.cpp)
UVW_ADD_TEST(idle uvw/idle.cpp)
UVW_ADD_LIB_TEST(lib uvw/lib.cpp)
UVW_ADD_TEST(loop uvw/loop.cpp)
UVW_ADD_DIR_TEST(pipe uvw/pipe.cpp)
UVW_ADD_TEST(prepare uvw/prepare.cpp)
UVW_ADD_TEST(process uvw/process.cpp)
UVW_ADD_TEST(request uvw/request.cpp)
UVW_ADD_TEST(resource uvw/resource.cpp)
UVW_ADD_TEST(signal uvw/signal.cpp)
UVW_ADD_TEST(stream uvw/stream.cpp)
UVW_ADD_TEST(tcp uvw/tcp.cpp)
UVW_ADD_TEST(thread uvw/thread.cpp)
UVW_ADD_TEST(timer uvw/timer.cpp)
UVW_ADD_TEST(tty uvw/tty.cpp)
UVW_ADD_TEST(udp uvw/udp.cpp)
UVW_ADD_TEST(uv_type uvw/uv_type.cpp)
UVW_ADD_TEST(util uvw/util.cpp)
UVW_ADD_TEST(work uvw/work.cpp)

if(NOT CMAKE_SYSTEM_NAME MATCHES OpenBSD)
    UVW_ADD_DIR_TEST(file_req_sendfile uvw/file_req_sendfile.cpp)
endif()

if(UVW_BUILD_DNS_TEST)
    UVW_ADD_TEST(dns uvw/dns.cpp)
endif()
