#include <gtest/gtest.h>
#include <uvw/check.h>

TEST(Check, StartAndStop) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::check_handle>();

    bool checkCheckEvent = false;

    handle->on<uvw::error_event>([](auto &&...) { FAIL(); });

    handle->on<uvw::check_event>([&checkCheckEvent](const auto &, auto &hndl) {
        ASSERT_FALSE(checkCheckEvent);

        checkCheckEvent = true;

        ASSERT_EQ(0, hndl.stop());

        hndl.close();

        ASSERT_TRUE(hndl.closing());
    });

    ASSERT_EQ(0, handle->start());
    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run(uvw::loop::run_mode::NOWAIT);

    ASSERT_TRUE(checkCheckEvent);
}

TEST(Check, Fake) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::check_handle>();

    handle->on<uvw::error_event>([](auto &&...) { FAIL(); });
    handle->on<uvw::check_event>([](auto &&...) { FAIL(); });

    handle->start();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
