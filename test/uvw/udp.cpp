#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Udp, Functionalities) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::UdpHandle>();

    ASSERT_FALSE(handle->multicastMembership("0.0.0.0", "127.0.0.1", uvw::UdpHandle::Membership::JOIN_GROUP));
    ASSERT_TRUE(handle->multicastMembership("224.0.0.1", "127.0.0.1", uvw::UdpHandle::Membership::JOIN_GROUP));
    ASSERT_TRUE(handle->multicastMembership("224.0.0.1", "127.0.0.1", uvw::UdpHandle::Membership::LEAVE_GROUP));
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
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::UdpHandle>();

    handle->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::UdpHandle &) { FAIL(); });

    handle->bind(address, port);
    handle->recv();
    handle->stop();
    handle->close();

    loop->run();
}


TEST(Udp, ReadTrySend) {
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::UdpHandle>();
    auto client = loop->resource<uvw::UdpHandle>();

    server->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::UdpHandle &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::UdpHandle &) { FAIL(); });

    server->once<uvw::UDPDataEvent>([&client](const uvw::UDPDataEvent &, uvw::UdpHandle &handle) {
        client->close();
        handle.close();
    });

    server->bind(uvw::Addr{ address, port });
    server->recv();

    auto dataTrySend = std::unique_ptr<char[]>(new char[1]{ 'a' });

    client->trySend(uvw::Addr{ address, port }, dataTrySend.get(), 1);
    client->trySend(address, port, nullptr, 0);

    client->trySend(uvw::Addr{ address, port }, std::move(dataTrySend), 1);
    client->trySend(address, port, std::unique_ptr<char[]>{}, 0);

    loop->run();
}


TEST(Udp, ReadSend) {
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::UdpHandle>();
    auto client = loop->resource<uvw::UdpHandle>();

    server->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::UdpHandle &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::UdpHandle &) { FAIL(); });

    server->once<uvw::UDPDataEvent>([](const uvw::UDPDataEvent &, uvw::UdpHandle &handle) {
        handle.close();
    });

    client->once<uvw::SendEvent>([](const uvw::SendEvent &, uvw::UdpHandle &handle) {
        handle.close();
    });

    server->bind(address, port);
    server->recv();

    auto dataSend = std::unique_ptr<char[]>(new char[2]{ 'b', 'c' });

    client->send(uvw::Addr{ address , port }, dataSend.get(), 2);
    client->send(address, port, nullptr, 0);

    client->send(uvw::Addr{ address , port }, std::move(dataSend), 2);
    client->send(address, port, std::unique_ptr<char[]>{}, 0);

    loop->run();
}


TEST(Udp, Sock) {
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::UdpHandle>();

    handle->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::UdpHandle &) { FAIL(); });

    handle->bind(address, port);
    handle->recv();

    uvw::Addr sock = handle->sock();
    ASSERT_EQ(sock.ip, address);
    ASSERT_EQ(sock.port, decltype(sock.port){port});

    handle->close();
    loop->run();
}
