#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Work, RunTask) {
    auto loop = uvw::Loop::getDefault();

    bool checkErrorEvent = false;
    bool checkWorkEvent = false;
    bool checkTask = false;

    auto req = loop->resource<uvw::WorkReq>([&checkTask]() {
        ASSERT_FALSE(checkTask);
        checkTask = true;
    });

    req->on<uvw::WorkEvent>([&checkWorkEvent](const auto &, auto &) {
        ASSERT_FALSE(checkWorkEvent);
        checkWorkEvent = true;
    });

    req->on<uvw::ErrorEvent>([&checkErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkErrorEvent);
        checkErrorEvent = true;
    });

    req->queue();
    loop->run();

    ASSERT_FALSE(checkErrorEvent);
    ASSERT_TRUE(checkWorkEvent);
    ASSERT_TRUE(checkTask);
}

TEST(Work, Cancellation) {
    auto loop = uvw::Loop::getDefault();

    bool checkErrorEvent = false;
    bool checkWorkEvent = false;
    bool checkTask = false;

    auto req = loop->resource<uvw::WorkReq>([&checkTask]() {
        ASSERT_FALSE(checkTask);
        checkTask = true;
    });

    req->on<uvw::WorkEvent>([&checkWorkEvent](const auto &, auto &) {
        ASSERT_FALSE(checkWorkEvent);
        checkWorkEvent = true;
    });

    req->on<uvw::ErrorEvent>([&checkErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkErrorEvent);
        checkErrorEvent = true;
    });

    req->queue();
    req->cancel();
    loop->run();

    ASSERT_TRUE(checkErrorEvent);
    ASSERT_FALSE(checkWorkEvent);
    ASSERT_FALSE(checkTask);
}
