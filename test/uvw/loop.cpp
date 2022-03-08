#include <gtest/gtest.h>
#include <uvw/loop.h>
#include <uvw/prepare.h>
#include <uvw/work.h>

TEST(Loop, DefaultLoop) {
    auto def = uvw::Loop::getDefault();

    ASSERT_TRUE(static_cast<bool>(def));
    ASSERT_FALSE(def->alive());
    ASSERT_NO_THROW(def->stop());

    def->walk([](auto &) { FAIL(); });

    auto def2 = uvw::Loop::getDefault();
    ASSERT_EQ(def, def2);
}

TEST(Loop, Functionalities) {
    auto loop = uvw::Loop::create();
    auto handle = loop->resource<uvw::PrepareHandle>();
    auto req = loop->resource<uvw::WorkReq>([] {});

    loop->on<uvw::ErrorEvent>([](auto &&...) { FAIL(); });
    req->on<uvw::ErrorEvent>([](auto &&...) { FAIL(); });
    handle->on<uvw::ErrorEvent>([](auto &&...) { FAIL(); });

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
    handle->on<uvw::PrepareEvent>([](const auto &, auto &hndl) {
        hndl.loop().walk([](auto &) {
            static bool trigger = true;
            ASSERT_TRUE(trigger);
            trigger = false;
        });

        hndl.close();
    });

    ASSERT_TRUE(loop->alive());
    ASSERT_NO_THROW(loop->run());

    loop->walk([](auto &) { FAIL(); });

    ASSERT_NO_THROW(loop->run<uvw::Loop::Mode::ONCE>());
    ASSERT_NO_THROW(loop->run<uvw::Loop::Mode::NOWAIT>());

    ASSERT_FALSE(loop->alive());
}

TEST(Loop, UserData) {
    auto loop = uvw::Loop::create();
    loop->data(std::make_shared<int>(42));

    ASSERT_EQ(*std::static_pointer_cast<int>(loop->data()), 42);
    ASSERT_EQ(*loop->data<int>(), 42);

    loop->run();

    ASSERT_EQ(*std::static_pointer_cast<int>(loop->data()), 42);
    ASSERT_EQ(*loop->data<int>(), 42);
}

TEST(Loop, Configure) {
    auto loop = uvw::Loop::create();
    ASSERT_NO_THROW(loop->configure(uvw::Loop::Configure::BLOCK_SIGNAL, 9));
    ASSERT_NO_THROW(loop->run());
}

TEST(Loop, IdleTime) {
    auto loop = uvw::Loop::create();
    loop->configure(uvw::Loop::Configure::IDLE_TIME);
    ASSERT_EQ(loop->idleTime().count(), 0u);
}

TEST(Loop, Raw) {
    auto loop = uvw::Loop::getDefault();
    const auto &cloop = uvw::Loop::getDefault();

    auto *raw = loop->raw();
    auto *craw = cloop->raw();

    ASSERT_EQ(raw, craw);
}
