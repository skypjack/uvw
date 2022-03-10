#include <gtest/gtest.h>
#include <uvw/async.h>
#include <uvw/uv_type.hpp>

TEST(UvType, Functionalities) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::async_handle>();

    ASSERT_TRUE(handle);
    ASSERT_EQ(&handle->parent(), loop.get());
}

TEST(UvType, Raw) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::async_handle>();
    const auto &chandle = handle;

    auto *raw = handle->raw();
    auto *craw = chandle->raw();

    ASSERT_EQ(raw, craw);
}
