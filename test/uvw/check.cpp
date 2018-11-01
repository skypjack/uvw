#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Check, StartAndStop) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::CheckHandle>();

    bool checkCheckEvent = false;

    handle->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::CheckHandle &) { FAIL(); });

    handle->on<uvw::CheckEvent>([&checkCheckEvent](const uvw::CheckEvent &, uvw::CheckHandle &hndl) {
        ASSERT_FALSE(checkCheckEvent);
        checkCheckEvent = true;
        hndl.stop();
        hndl.close();
        ASSERT_TRUE(hndl.closing());
    });

    handle->start();

    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run<uvw::Loop::Mode::NOWAIT>();

    ASSERT_TRUE(checkCheckEvent);
}


TEST(Check, Fake) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::CheckHandle>();

    handle->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::CheckHandle &) { FAIL(); });
    handle->on<uvw::CheckEvent>([](const uvw::CheckEvent &, uvw::CheckHandle &) { FAIL(); });

    handle->start();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
