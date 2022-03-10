#include <gtest/gtest.h>
#include <uvw/check.h>
#include <uvw/timer.h>
#include <uvw/work.h>

TEST(Work, RunTask) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::check_handle>();

    bool checkTask = false;

    auto req = loop->resource<uvw::work_req>([&checkTask]() {
        ASSERT_FALSE(checkTask);
        checkTask = true;
    });

    req->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    req->on<uvw::work_event>([&handle](const auto &, auto &) {
        handle->close();
    });

    handle->start();
    req->queue();
    loop->run();

    ASSERT_TRUE(checkTask);
}

TEST(Work, Cancellation) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::timer_handle>();

    bool checkErrorEvent = false;

    handle->on<uvw::timer_event>([](const auto &, auto &hndl) {
        hndl.stop();
        hndl.close();
    });

    for(auto i = 0; i < 5 /* default uv thread pool size + 1 */; ++i) {
        auto req = loop->resource<uvw::work_req>([]() {});

        req->on<uvw::work_event>([](const auto &, auto &) {});
        req->on<uvw::error_event>([&checkErrorEvent](const auto &, auto &) { checkErrorEvent = true; });

        req->queue();
        req->cancel();
    }

    handle->start(uvw::timer_handle::time{500}, uvw::timer_handle::time{500});
    loop->run();

    ASSERT_TRUE(checkErrorEvent);
}
