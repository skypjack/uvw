#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Udp, Functionalities) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::UDPHandle>();

    ASSERT_FALSE(handle->multicastMembership("0.0.0.0", "127.0.0.1", uvw::UDPHandle::Membership::JOIN_GROUP));
    ASSERT_TRUE(handle->multicastMembership("224.0.0.1", "127.0.0.1", uvw::UDPHandle::Membership::JOIN_GROUP));
    ASSERT_TRUE(handle->multicastMembership("224.0.0.1", "127.0.0.1", uvw::UDPHandle::Membership::LEAVE_GROUP));
    ASSERT_TRUE(handle->multicastLoop(true));
    ASSERT_TRUE(handle->multicastTtl(42));
    ASSERT_TRUE(handle->multicastInterface("127.0.0.1"));
    ASSERT_TRUE(handle->broadcast(true));
    ASSERT_TRUE(handle->ttl(42));
    ASSERT_FALSE(handle->ttl(0));

    handle->close();
    loop->run();
}


TEST(Udp, BindRecvStop) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::UDPHandle>();

    handle->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    handle->bind("127.0.0.1", 4242);
    handle->recv();
    handle->stop();
    handle->close();

    loop->run();
}


TEST(Udp, ReadTrySend) {
    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::UDPHandle>();
    auto client = loop->resource<uvw::UDPHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::UDPDataEvent>([&client](const uvw::UDPDataEvent &, uvw::UDPHandle &handle) {
        client->close();
        handle.close();
    });

    server->bind("127.0.0.1", 4242);
    server->recv();

    auto dataTrySend = std::unique_ptr<char[]>(new char[1]{ 'a' });
    client->trySend("127.0.0.1", 4242, std::move(dataTrySend), 1);

    loop->run();
}


TEST(Udp, ReadSend) {
    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::UDPHandle>();
    auto client = loop->resource<uvw::UDPHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::UDPDataEvent>([](const uvw::UDPDataEvent &, uvw::UDPHandle &handle) {
        handle.close();
    });

    client->once<uvw::SendEvent>([](const uvw::SendEvent &, uvw::UDPHandle &handle) {
        handle.close();
    });

    server->bind("127.0.0.1", 4242);
    server->recv();

    auto dataSend = std::unique_ptr<char[]>(new char[2]{ 'b', 'c' });
    client->send(uvw::Addr{ "127.0.0.1", 4242 }, std::move(dataSend), 1);

    loop->run();
}


TEST(Udp, Sock) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::UDPHandle>();

    handle->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    handle->bind("127.0.0.1", 4242);
    handle->recv();

    uvw::Addr sock = handle->sock();
    ASSERT_EQ(sock.ip, "127.0.0.1");
    ASSERT_EQ(sock.port, decltype(sock.port){4242});

    handle->close();
    loop->run();
}
