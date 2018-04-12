#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(UnderlyingType, Functionalities) {
    auto loop = uvw::Loop::getDefault();
    auto handle = uvw::AsyncHandle::create(loop);

    ASSERT_TRUE(handle);
    ASSERT_EQ(&handle->loop(), loop.get());
}

TEST(UnderlyingType, Raw) {
    auto loop = uvw::Loop::getDefault();
    auto handle = uvw::AsyncHandle::create(loop);
    const auto &chandle = handle;

    auto *raw = handle->raw();
    auto *craw = chandle->raw();

    ASSERT_EQ(raw, craw);
}
