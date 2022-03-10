#include <gtest/gtest.h>
#include <uvw/dns.h>

TEST(GetAddrInfo, GetNodeAddrInfo) {
    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::get_addr_info_req>();

    bool checkAddrInfoEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::addr_info_event>([&checkAddrInfoEvent](const auto &, auto &) {
        ASSERT_FALSE(checkAddrInfoEvent);
        checkAddrInfoEvent = true;
    });

    request->node_addr_info("irc.freenode.net");

    loop->run();

    ASSERT_TRUE(checkAddrInfoEvent);
}

TEST(GetAddrInfo, GetNodeAddrInfoSync) {
    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::get_addr_info_req>();

    ASSERT_TRUE(request->node_addr_info_sync("irc.freenode.net").first);
    ASSERT_FALSE(request->node_addr_info_sync("").first);

    loop->run();
}

TEST(GetAddrInfo, GetServiceAddrInfo) {
    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::get_addr_info_req>();

    bool checkErrorEvent = false;

    request->on<uvw::error_event>([&checkErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkErrorEvent);
        checkErrorEvent = true;
    });

    request->service_addr_info("foobar");

    loop->run();

    ASSERT_TRUE(checkErrorEvent);
}

TEST(GetAddrInfo, GetServiceAddrInfoSync) {
    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::get_addr_info_req>();

    ASSERT_FALSE(request->service_addr_info_sync("foobar").first);

    loop->run();
}

TEST(GetAddrInfo, GetAddrInfo) {
    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::get_addr_info_req>();

    bool checkAddrInfoEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::addr_info_event>([&checkAddrInfoEvent](const auto &, auto &) {
        ASSERT_FALSE(checkAddrInfoEvent);
        checkAddrInfoEvent = true;
    });

    request->addr_info("irc.freenode.net", "6667");

    loop->run();

    ASSERT_TRUE(checkAddrInfoEvent);
}

TEST(GetAddrInfo, GetAddrInfoSync) {
    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::get_addr_info_req>();

    ASSERT_TRUE(request->addr_info_sync("irc.freenode.net", "6667").first);
    ASSERT_FALSE(request->addr_info_sync("", "").first);

    loop->run();
}

TEST(GetNameInfo, GetNameInfo) {
    auto loop = uvw::loop::get_default();
    auto koRequest = loop->resource<uvw::get_name_info_req>();
    auto okRequest = loop->resource<uvw::get_name_info_req>();

    bool checkErrorEvent = false;
    bool checkNameInfoEvent = false;

    koRequest->on<uvw::error_event>([&checkErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkErrorEvent);
        checkErrorEvent = true;
    });

    okRequest->on<uvw::name_info_event>([&checkNameInfoEvent](const auto &, auto &) {
        ASSERT_FALSE(checkNameInfoEvent);
        checkNameInfoEvent = true;
    });

    koRequest->name_info(uvw::socket_address{"", 0}, -1);
    okRequest->name_info("irc.freenode.net", 6667);

    loop->run();

    ASSERT_TRUE(checkErrorEvent);
    ASSERT_TRUE(checkNameInfoEvent);
}

TEST(GetNameInfo, GetNameInfoSync) {
    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::get_name_info_req>();

    ASSERT_FALSE(request->name_info_sync(uvw::socket_address{"", 0}, -1).first);
    ASSERT_TRUE(request->name_info_sync("irc.freenode.net", 6667).first);

    loop->run();
}
