#include <gtest/gtest.h>
#include <uvw/check.h>
#include <uvw/timer.h>
#include <uvw/work.h>

TEST(Work, RunTask) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::CheckHandle>();

    bool checkTask = false;

    auto req = loop->resource<uvw::WorkReq>([&checkTask]() {
        ASSERT_FALSE(checkTask);
        checkTask = true;
    });

    req->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    req->on<uvw::WorkEvent>([&handle](const auto &, auto &) {
        handle->close();
    });

    handle->start();
    req->queue();
    loop->run();

    ASSERT_TRUE(checkTask);
}

TEST(Work, Cancellation) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::TimerHandle>();

    bool checkErrorEvent = false;

    handle->on<uvw::TimerEvent>([](const auto &, auto &hndl) {
        hndl.stop();
        hndl.close();
    });

    for(auto i = 0; i < 5 /* default uv thread pool size + 1 */; ++i) {
        auto req = loop->resource<uvw::WorkReq>([]() {});

        req->on<uvw::WorkEvent>([](const auto &, auto &) {});
        req->on<uvw::ErrorEvent>([&checkErrorEvent](const auto &, auto &) { checkErrorEvent = true; });

        req->queue();
        req->cancel();
    }

    handle->start(uvw::TimerHandle::Time{500}, uvw::TimerHandle::Time{500});
    loop->run();

    ASSERT_TRUE(checkErrorEvent);
}
