#include <gtest/gtest.h>
#include <uvw/prepare.h>

TEST(Prepare, StartAndStop) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::prepare_handle>();

    bool checkPrepareEvent = false;

    handle->on<uvw::error_event>([](auto &&...) { FAIL(); });

    handle->on<uvw::prepare_event>([&checkPrepareEvent](const auto &, auto &hndl) {
        ASSERT_FALSE(checkPrepareEvent);

        checkPrepareEvent = true;

        ASSERT_EQ(0, hndl.stop());

        hndl.close();

        ASSERT_TRUE(hndl.closing());
    });

    ASSERT_EQ(0, handle->start());
    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_TRUE(checkPrepareEvent);
}

TEST(Prepare, Fake) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::prepare_handle>();

    handle->on<uvw::error_event>([](auto &&...) { FAIL(); });
    handle->on<uvw::prepare_event>([](auto &&...) { FAIL(); });

    handle->start();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
