#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Prepare, StartAndStop) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::PrepareHandle>();

    bool checkPrepareEvent = false;

    handle->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::PrepareHandle &) { FAIL(); });

    handle->on<uvw::PrepareEvent>([&checkPrepareEvent](const uvw::PrepareEvent &, uvw::PrepareHandle &hndl) {
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

    handle->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::PrepareHandle &) { FAIL(); });
    handle->on<uvw::PrepareEvent>([](const uvw::PrepareEvent &, uvw::PrepareHandle &) { FAIL(); });

    handle->start();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
