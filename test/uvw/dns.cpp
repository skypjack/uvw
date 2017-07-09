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

    request->nodeAddrInfo("irc.freenode.net");

    loop->run();

    ASSERT_TRUE(checkAddrInfoEvent);
}


TEST(GetAddrInfo, GetNodeAddrInfoSync) {
    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::GetAddrInfoReq>();

    ASSERT_TRUE(request->nodeAddrInfoSync("irc.freenode.net").first);
    ASSERT_FALSE(request->nodeAddrInfoSync("").first);

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

    request->serviceAddrInfo("foobar");

    loop->run();

    ASSERT_TRUE(checkErrorEvent);
}


TEST(GetAddrInfo, GetServiceAddrInfoSync) {
    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::GetAddrInfoReq>();

    ASSERT_FALSE(request->serviceAddrInfoSync("foobar").first);

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

    request->addrInfo("irc.freenode.net", "6667");

    loop->run();

    ASSERT_TRUE(checkAddrInfoEvent);
}


TEST(GetAddrInfo, GetAddrInfoSync) {
    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::GetAddrInfoReq>();

    ASSERT_TRUE(request->addrInfoSync("irc.freenode.net", "6667").first);
    ASSERT_FALSE(request->addrInfoSync("", "").first);

    loop->run();
}


TEST(GetNameInfo, GetNameInfo) {
    auto loop = uvw::Loop::getDefault();
    auto koRequest = loop->resource<uvw::GetNameInfoReq>();
    auto okRequest = loop->resource<uvw::GetNameInfoReq>();

    bool checkErrorEvent = false;
    bool checkNameInfoEvent = false;

    koRequest->on<uvw::ErrorEvent>([&checkErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkErrorEvent);
        checkErrorEvent = true;
    });

    okRequest->on<uvw::NameInfoEvent>([&checkNameInfoEvent](const auto &, auto &) {
        ASSERT_FALSE(checkNameInfoEvent);
        checkNameInfoEvent = true;
    });

    koRequest->nameInfo(uvw::Addr{ "", 0 }, -1);
    okRequest->nameInfo("irc.freenode.net", 6667);

    loop->run();

    ASSERT_TRUE(checkErrorEvent);
    ASSERT_TRUE(checkNameInfoEvent);
}


TEST(GetNameInfo, GetNameInfoSync) {
    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::GetNameInfoReq>();

    ASSERT_FALSE(request->nameInfoSync(uvw::Addr{ "", 0 }, -1).first);
    ASSERT_TRUE(request->nameInfoSync("irc.freenode.net", 6667).first);

    loop->run();
}
