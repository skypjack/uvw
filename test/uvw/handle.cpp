#include <gtest/gtest.h>
#include <uvw/async.h>
#include <uvw/handle.hpp>

struct fake_handle_t {
    void *data;
};

struct fake_handle: uvw::handle<fake_handle, fake_handle_t> {
    using handle::handle;

    int init() {
        return 1;
    }
};

TEST(Handle, Functionalities) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::async_handle>();

    ASSERT_EQ(uvw::utilities::guess_handle(handle->category()), uvw::handle_type::ASYNC);
    ASSERT_EQ(handle->type(), uvw::handle_type::ASYNC);

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

    ASSERT_LT(handle->send_buffer_size(), 0);
    ASSERT_NE(0, handle->send_buffer_size(0));

    ASSERT_LT(handle->recv_buffer_size(), 0);
    ASSERT_NE(0, handle->recv_buffer_size(0));

    ASSERT_NO_THROW([[maybe_unused]] const auto fd = handle->fd());
}

TEST(Handle, InitializationFailure) {
    auto loop = uvw::loop::get_default();
    auto resource = loop->resource<fake_handle>();

    ASSERT_FALSE(static_cast<bool>(resource));
}
