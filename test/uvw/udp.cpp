#include <gtest/gtest.h>
#include <uvw/udp.h>

namespace {

auto custom_alloc_callback(const uvw::udp_handle &, std::size_t suggested) {
    return std::make_pair(new char[suggested], suggested);
}

} // namespace

TEST(UDP, Functionalities) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::udp_handle>();

    ASSERT_FALSE(handle->multicast_membership("0.0.0.0", "127.0.0.1", uvw::udp_handle::membership::JOIN_GROUP));
    ASSERT_TRUE(handle->multicast_membership("224.0.0.1", "127.0.0.1", uvw::udp_handle::membership::JOIN_GROUP));
    ASSERT_TRUE(handle->multicast_membership("224.0.0.1", "127.0.0.1", uvw::udp_handle::membership::LEAVE_GROUP));
    ASSERT_TRUE(handle->multicast_loop(true));
    ASSERT_TRUE(handle->multicast_ttl(42));
    ASSERT_TRUE(handle->multicast_interface("127.0.0.1"));
    ASSERT_TRUE(handle->broadcast(true));
    ASSERT_TRUE(handle->ttl(42));
    ASSERT_FALSE(handle->ttl(0));

    handle->close();
    loop->run();
}

TEST(UDP, BindRecvStop) {
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::udp_handle>();

    handle->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    ASSERT_EQ(0, (handle->bind(address, port)));
    ASSERT_EQ(0, handle->recv());
    ASSERT_EQ(0, handle->stop());

    handle->close();

    loop->run();
}

TEST(UDP, ReadTrySend) {
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::loop::get_default();
    auto server = loop->resource<uvw::udp_handle>();
    auto client = loop->resource<uvw::udp_handle>();

    server->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    server->on<uvw::udp_data_event>([&client](const uvw::udp_data_event &, uvw::udp_handle &handle) {
        client->close();
        handle.close();
    });

    ASSERT_EQ(0, (server->bind(uvw::socket_address{address, port})));
    ASSERT_EQ(0, server->recv());

    auto dataTrySend = std::unique_ptr<char[]>(new char[1]{'a'});

    ASSERT_EQ(1, client->try_send(uvw::socket_address{address, port}, dataTrySend.get(), 1));
    ASSERT_EQ(0, client->try_send(address, port, nullptr, 0));

    ASSERT_EQ(1, client->try_send(uvw::socket_address{address, port}, std::move(dataTrySend), 1));
    ASSERT_EQ(0, client->try_send(address, port, std::unique_ptr<char[]>{}, 0));

    loop->run();
}

TEST(UDP, ReadSend) {
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::loop::get_default();
    auto server = loop->resource<uvw::udp_handle>();
    auto client = loop->resource<uvw::udp_handle>();

    server->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    server->on<uvw::udp_data_event>([](const uvw::udp_data_event &, uvw::udp_handle &handle) {
        handle.close();
    });

    client->on<uvw::send_event>([](const uvw::send_event &, uvw::udp_handle &handle) {
        handle.close();
    });

    ASSERT_EQ(0, (server->bind(address, port)));
    ASSERT_EQ(0, server->recv());

    auto dataSend = std::unique_ptr<char[]>(new char[2]{'b', 'c'});

    client->send(uvw::socket_address{address, port}, dataSend.get(), 2);
    client->send(address, port, nullptr, 0);

    client->send(uvw::socket_address{address, port}, std::move(dataSend), 2);
    client->send(address, port, std::unique_ptr<char[]>{}, 0);

    loop->run();
}

TEST(UDP, ReadSendCustomAlloc) {
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::loop::get_default();
    auto server = loop->resource<uvw::udp_handle>();
    auto client = loop->resource<uvw::udp_handle>();

    server->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    server->on<uvw::udp_data_event>([](const uvw::udp_data_event &, uvw::udp_handle &handle) {
        handle.close();
    });

    client->on<uvw::send_event>([](const uvw::send_event &, uvw::udp_handle &handle) {
        handle.close();
    });

    ASSERT_EQ(0, (server->bind(address, port)));
    ASSERT_EQ(0, server->recv<&custom_alloc_callback>());

    auto dataSend = std::unique_ptr<char[]>(new char[3]{'b', 'c'});

    client->send(uvw::socket_address{address, port}, dataSend.get(), 2);
    client->send(address, port, nullptr, 0);

    client->send(uvw::socket_address{address, port}, std::move(dataSend), 2);
    client->send(address, port, std::unique_ptr<char[]>{}, 0);

    loop->run();
}

TEST(UDP, Sock) {
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::udp_handle>();

    handle->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    handle->bind(address, port);
    handle->recv();

    const uvw::socket_address sock = handle->sock();

    ASSERT_EQ(sock.ip, address);
    ASSERT_EQ(sock.port, decltype(sock.port){port});

    handle->close();
    loop->run();
}
