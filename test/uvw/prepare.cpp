#include <gtest/gtest.h>
#include <uvw/prepare.h>

TEST(Prepare, StartAndStop) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::PrepareHandle>();

    bool checkPrepareEvent = false;

    handle->on<uvw::ErrorEvent>([](auto &&...) { FAIL(); });

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

    handle->on<uvw::ErrorEvent>([](auto &&...) { FAIL(); });
    handle->on<uvw::PrepareEvent>([](auto &&...) { FAIL(); });

    handle->start();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
