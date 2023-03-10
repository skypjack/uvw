#include <gtest/gtest.h>
#include <uvw/idle.h>

TEST(Idle, StartAndStop) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::idle_handle>();

    bool checkIdleEvent = false;

    handle->on<uvw::error_event>([](auto &&...) { FAIL(); });

    handle->on<uvw::idle_event>([&checkIdleEvent](const auto &, auto &hndl) {
        ASSERT_FALSE(checkIdleEvent);

        checkIdleEvent = true;

        ASSERT_EQ(0, hndl.stop());

        hndl.close();

        ASSERT_TRUE(hndl.closing());
    });

    ASSERT_EQ(0, handle->start());
    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_TRUE(checkIdleEvent);
}

TEST(Idle, Fake) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::idle_handle>();

    handle->on<uvw::error_event>([](auto &&...) { FAIL(); });
    handle->on<uvw::idle_event>([](auto &&...) { FAIL(); });

    handle->start();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
