#include <gtest/gtest.h>
#include <uvw/check.h>
#include <uvw/signal.h>

TEST(Signal, Start) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::signal_handle>();

    handle->on<uvw::error_event>([](auto &&...) { FAIL(); });
    handle->on<uvw::check_event>([](auto &&...) { FAIL(); });

    handle->start(2);

    ASSERT_EQ(2, handle->signal());

    handle->stop();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}

TEST(Signal, OneShot) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::signal_handle>();

    handle->on<uvw::error_event>([](auto &&...) { FAIL(); });
    handle->on<uvw::check_event>([](auto &&...) { FAIL(); });

    handle->one_shot(2);

    ASSERT_EQ(2, handle->signal());

    handle->stop();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
