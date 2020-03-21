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
        target_link_libraries(${LIB_NAME} PUBLIC $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer>)
    endif()

    if(HAS_LIBCPP)
        target_compile_options(${LIB_NAME} BEFORE PUBLIC -stdlib=libc++)
    endif()
endfunction()

#
# Static library
#

add_library(uvw-static STATIC EXCLUDE_FROM_ALL)
add_library(uvw::uvw-static ALIAS uvw-static)
target_link_libraries(uvw-static PUBLIC uv::uv-static)
add_uvw_library(uvw-static)

#
# Shared library
#

add_library(uvw-shared SHARED EXCLUDE_FROM_ALL)
add_library(uvw::uvw-shared ALIAS uvw-shared)
target_link_libraries(uvw-shared PUBLIC uv::uv-shared)
set_target_properties(uvw-shared PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${UVW_VERSION_MAJOR})
add_uvw_library(uvw-shared)

#
# Install targets
#

install(EXPORT uvw NAMESPACE uvw:: DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS uvw-static EXPORT uvw ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS uvw-shared EXPORT uvw LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS uv_a EXPORT uvw ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS uv EXPORT uvw LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
