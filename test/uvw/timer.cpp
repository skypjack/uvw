#include <gtest/gtest.h>
#include <uvw/timer.h>

TEST(Timer, StartAndStop) {
    auto loop = uvw::loop::get_default();
    auto handleNoRepeat = loop->resource<uvw::timer_handle>();
    auto handleRepeat = loop->resource<uvw::timer_handle>();

    bool checkTimerNoRepeatEvent = false;
    bool checkTimerRepeatEvent = false;

    handleNoRepeat->on<uvw::error_event>([](auto &&...) { FAIL(); });
    handleRepeat->on<uvw::error_event>([](auto &&...) { FAIL(); });

    handleNoRepeat->on<uvw::timer_event>([&checkTimerNoRepeatEvent](const auto &, auto &handle) {
        ASSERT_FALSE(checkTimerNoRepeatEvent);
        checkTimerNoRepeatEvent = true;
        handle.stop();
        handle.close();
        ASSERT_TRUE(handle.closing());
    });

    handleRepeat->on<uvw::timer_event>([&checkTimerRepeatEvent](const auto &, auto &handle) {
        if(checkTimerRepeatEvent) {
            handle.stop();
            handle.close();
            ASSERT_TRUE(handle.closing());
        } else {
            checkTimerRepeatEvent = true;
            ASSERT_FALSE(handle.closing());
        }
    });

    handleNoRepeat->start(uvw::timer_handle::time{0}, uvw::timer_handle::time{0});
    handleRepeat->start(uvw::timer_handle::time{0}, uvw::timer_handle::time{1});

    ASSERT_TRUE(handleNoRepeat->active());
    ASSERT_FALSE(handleNoRepeat->closing());

    ASSERT_TRUE(handleRepeat->active());
    ASSERT_FALSE(handleRepeat->closing());

    loop->run();

    ASSERT_TRUE(checkTimerNoRepeatEvent);
    ASSERT_TRUE(checkTimerRepeatEvent);
}

TEST(Timer, Again) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::timer_handle>();

    bool checkErrorEvent = false;
    bool checkTimerEvent = false;

    handle->on<uvw::error_event>([&checkErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkErrorEvent);
        checkErrorEvent = true;
    });

    handle->on<uvw::timer_event>([&checkTimerEvent](const auto &, auto &hndl) {
        static bool guard = false;

        if(guard) {
            hndl.stop();
            hndl.close();
            checkTimerEvent = true;
            ASSERT_TRUE(hndl.closing());
        } else {
            guard = true;
            hndl.again();
            ASSERT_EQ(hndl.repeat(), uvw::timer_handle::time{1});
            ASSERT_FALSE(hndl.closing());
        }
    });

    ASSERT_NO_THROW(handle->again());
    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(checkErrorEvent);

    checkErrorEvent = false;
    handle->start(uvw::timer_handle::time{0}, uvw::timer_handle::time{1});

    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_FALSE(checkErrorEvent);
    ASSERT_TRUE(checkTimerEvent);

    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    handle->start(uvw::timer_handle::time{0}, uvw::timer_handle::time{1});

    ASSERT_TRUE(checkErrorEvent);
}

TEST(Timer, Repeat) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::timer_handle>();

    ASSERT_NO_THROW(handle->repeat(uvw::timer_handle::time{42}));
    ASSERT_EQ(handle->repeat(), uvw::timer_handle::time{42});
    ASSERT_NO_THROW(handle->close());

    // this forces an internal call to the close callback
    // (possible leak detected by valgrind otherwise)
    loop->run();
}

TEST(Timer, Fake) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::timer_handle>();

    handle->on<uvw::error_event>([](auto &&...) { FAIL(); });
    handle->on<uvw::timer_event>([](auto &&...) { FAIL(); });

    handle->start(uvw::timer_handle::time{0}, uvw::timer_handle::time{0});
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}

TEST(Timer, BaseHandleWalk) {
    auto loop = uvw::loop::get_default();
    auto timer = loop->resource<uvw::timer_handle>();

    timer->on<uvw::timer_event>([](const auto &, uvw::timer_handle &handle) {
        handle.parent().walk(uvw::overloaded{[](uvw::timer_handle &h) { h.close(); }, [](auto &&) {}});
    });

    timer->start(uvw::timer_handle::time{100}, uvw::timer_handle::time{100});
    loop->run();
}
