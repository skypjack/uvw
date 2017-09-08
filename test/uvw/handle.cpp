#include <gtest/gtest.h>
#include <uvw.hpp>


struct fake_handle_t { void *data; };


struct FakeHandle: uvw::Handle<FakeHandle, fake_handle_t> {
    using Handle::Handle;

    template<typename... Args>
    bool init(Args&&...) { return initialize([](auto...){ return true; }); }
};


TEST(Handle, Functionalities) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::AsyncHandle>();

    ASSERT_EQ(uvw::Utilities::guessHandle(handle->category()), uvw::HandleType::ASYNC);
    ASSERT_EQ(handle->type(), uvw::HandleType::ASYNC);

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


TEST(Handle, InitializationFailure) {
    auto loop = uvw::Loop::getDefault();
    auto resource = loop->resource<FakeHandle>();

    ASSERT_FALSE(static_cast<bool>(resource));
}
