#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Idle, StartAndStop) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::IdleHandle>();

    bool checkErrorEvent = false;
    bool checkIdleEvent = false;

    handle->on<uvw::ErrorEvent>([&checkErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkErrorEvent);
        checkErrorEvent = true;
    });

    handle->on<uvw::IdleEvent>([&checkIdleEvent](const auto &, auto &hndl) {
        ASSERT_FALSE(checkIdleEvent);
        checkIdleEvent = true;
        hndl.stop();
        hndl.close();
        ASSERT_TRUE(hndl.closing());
    });

    handle->start();

    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_FALSE(checkErrorEvent);
    ASSERT_TRUE(checkIdleEvent);
}


TEST(Idle, Fake) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::IdleHandle>();

    auto l = [](const auto &, auto &) { FAIL(); };
    handle->on<uvw::ErrorEvent>(l);
    handle->on<uvw::IdleEvent>(l);

    handle->start();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
