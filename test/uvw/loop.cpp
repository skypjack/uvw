#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Loop, DefaultLoop) {
    auto def = uvw::Loop::getDefault();

    ASSERT_TRUE(static_cast<bool>(def));
    ASSERT_FALSE(def->alive());
    ASSERT_NO_THROW(def->stop());

    def->walk([](uvw::BaseHandle &) { FAIL(); });

    auto def2 = uvw::Loop::getDefault();
    ASSERT_EQ(def, def2);
}


TEST(Loop, Functionalities) {
    auto loop = uvw::Loop::create();
    auto handle = loop->resource<uvw::PrepareHandle>();
    auto req = loop->resource<uvw::WorkReq>([]{});

    auto err = [](const auto &, auto &) { FAIL(); };

    loop->on<uvw::ErrorEvent>(err);
    req->on<uvw::ErrorEvent>(err);
    handle->on<uvw::ErrorEvent>(err);

    ASSERT_TRUE(static_cast<bool>(handle));
    ASSERT_TRUE(static_cast<bool>(req));

    ASSERT_TRUE(loop->descriptor());
    ASSERT_NO_THROW(loop->now());
    ASSERT_NO_THROW(loop->update());
    ASSERT_NO_THROW(loop->fork());

    ASSERT_FALSE(loop->alive());
    ASSERT_FALSE(loop->timeout().first);

    handle->start();
    handle->on<uvw::PrepareEvent>([](const auto &, auto &hndl) {
        hndl.loop().walk([](uvw::BaseHandle &) {
            static bool trigger = true;
            ASSERT_TRUE(trigger);
            trigger = false;
        });

        hndl.close();
    });

    ASSERT_TRUE(loop->alive());
    ASSERT_TRUE(loop->timeout().first);
    ASSERT_NO_THROW(loop->run());

    loop->walk([](uvw::BaseHandle &) { FAIL(); });

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

TEST(Loop, Raw) {
    auto loop = uvw::Loop::getDefault();
    const auto &cloop = uvw::Loop::getDefault();

    auto *raw = loop->raw();
    auto *craw = cloop->raw();

    ASSERT_EQ(raw, craw);
}
