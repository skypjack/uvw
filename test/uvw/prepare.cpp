#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Prepare, StartAndStop) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::PrepareHandle>();

    bool checkPrepareEvent = false;

    handle->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    handle->on<uvw::PrepareEvent>([&checkPrepareEvent](const auto &, auto &hndl) {
        ASSERT_FALSE(checkPrepareEvent);
        checkPrepareEvent = true;
        hndl.stop();
        hndl.close();
        ASSERT_TRUE(hndl.closing());
    });

    handle->start();

    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_TRUE(checkPrepareEvent);
}


TEST(Prepare, Fake) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::PrepareHandle>();

    auto l = [](const auto &, auto &) { FAIL(); };
    handle->on<uvw::ErrorEvent>(l);
    handle->on<uvw::PrepareEvent>(l);

    handle->start();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
