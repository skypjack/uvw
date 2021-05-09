#
# Setup libraries
#

function(add_uvw_library LIB_NAME)
    target_sources(
        ${LIB_NAME}
        PRIVATE
            uvw/async.cpp
            uvw/check.cpp
            uvw/dns.cpp
            uvw/emitter.cpp
            uvw/fs.cpp
            uvw/fs_event.cpp
            uvw/fs_poll.cpp
            uvw/idle.cpp
            uvw/lib.cpp
            uvw/loop.cpp
            uvw/pipe.cpp
            uvw/poll.cpp
            uvw/prepare.cpp
            uvw/process.cpp
            uvw/signal.cpp
            uvw/stream.cpp
            uvw/tcp.cpp
            uvw/thread.cpp
            uvw/timer.cpp
            uvw/tty.cpp
            uvw/udp.cpp
            uvw/util.cpp
            uvw/work.cpp
    )

    set_target_properties(${LIB_NAME} PROPERTIES POSITION_INDEPENDENT_CODE 1)
    target_compile_definitions(${LIB_NAME} PUBLIC UVW_AS_LIB)
    target_compile_features(${LIB_NAME} PUBLIC cxx_std_17)

    target_include_directories(
        ${LIB_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    if(USE_ASAN)
        target_compile_options(${LIB_NAME} PUBLIC $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer>)
        target_link_libraries(${LIB_NAME} PUBLIC $<$<CONFIG:Debug>:-fsanitize=address>)
    endif()

    if(USE_UBSAN)
        target_compile_options(${LIB_NAME} PUBLIC $<$<CONFIG:Debug>:-fsanitize=undefined -fno-sanitize-recover=all -fno-omit-frame-pointer>)
        target_link_libraries(${LIB_NAME} PUBLIC $<$<CONFIG:Debug>:-fsanitize=undefined>)
    endif()

    if(HAS_LIBCPP)
        target_compile_options(${LIB_NAME} BEFORE PUBLIC -stdlib=libc++)
    endif()
endfunction()

# 
# Build and install libraries
#

if (BUILD_UVW_SHARED_LIB)
    add_library(uvw-shared SHARED)
    add_library(uvw::uvw-shared ALIAS uvw-shared)
    target_link_libraries(uvw-shared PUBLIC $<$<TARGET_EXISTS:uv::uv-shared>:uv::uv-shared>)
    set_target_properties(uvw-shared PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD 1)

    add_uvw_library(uvw-shared)

    install(TARGETS uvw-shared EXPORT uvw ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR})

    if(FETCH_LIBUV)
        install(TARGETS uv_a EXPORT uvw ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
else()
    add_library(uvw-static STATIC)
    add_library(uvw::uvw-static ALIAS uvw-static)
    target_link_libraries(uvw-static PUBLIC $<$<TARGET_EXISTS:uv::uv-static>:uv::uv-static> $<$<NOT:$<TARGET_EXISTS:uv::uv-static>>:uv_a dl>)
    set_target_properties(uvw-static PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD 1)

    add_uvw_library(uvw-static)

    install(TARGETS uvw-static EXPORT uvw ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

    if(FETCH_LIBUV)
        install(TARGETS uv EXPORT uvw LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
endif()
