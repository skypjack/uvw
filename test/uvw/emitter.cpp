#include <type_traits>
#include <gtest/gtest.h>
#include <uvw/emitter.h>

struct FakeEvent {};

struct TestEmitter: uvw::emitter<TestEmitter, FakeEvent> {
    void emit() {
        publish(FakeEvent{});
    }
};

TEST(ErrorEvent, Functionalities) {
    auto ecode = static_cast<std::underlying_type_t<uv_errno_t>>(UV_EADDRINUSE);

    uvw::error_event event{ecode};

    ASSERT_EQ(ecode, uvw::error_event::translate(ecode));
    ASSERT_NE(event.what(), nullptr);
    ASSERT_NE(event.name(), nullptr);
    ASSERT_EQ(event.code(), ecode);

    ASSERT_FALSE(static_cast<bool>(uvw::error_event{0}));
    ASSERT_TRUE(static_cast<bool>(uvw::error_event{ecode}));
}

TEST(Emitter, Functionalities) {
    TestEmitter emitter{};

    emitter.on<uvw::error_event>([](const uvw::error_event &, TestEmitter &) {});

    ASSERT_TRUE(emitter.has<uvw::error_event>());
    ASSERT_FALSE(emitter.has<FakeEvent>());

    emitter.reset<FakeEvent>();

    ASSERT_TRUE(emitter.has<uvw::error_event>());
    ASSERT_FALSE(emitter.has<FakeEvent>());

    emitter.reset<uvw::error_event>();

    ASSERT_FALSE(emitter.has<uvw::error_event>());
    ASSERT_FALSE(emitter.has<FakeEvent>());

    bool sentinel = false;
    emitter.on<uvw::error_event>([](const auto &, auto &) {});
    emitter.on<FakeEvent>([&](const auto &, auto &) { sentinel = true; });

    ASSERT_FALSE(sentinel);
    ASSERT_TRUE(emitter.has<uvw::error_event>());
    ASSERT_TRUE(emitter.has<FakeEvent>());

    emitter.emit();

    ASSERT_TRUE(sentinel);
    ASSERT_TRUE(emitter.has<uvw::error_event>());
    ASSERT_TRUE(emitter.has<FakeEvent>());

    emitter.reset();

    ASSERT_FALSE(emitter.has<uvw::error_event>());
    ASSERT_FALSE(emitter.has<FakeEvent>());
}
