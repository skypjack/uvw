#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Poll, StartAndStopReadableWritable) {
    auto loop = uvw::Loop::getDefault();
    auto handle = uvw::PollHandle::create(loop, 0);

    handle = nullptr;

    // TODO
}
