#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Async, Send) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::AsyncHandle>();

    bool checkErrorEvent = false;
    bool checkAsyncEvent = false;

    handle->on<uvw::ErrorEvent>([&checkErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkErrorEvent);
        checkErrorEvent = true;
    });

    handle->on<uvw::AsyncEvent>([&checkAsyncEvent](const auto &, auto &handle) {
        ASSERT_FALSE(checkAsyncEvent);
        checkAsyncEvent = true;
        handle.close();
        ASSERT_TRUE(handle.closing());
    });

    handle->send();

    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_FALSE(checkErrorEvent);
    ASSERT_TRUE(checkAsyncEvent);
}


TEST(Async, Fake) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::AsyncHandle>();

    auto l = [](const auto &, auto &) { FAIL(); };
    handle->on<uvw::ErrorEvent>(l);
    handle->on<uvw::AsyncEvent>(l);

    handle->send();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
