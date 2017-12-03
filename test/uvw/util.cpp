#include <memory>
#include <cstdlib>
#include <gtest/gtest.h>
#include <uvw.hpp>


template<typename T>
struct tag { using type = T; };


TEST(Util, UnscopedFlags) {
    enum class UnscopedEnum { FOO = 1, BAR = 2, BAZ = 4, QUUX = 8 };

    uvw::Flags<UnscopedEnum> flags{};

    ASSERT_NO_THROW((flags = uvw::Flags<UnscopedEnum>::from<UnscopedEnum::FOO, UnscopedEnum::BAR>()));
    ASSERT_NO_THROW((flags = uvw::Flags<UnscopedEnum>{UnscopedEnum::BAZ}));
    ASSERT_NO_THROW((flags = uvw::Flags<UnscopedEnum>{static_cast<uvw::Flags<UnscopedEnum>::Type>(UnscopedEnum::QUUX)}));

    ASSERT_NO_THROW((flags = uvw::Flags<UnscopedEnum>{std::move(flags)}));
    ASSERT_NO_THROW((flags = uvw::Flags<UnscopedEnum>{flags}));

    flags = uvw::Flags<UnscopedEnum>::from<UnscopedEnum::FOO, UnscopedEnum::QUUX>();

    ASSERT_TRUE(static_cast<bool>(flags));
    ASSERT_EQ(static_cast<uvw::Flags<UnscopedEnum>::Type>(flags), 9);

    ASSERT_TRUE(flags & uvw::Flags<UnscopedEnum>::from<UnscopedEnum::FOO>());
    ASSERT_FALSE(flags & UnscopedEnum::BAR);
    ASSERT_FALSE(flags & uvw::Flags<UnscopedEnum>::from<UnscopedEnum::BAZ>());
    ASSERT_TRUE(flags & UnscopedEnum::QUUX);

    ASSERT_NO_THROW(flags = flags | UnscopedEnum::BAR);
    ASSERT_NO_THROW(flags = flags | uvw::Flags<UnscopedEnum>::from<UnscopedEnum::BAZ>());

    ASSERT_TRUE(flags & UnscopedEnum::FOO);
    ASSERT_TRUE(flags & uvw::Flags<UnscopedEnum>::from<UnscopedEnum::BAR>());
    ASSERT_TRUE(flags & UnscopedEnum::BAZ);
    ASSERT_TRUE(flags & uvw::Flags<UnscopedEnum>::from<UnscopedEnum::QUUX>());
}


TEST(Util, ScopedFlags) {
    enum class ScopedEnum { FOO = 1, BAR = 2, BAZ = 4, QUUX = 8 };

    uvw::Flags<ScopedEnum> flags{};

    ASSERT_NO_THROW((flags = uvw::Flags<ScopedEnum>::from<ScopedEnum::FOO, ScopedEnum::BAR>()));
    ASSERT_NO_THROW((flags = uvw::Flags<ScopedEnum>{ScopedEnum::BAZ}));
    ASSERT_NO_THROW((flags = uvw::Flags<ScopedEnum>{static_cast<uvw::Flags<ScopedEnum>::Type>(ScopedEnum::QUUX)}));

    ASSERT_NO_THROW((flags = uvw::Flags<ScopedEnum>{std::move(flags)}));
    ASSERT_NO_THROW((flags = uvw::Flags<ScopedEnum>{flags}));

    flags = uvw::Flags<ScopedEnum>::from<ScopedEnum::FOO, ScopedEnum::QUUX>();

    ASSERT_TRUE(static_cast<bool>(flags));
    ASSERT_EQ(static_cast<uvw::Flags<ScopedEnum>::Type>(flags), 9);

    ASSERT_TRUE(flags & uvw::Flags<ScopedEnum>::from<ScopedEnum::FOO>());
    ASSERT_FALSE(flags & ScopedEnum::BAR);
    ASSERT_FALSE(flags & uvw::Flags<ScopedEnum>::from<ScopedEnum::BAZ>());
    ASSERT_TRUE(flags & ScopedEnum::QUUX);

    ASSERT_NO_THROW(flags = flags | ScopedEnum::BAR);
    ASSERT_NO_THROW(flags = flags | uvw::Flags<ScopedEnum>::from<ScopedEnum::BAZ>());

    ASSERT_TRUE(flags & ScopedEnum::FOO);
    ASSERT_TRUE(flags & uvw::Flags<ScopedEnum>::from<ScopedEnum::BAR>());
    ASSERT_TRUE(flags & ScopedEnum::BAZ);
    ASSERT_TRUE(flags & uvw::Flags<ScopedEnum>::from<ScopedEnum::QUUX>());
}


TEST(Util, Utilities) {
    ASSERT_EQ(uvw::PidType{}, uvw::PidType{});

    ASSERT_NE(uvw::Utilities::OS::pid(), uvw::PidType{});
    ASSERT_NE(uvw::Utilities::OS::parent(), uvw::PidType{});
    ASSERT_FALSE(uvw::Utilities::OS::homedir().empty());
    ASSERT_FALSE(uvw::Utilities::OS::tmpdir().empty());
    ASSERT_NE(uvw::Utilities::OS::hostname(), "");

    ASSERT_TRUE(uvw::Utilities::OS::env("UVW_TEST_UTIL_UTILITIES", "TRUE"));
    ASSERT_TRUE(uvw::Utilities::OS::env("UVW_TEST_UTIL_UTILITIES") == "TRUE");
    ASSERT_TRUE(uvw::Utilities::OS::env("UVW_TEST_UTIL_UTILITIES", ""));
    ASSERT_FALSE(uvw::Utilities::OS::env("UVW_TEST_UTIL_UTILITIES") == "TRUE");

    auto passwd = uvw::Utilities::OS::passwd();

    ASSERT_TRUE(static_cast<bool>(passwd));
    ASSERT_FALSE(passwd.username().empty());
    ASSERT_FALSE(passwd.homedir().empty());
    ASSERT_NO_THROW(passwd.uid());
    ASSERT_NO_THROW(passwd.gid());
    ASSERT_FALSE(passwd.shell().empty());

    ASSERT_EQ(uvw::Utilities::guessHandle(uvw::FileHandle{-1}), uvw::HandleType::UNKNOWN);
    ASSERT_NE(uvw::Utilities::guessHandle(uvw::StdIN), uvw::HandleType::UNKNOWN);

    auto guessHandle = [](auto tag, auto type) {
        auto loop = uvw::Loop::getDefault();
        auto handle = loop->resource<typename decltype(tag)::type>();
        ASSERT_EQ(uvw::Utilities::guessHandle(handle->category()), type);
        handle->close();
        loop->run();
    };

    guessHandle(tag<uvw::AsyncHandle>{}, uvw::HandleType::ASYNC);
    guessHandle(tag<uvw::CheckHandle>{}, uvw::HandleType::CHECK);
    guessHandle(tag<uvw::FsEventHandle>{}, uvw::HandleType::FS_EVENT);
    guessHandle(tag<uvw::FsPollHandle>{}, uvw::HandleType::FS_POLL);
    guessHandle(tag<uvw::IdleHandle>{}, uvw::HandleType::IDLE);
    guessHandle(tag<uvw::PipeHandle>{}, uvw::HandleType::PIPE);
    guessHandle(tag<uvw::PrepareHandle>{}, uvw::HandleType::PREPARE);
    guessHandle(tag<uvw::TcpHandle>{}, uvw::HandleType::TCP);
    guessHandle(tag<uvw::TimerHandle>{}, uvw::HandleType::TIMER);
    guessHandle(tag<uvw::UDPHandle>{}, uvw::HandleType::UDP);
    guessHandle(tag<uvw::SignalHandle>{}, uvw::HandleType::SIGNAL);

    auto cpuInfo = uvw::Utilities::cpuInfo();

    ASSERT_NE(cpuInfo.size(), decltype(cpuInfo.size()){0});
    ASSERT_FALSE(cpuInfo[0].model.empty());
    ASSERT_NE(cpuInfo[0].speed, decltype(cpuInfo[0].speed){0});

    auto interfaceAddresses = uvw::Utilities::interfaceAddresses();

    ASSERT_NE(interfaceAddresses.size(), decltype(interfaceAddresses.size()){0});
    ASSERT_FALSE(interfaceAddresses[0].name.empty());
    ASSERT_FALSE(interfaceAddresses[0].address.ip.empty());
    ASSERT_FALSE(interfaceAddresses[0].netmask.ip.empty());

    ASSERT_NO_THROW(uvw::Utilities::indexToName(0));
    ASSERT_NO_THROW(uvw::Utilities::indexToIid(0));

    ASSERT_TRUE(uvw::Utilities::replaceAllocator(
        [](size_t size) { return malloc(size); },
        [](void *ptr, size_t size) { return realloc(ptr, size); },
        [](size_t num, size_t size) { return calloc(num, size); },
        [](void *ptr) { return free(ptr); }
    ));

    ASSERT_NO_THROW(uvw::Utilities::loadAverage());
    ASSERT_NE(uvw::Utilities::totalMemory(), decltype(uvw::Utilities::totalMemory()){0});
    ASSERT_NE(uvw::Utilities::uptime(), decltype(uvw::Utilities::uptime()){0});
    ASSERT_NO_THROW(uvw::Utilities::rusage());
    ASSERT_NE(uvw::Utilities::hrtime(), decltype(uvw::Utilities::hrtime()){0});
    ASSERT_FALSE(uvw::Utilities::path().empty());
    ASSERT_FALSE(uvw::Utilities::cwd().empty());
    ASSERT_TRUE(uvw::Utilities::chdir(uvw::Utilities::cwd()));

    std::unique_ptr<char[], void(*)(void *)> fake{new char[1], [](void *ptr) { delete[] static_cast<char *>(ptr); }};
    char *argv = fake.get();
    argv[0] = '\0';

    ASSERT_NE(uvw::Utilities::setupArgs(1, &argv), nullptr);
    ASSERT_NE(uvw::Utilities::processTitle(), std::string{});
    ASSERT_TRUE(uvw::Utilities::processTitle(uvw::Utilities::processTitle()));
}
