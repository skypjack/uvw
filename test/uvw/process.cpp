#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Process, StdIO) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::ProcessHandle>();
    auto pipe = loop->resource<uvw::PipeHandle>();

    handle->stdio(*pipe, uvw::Flags<uvw::ProcessHandle::StdIO>::from<uvw::ProcessHandle::StdIO::CREATE_PIPE, uvw::ProcessHandle::StdIO::READABLE_PIPE>());
    pipe->close();
    loop->run();
}


TEST(Process, TODO) {
    auto loop = uvw::Loop::getDefault();
    auto handle = uvw::ProcessHandle::create(loop);

    handle = nullptr;

    // TODO
}
