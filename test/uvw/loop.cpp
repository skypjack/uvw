#include <gtest/gtest.h>
#include <uvw/loop.h>
#include <uvw/prepare.h>
#include <uvw/work.h>

TEST(Loop, DefaultLoop) {
    auto def = uvw::loop::get_default();

    ASSERT_TRUE(static_cast<bool>(def));
    ASSERT_FALSE(def->alive());
    ASSERT_NO_THROW(def->stop());

    def->walk([](auto &) { FAIL(); });

    auto def2 = uvw::loop::get_default();
    ASSERT_EQ(def, def2);
}

TEST(Loop, Functionalities) {
    auto loop = uvw::loop::create();
    auto handle = loop->resource<uvw::prepare_handle>();
    auto req = loop->resource<uvw::work_req>([] {});

    loop->on<uvw::error_event>([](auto &&...) { FAIL(); });
    req->on<uvw::error_event>([](auto &&...) { FAIL(); });
    handle->on<uvw::error_event>([](auto &&...) { FAIL(); });

    ASSERT_TRUE(static_cast<bool>(handle));
    ASSERT_TRUE(static_cast<bool>(req));

    ASSERT_TRUE(loop->descriptor());
    ASSERT_NO_THROW(loop->now());
    ASSERT_NO_THROW(loop->update());

#ifndef _MSC_VER
    // fork isn't implemented on Windows in libuv and it returns an error by default
    ASSERT_NO_THROW(loop->fork());
#endif

    ASSERT_FALSE(loop->alive());
    ASSERT_FALSE(loop->timeout().first);

    handle->start();
    handle->on<uvw::prepare_event>([](const auto &, auto &hndl) {
        hndl.parent().walk([](auto &) {
            static bool trigger = true;
            ASSERT_TRUE(trigger);
            trigger = false;
        });

        hndl.close();
    });

    ASSERT_TRUE(loop->alive());
    ASSERT_NO_THROW(loop->run());

    loop->walk([](auto &) { FAIL(); });

    ASSERT_NO_THROW(loop->run(uvw::loop::run_mode::ONCE));
    ASSERT_NO_THROW(loop->run(uvw::loop::run_mode::NOWAIT));

    ASSERT_FALSE(loop->alive());
}

TEST(Loop, UserData) {
    auto loop = uvw::loop::create();
    loop->data(std::make_shared<int>(42));

    ASSERT_EQ(*std::static_pointer_cast<int>(loop->data()), 42);
    ASSERT_EQ(*loop->data<int>(), 42);

    loop->run();

    ASSERT_EQ(*std::static_pointer_cast<int>(loop->data()), 42);
    ASSERT_EQ(*loop->data<int>(), 42);
}

TEST(Loop, Configure) {
    auto loop = uvw::loop::create();
    ASSERT_NO_THROW(loop->configure(uvw::loop::option::BLOCK_SIGNAL, 9));
    ASSERT_NO_THROW(loop->run());
}

TEST(Loop, IdleTime) {
    auto loop = uvw::loop::create();
    loop->configure(uvw::loop::option::IDLE_TIME);
    ASSERT_EQ(loop->idle_time().count(), 0u);
}

TEST(Loop, Raw) {
    auto loop = uvw::loop::get_default();
    const auto &cloop = uvw::loop::get_default();

    auto *raw = loop->raw();
    auto *craw = cloop->raw();

    ASSERT_EQ(raw, craw);
}
