#include <gtest/gtest.h>
#include <uvw/pipe.h>
#include <uvw/process.h>

TEST(Process, Pid) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::process_handle>();

    ASSERT_EQ(handle->pid(), 0);

    loop->run();
}

TEST(Process, Cwd) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::process_handle>();

    handle->cwd(".");

    loop->run();
}

TEST(Process, StdIO) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::process_handle>();
    auto pipe = loop->resource<uvw::pipe_handle>();

    uvw::process_handle::disable_stdio_inheritance();
    handle->stdio(*pipe, uvw::process_handle::stdio_flags::CREATE_PIPE | uvw::process_handle::stdio_flags::READABLE_PIPE);
    handle->stdio(uvw::std_in, uvw::process_handle::stdio_flags::IGNORE_STREAM);
    handle->stdio(uvw::std_out, uvw::process_handle::stdio_flags::IGNORE_STREAM);
    handle->stdio(uvw::std_out, uvw::process_handle::stdio_flags::INHERIT_FD);

    pipe->close();
    loop->run();
}
