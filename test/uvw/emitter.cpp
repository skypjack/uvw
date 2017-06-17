#include <type_traits>
#include <gtest/gtest.h>
#include <uvw/emitter.hpp>


struct FakeEvent { };

struct TestEmitter: uvw::Emitter<TestEmitter> {
    void emit() { publish(FakeEvent{}); }
};


TEST(ErrorEvent, Functionalities) {
    auto ecode = static_cast<std::underlying_type_t<uv_errno_t>>(UV_EADDRINUSE);

    uvw::ErrorEvent event{ecode};

    ASSERT_EQ(ecode, uvw::ErrorEvent::translate(ecode));
    ASSERT_NE(event.what(), nullptr);
    ASSERT_NE(event.name(), nullptr);
    ASSERT_EQ(event.code(), ecode);

    ASSERT_FALSE(static_cast<bool>(uvw::ErrorEvent{0}));
    ASSERT_TRUE(static_cast<bool>(uvw::ErrorEvent{ecode}));
}


TEST(Emitter, EmptyAndClear) {
    TestEmitter emitter{};

    ASSERT_TRUE(emitter.empty());

    emitter.on<uvw::ErrorEvent>([](const auto &, auto &){});

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::ErrorEvent>());
    ASSERT_TRUE(emitter.empty<FakeEvent>());

    emitter.clear<FakeEvent>();

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::ErrorEvent>());
    ASSERT_TRUE(emitter.empty<FakeEvent>());

    emitter.clear<uvw::ErrorEvent>();

    ASSERT_TRUE(emitter.empty());
    ASSERT_TRUE(emitter.empty<uvw::ErrorEvent>());
    ASSERT_TRUE(emitter.empty<FakeEvent>());

    emitter.on<uvw::ErrorEvent>([](const auto &, auto &){});
    emitter.on<FakeEvent>([](const auto &, auto &){});

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<uvw::ErrorEvent>());
    ASSERT_FALSE(emitter.empty<FakeEvent>());

    emitter.clear();

    ASSERT_TRUE(emitter.empty());
    ASSERT_TRUE(emitter.empty<uvw::ErrorEvent>());
    ASSERT_TRUE(emitter.empty<FakeEvent>());
}


TEST(Emitter, On) {
    TestEmitter emitter{};

    emitter.on<FakeEvent>([](const auto &, auto &){});

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<FakeEvent>());

    emitter.emit();

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<FakeEvent>());
}


TEST(Emitter, Once) {
    TestEmitter emitter{};

    emitter.once<FakeEvent>([](const auto &, auto &){});

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<FakeEvent>());

    emitter.emit();

    ASSERT_TRUE(emitter.empty());
    ASSERT_TRUE(emitter.empty<FakeEvent>());
}


TEST(Emitter, OnceAndErase) {
    TestEmitter emitter{};

    auto conn = emitter.once<FakeEvent>([](const auto &, auto &){});

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<FakeEvent>());

    emitter.erase(conn);

    ASSERT_TRUE(emitter.empty());
    ASSERT_TRUE(emitter.empty<FakeEvent>());
}


TEST(Emitter, OnAndErase) {
    TestEmitter emitter{};

    auto conn = emitter.on<FakeEvent>([](const auto &, auto &){});

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<FakeEvent>());

    emitter.erase(conn);

    ASSERT_TRUE(emitter.empty());
    ASSERT_TRUE(emitter.empty<FakeEvent>());
}


TEST(Emitter, CallbackClear) {
    TestEmitter emitter{};

    emitter.on<FakeEvent>([](const auto &, auto &ref) {
        ref.template on<FakeEvent>([](const auto &, auto &){});
        ref.clear();
    });

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<FakeEvent>());

    emitter.emit();

    ASSERT_TRUE(emitter.empty());
    ASSERT_TRUE(emitter.empty<FakeEvent>());

    emitter.on<FakeEvent>([](const auto &, auto &ref) {
        ref.clear();
        ref.template on<FakeEvent>([](const auto &, auto &){});
    });

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<FakeEvent>());

    emitter.emit();

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.empty<FakeEvent>());
}
