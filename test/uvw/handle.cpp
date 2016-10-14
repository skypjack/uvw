#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Handle, Functionalities) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::AsyncHandle>();

    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());
    ASSERT_NO_THROW(handle->close());
    ASSERT_FALSE(handle->active());

    // this forces an internal call to the close callback
    // (possible leak detected by valgrind otherwise)
    loop->run();

    ASSERT_NO_THROW(handle->reference());
    ASSERT_TRUE(handle->referenced());
    ASSERT_NO_THROW(handle->unreference());
    ASSERT_FALSE(handle->referenced());

    ASSERT_NE(handle->size(), static_cast<decltype(handle->size())>(0));

    ASSERT_EQ(handle->sendBufferSize(), static_cast<decltype(handle->sendBufferSize())>(0));
    ASSERT_FALSE(handle->sendBufferSize(0));

    ASSERT_EQ(handle->recvBufferSize(), static_cast<decltype(handle->recvBufferSize())>(0));
    ASSERT_FALSE(handle->recvBufferSize(0));

    ASSERT_NO_THROW(handle->fileno());
}
