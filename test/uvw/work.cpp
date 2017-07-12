#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Work, RunTask) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::CheckHandle>();

    bool checkWorkEvent = false;
    bool checkTask = false;

    handle->on<uvw::CheckEvent>([&checkWorkEvent](const auto &, auto &hndl) {
        if(checkWorkEvent) {
            hndl.stop();
            hndl.close();
        }
    });

    auto req = loop->resource<uvw::WorkReq>([&checkTask]() {
        ASSERT_FALSE(checkTask);
        checkTask = true;
    });

    req->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    req->on<uvw::WorkEvent>([&checkWorkEvent](const auto &, auto &) {
        ASSERT_FALSE(checkWorkEvent);
        checkWorkEvent = true;
    });

    handle->start();
    req->queue();
    loop->run();

    ASSERT_TRUE(checkWorkEvent);
    ASSERT_TRUE(checkTask);
}

TEST(Work, Cancellation) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::TimerHandle>();

    bool checkErrorEvent = false;
    bool checkWorkEvent = false;
    bool checkTask = false;

    handle->on<uvw::TimerEvent>([](const auto &, auto &hndl) {
        hndl.stop();
        hndl.close();
    });

    auto req = loop->resource<uvw::WorkReq>([&checkTask]() {
        ASSERT_FALSE(checkTask);
        checkTask = true;
    });

    req->on<uvw::ErrorEvent>([&checkErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkErrorEvent);
        checkErrorEvent = true;
    });

    req->on<uvw::WorkEvent>([&checkWorkEvent](const auto &, auto &) {
        ASSERT_FALSE(checkWorkEvent);
        checkWorkEvent = true;
    });

    handle->start(uvw::TimerHandle::Time{500}, uvw::TimerHandle::Time{500});
    req->queue();
    req->cancel();
    loop->run();

    ASSERT_TRUE(checkErrorEvent);
    ASSERT_FALSE(checkWorkEvent);
    ASSERT_FALSE(checkTask);
}
