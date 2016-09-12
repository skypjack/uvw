#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Check, StartAndStop) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::CheckHandle>();

    bool checkErrorEvent = false;
    bool checkCheckEvent = false;

    handle->on<uvw::ErrorEvent>([&checkErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkErrorEvent);
        checkErrorEvent = true;
    });

    handle->on<uvw::CheckEvent>([&checkCheckEvent](const auto &, auto &handle) {
        ASSERT_FALSE(checkCheckEvent);
        checkCheckEvent = true;
        handle.stop();
        handle.close();
        ASSERT_TRUE(handle.closing());
    });

    handle->start();

    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run<uvw::Loop::Mode::NOWAIT>();

    ASSERT_FALSE(checkErrorEvent);
    ASSERT_TRUE(checkCheckEvent);
}


TEST(Check, Fake) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::CheckHandle>();

    auto l = [](const auto &, auto &) { FAIL(); };
    handle->on<uvw::ErrorEvent>(l);
    handle->on<uvw::CheckEvent>(l);

    handle->start();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
