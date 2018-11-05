#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Idle, StartAndStop) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::IdleHandle>();

    bool checkIdleEvent = false;

    handle->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::IdleHandle &) { FAIL(); });

    handle->on<uvw::IdleEvent>([&checkIdleEvent](const uvw::IdleEvent &, uvw::IdleHandle &hndl) {
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

    ASSERT_TRUE(checkIdleEvent);
}


TEST(Idle, Fake) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::IdleHandle>();

    handle->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::IdleHandle &) { FAIL(); });
    handle->on<uvw::IdleEvent>([](const uvw::IdleEvent &, uvw::IdleHandle &) { FAIL(); });

    handle->start();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
