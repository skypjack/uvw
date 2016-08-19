#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Signal, Fake) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::SignalHandle>();

    auto l = [](const auto &, auto &) { ASSERT_FALSE(true); };
    handle->on<uvw::ErrorEvent>(l);
    handle->on<uvw::CheckEvent>(l);

    handle->start(42);

    ASSERT_EQ(42, handle->signal());

    handle->stop();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
