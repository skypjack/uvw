#include <gtest/gtest.h>
#include <uvw/emitter.hpp>
#include <uvw/event.hpp>


struct FakeEvent: uvw::Event<FakeEvent> { };

struct TestEmitter: uvw::Emitter<TestEmitter> {
    void emit() { publish(FakeEvent{}); }
};


TEST(Emitter, ClearAndClear) {
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
