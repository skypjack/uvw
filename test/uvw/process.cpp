#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Process, Pid) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::ProcessHandle>();

    ASSERT_EQ(handle->pid(), 0);

    loop->run();
}


TEST(Process, Cwd) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::ProcessHandle>();

    handle->cwd(".");

    loop->run();
}


TEST(Process, StdIO) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::ProcessHandle>();
    auto pipe = loop->resource<uvw::PipeHandle>();

    uvw::ProcessHandle::disableStdIOInheritance();
    handle->stdio(*pipe, uvw::Flags<uvw::ProcessHandle::StdIO>::from<uvw::ProcessHandle::StdIO::CREATE_PIPE, uvw::ProcessHandle::StdIO::READABLE_PIPE>());
    handle->stdio(uvw::StdIN, uvw::ProcessHandle::StdIO::IGNORE_STREAM);
    handle->stdio(uvw::StdOUT, uvw::ProcessHandle::StdIO::IGNORE_STREAM);
    handle->stdio(uvw::StdOUT, uvw::ProcessHandle::StdIO::INHERIT_FD);

    pipe->close();
    loop->run();
}


TEST(Process, TODO) {
    auto loop = uvw::Loop::getDefault();
    auto handle = uvw::ProcessHandle::create(loop);

    handle = nullptr;

    // TODO
}
