#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Loop, PartiallyDone) {
    auto def = uvw::Loop::getDefault();

    ASSERT_TRUE(static_cast<bool>(def));
    ASSERT_FALSE(def->alive());
    ASSERT_NO_THROW(def->stop());

    def->walk([](uvw::BaseHandle &) { ASSERT_TRUE(false); });

    auto loop = uvw::Loop::create();
    auto handle = loop->resource<uvw::PrepareHandle>();
    auto req = loop->resource<uvw::WorkReq>([]() {});

    auto err = [](const auto &, auto &) { ASSERT_TRUE(false); };

    loop->on<uvw::ErrorEvent>(err);
    req->on<uvw::ErrorEvent>(err);
    handle->on<uvw::ErrorEvent>(err);

    ASSERT_TRUE(static_cast<bool>(handle));
    ASSERT_TRUE(static_cast<bool>(req));

    ASSERT_FALSE(loop->alive());

    handle->start();
    handle->on<uvw::PrepareEvent>([](const auto &, auto &handle) {
        handle.loop().walk([](uvw::BaseHandle &) {
            static bool trigger = true;
            ASSERT_TRUE(trigger);
            trigger = false;
        });

        handle.close();
    });

    ASSERT_TRUE(loop->alive());
    ASSERT_NO_THROW(loop->run());

    loop->walk([](uvw::BaseHandle &) { ASSERT_TRUE(false); });

    ASSERT_NO_THROW(loop->run<uvw::Loop::Mode::ONCE>());
    ASSERT_NO_THROW(loop->run<uvw::Loop::Mode::NOWAIT>());

    ASSERT_FALSE(loop->alive());
}
