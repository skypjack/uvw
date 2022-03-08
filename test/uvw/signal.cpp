#include <gtest/gtest.h>
#include <uvw/check.h>
#include <uvw/signal.h>

TEST(Signal, Start) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::SignalHandle>();

    handle->on<uvw::ErrorEvent>([](auto &&...) { FAIL(); });
    handle->on<uvw::CheckEvent>([](auto &&...) { FAIL(); });

    handle->start(2);

    ASSERT_EQ(2, handle->signal());

    handle->stop();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}

TEST(Signal, OneShot) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::SignalHandle>();

    handle->on<uvw::ErrorEvent>([](auto &&...) { FAIL(); });
    handle->on<uvw::CheckEvent>([](auto &&...) { FAIL(); });

    handle->oneShot(2);

    ASSERT_EQ(2, handle->signal());

    handle->stop();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
