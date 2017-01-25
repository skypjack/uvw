#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(GetAddrInfo, GetNodeAddrInfo) {
    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::GetAddrInfoReq>();

    bool checkAddrInfoEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::AddrInfoEvent>([&checkAddrInfoEvent](const auto &, auto &) {
        ASSERT_FALSE(checkAddrInfoEvent);
        checkAddrInfoEvent = true;
    });

    request->getNodeAddrInfo("irc.freenode.net");

    loop->run();

    ASSERT_TRUE(checkAddrInfoEvent);
}


TEST(GetAddrInfo, GetNodeAddrInfoSync) {
    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::GetAddrInfoReq>();

    ASSERT_TRUE(request->getNodeAddrInfoSync("irc.freenode.net").first);
    ASSERT_FALSE(request->getNodeAddrInfoSync("net.freenode.irc").first);

    loop->run();
}


TEST(GetAddrInfo, GetServiceAddrInfo) {
    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::GetAddrInfoReq>();

    bool checkErrorEvent = false;

    request->on<uvw::ErrorEvent>([&checkErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkErrorEvent);
        checkErrorEvent = true;
    });

    request->getServiceAddrInfo("foobar");

    loop->run();

    ASSERT_TRUE(checkErrorEvent);
}


TEST(GetAddrInfo, GetServiceAddrInfoSync) {
    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::GetAddrInfoReq>();

    ASSERT_FALSE(request->getServiceAddrInfoSync("foobar").first);

    loop->run();
}


TEST(GetAddrInfo, GetAddrInfo) {
    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::GetAddrInfoReq>();

    bool checkAddrInfoEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::AddrInfoEvent>([&checkAddrInfoEvent](const auto &, auto &) {
        ASSERT_FALSE(checkAddrInfoEvent);
        checkAddrInfoEvent = true;
    });

    request->getAddrInfo("irc.freenode.net", "6667");

    loop->run();

    ASSERT_TRUE(checkAddrInfoEvent);
}


TEST(GetAddrInfo, GetAddrInfoSync) {
    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::GetAddrInfoReq>();

    ASSERT_TRUE(request->getAddrInfoSync("irc.freenode.net", "6667").first);
    ASSERT_FALSE(request->getAddrInfoSync("net.freenode.irc", "6667").first);

    loop->run();
}


TEST(GetNameInfo, GetNameInfo) {
    auto loop = uvw::Loop::getDefault();
    auto koRequest = loop->resource<uvw::GetNameInfoReq>();
    auto okRequest = loop->resource<uvw::GetNameInfoReq>();

    bool checkErrorEvent = false;
    bool checkNameInfoEvent = false;

    koRequest->on<uvw::NameInfoEvent>([&checkErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkErrorEvent);
        checkErrorEvent = true;
    });

    okRequest->on<uvw::NameInfoEvent>([&checkNameInfoEvent](const auto &, auto &) {
        ASSERT_FALSE(checkNameInfoEvent);
        checkNameInfoEvent = true;
    });

    koRequest->getNameInfo(uvw::Addr{"", 6667});
    okRequest->getNameInfo("irc.freenode.net", 6667);

    loop->run();

    ASSERT_TRUE(checkErrorEvent);
    ASSERT_TRUE(checkNameInfoEvent);
}


TEST(GetNameInfo, GetNameInfoSync) {
    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::GetNameInfoReq>();

    ASSERT_TRUE(request->getNameInfoSync("irc.freenode.net", 6667).first);

    loop->run();
}
