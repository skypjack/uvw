#include <cstdlib>
#include <memory>
#include <gtest/gtest.h>
#include <uvw.hpp>

template<typename T>
struct tag {
    using type = T;
};

TEST(Util, Utilities) {
    ASSERT_EQ(uvw::pid_type{}, uvw::pid_type{});

    ASSERT_NE(uvw::utilities::os::pid(), uvw::pid_type{});
    ASSERT_NE(uvw::utilities::os::ppid(), uvw::pid_type{});
    ASSERT_FALSE(uvw::utilities::os::homedir().empty());
    ASSERT_FALSE(uvw::utilities::os::tmpdir().empty());
    ASSERT_NE(uvw::utilities::os::hostname(), "");

    ASSERT_TRUE(uvw::utilities::os::env("UVW_TEST_UTIL_UTILITIES", "TRUE"));
    ASSERT_TRUE(uvw::utilities::os::env("UVW_TEST_UTIL_UTILITIES") == "TRUE");
    ASSERT_TRUE(uvw::utilities::os::env("UVW_TEST_UTIL_UTILITIES", ""));
    ASSERT_FALSE(uvw::utilities::os::env("UVW_TEST_UTIL_UTILITIES") == "TRUE");

    auto passwd = uvw::utilities::os::passwd();

    ASSERT_TRUE(static_cast<bool>(passwd));
    ASSERT_FALSE(passwd.username().empty());
    ASSERT_FALSE(passwd.homedir().empty());
    ASSERT_NO_THROW(passwd.uid());
    ASSERT_NO_THROW(passwd.gid());

#ifndef _MSC_VER
    // libuv returns a null string for the shell on Windows
    ASSERT_FALSE(passwd.shell().empty());
#endif

    ASSERT_EQ(uvw::utilities::guess_handle(uvw::file_handle{-1}), uvw::handle_type::UNKNOWN);
    ASSERT_NE(uvw::utilities::guess_handle(uvw::std_in), uvw::handle_type::UNKNOWN);

    auto guessHandle = [](auto tag, auto type) {
        auto loop = uvw::loop::get_default();
        auto handle = loop->resource<typename decltype(tag)::type>();
        ASSERT_EQ(uvw::utilities::guess_handle(handle->category()), type);
        handle->close();
        loop->run();
    };

    guessHandle(tag<uvw::async_handle>{}, uvw::handle_type::ASYNC);
    guessHandle(tag<uvw::check_handle>{}, uvw::handle_type::CHECK);
    guessHandle(tag<uvw::fs_event_handle>{}, uvw::handle_type::FS_EVENT);
    guessHandle(tag<uvw::fs_poll_handle>{}, uvw::handle_type::FS_POLL);
    guessHandle(tag<uvw::idle_handle>{}, uvw::handle_type::IDLE);
    guessHandle(tag<uvw::pipe_handle>{}, uvw::handle_type::PIPE);
    guessHandle(tag<uvw::prepare_handle>{}, uvw::handle_type::PREPARE);
    guessHandle(tag<uvw::tcp_handle>{}, uvw::handle_type::TCP);
    guessHandle(tag<uvw::timer_handle>{}, uvw::handle_type::TIMER);
    guessHandle(tag<uvw::udp_handle>{}, uvw::handle_type::UDP);
    guessHandle(tag<uvw::signal_handle>{}, uvw::handle_type::SIGNAL);

    auto cpuInfo = uvw::utilities::cpu();

    ASSERT_NE(cpuInfo.size(), decltype(cpuInfo.size()){0});
    ASSERT_FALSE(cpuInfo[0].model.empty());
    /* returns 0 on unsupported architectures */
    ASSERT_GE(cpuInfo[0].speed, decltype(cpuInfo[0].speed){0});

    auto interfaceAddresses = uvw::utilities::interface_addresses();

    ASSERT_NE(interfaceAddresses.size(), decltype(interfaceAddresses.size()){0});
    ASSERT_FALSE(interfaceAddresses[0].name.empty());
    ASSERT_FALSE(interfaceAddresses[0].address.ip.empty());
    ASSERT_FALSE(interfaceAddresses[0].netmask.ip.empty());

    ASSERT_NO_THROW(uvw::utilities::index_to_name(0));
    ASSERT_NO_THROW(uvw::utilities::index_to_iid(0));

    ASSERT_TRUE(uvw::utilities::replace_allocator(
        [](size_t size) { return malloc(size); },
        [](void *ptr, size_t size) { return realloc(ptr, size); },
        [](size_t num, size_t size) { return calloc(num, size); },
        [](void *ptr) { return free(ptr); }));

    ASSERT_NO_THROW(uvw::utilities::load_average());
    ASSERT_NE(uvw::utilities::total_memory(), decltype(uvw::utilities::total_memory()){0});
    ASSERT_NE(uvw::utilities::available_memory(), decltype(uvw::utilities::available_memory()){0});
    ASSERT_NE(uvw::utilities::uptime(), decltype(uvw::utilities::uptime()){0});
    ASSERT_NO_THROW(uvw::utilities::rusage());
    ASSERT_NO_THROW(uvw::utilities::gettime(uvw::clock_id::MONOTONIC));
    ASSERT_NE(uvw::utilities::hrtime(), decltype(uvw::utilities::hrtime()){0});
    ASSERT_FALSE(uvw::utilities::path().empty());
    ASSERT_FALSE(uvw::utilities::cwd().empty());
    ASSERT_TRUE(uvw::utilities::chdir(uvw::utilities::cwd()));

    std::unique_ptr<char[], void (*)(void *)> fake{new char[1], [](void *ptr) { delete[] static_cast<char *>(ptr); }};
    char *argv = fake.get();
    argv[0] = '\0';

    ASSERT_NE(uvw::utilities::setup_args(1, &argv), nullptr);
    ASSERT_NE(uvw::utilities::process_title(), std::string{});
    ASSERT_TRUE(uvw::utilities::process_title(uvw::utilities::process_title()));

    ASSERT_NE(uvw::utilities::available_parallelism(), 0u);
}
