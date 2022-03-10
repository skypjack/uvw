#include <gtest/gtest.h>
#include <uvw/async.h>

TEST(Async, Send) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::async_handle>();

    bool checkAsyncEvent = false;

    handle->on<uvw::error_event>([](auto &&...) { FAIL(); });

    handle->on<uvw::async_event>([&checkAsyncEvent](const auto &, auto &hndl) {
        ASSERT_FALSE(checkAsyncEvent);
        checkAsyncEvent = true;
        hndl.close();
        ASSERT_TRUE(hndl.closing());
    });

    handle->send();

    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_TRUE(checkAsyncEvent);
}

TEST(Async, Fake) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::async_handle>();

    handle->on<uvw::error_event>([](auto &&...) { FAIL(); });
    handle->on<uvw::async_event>([](auto &&...) { FAIL(); });

    handle->send();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
