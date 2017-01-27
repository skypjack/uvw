#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Tcp, Functionalities) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::TcpHandle>();

    ASSERT_TRUE(handle->noDelay(true));
    ASSERT_TRUE(handle->keepAlive(true, uvw::TcpHandle::Time{128}));
    ASSERT_TRUE(handle->simultaneousAccepts());

    handle->close();
    loop->run();
}


TEST(Tcp, ReadWrite) {
    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::TcpHandle>();
    auto client = loop->resource<uvw::TcpHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::TcpHandle &srv) {
        std::shared_ptr<uvw::TcpHandle> socket = srv.loop().resource<uvw::TcpHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TcpHandle &) { FAIL(); });
        socket->on<uvw::CloseEvent>([&srv](const uvw::CloseEvent &, uvw::TcpHandle &) { srv.close(); });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TcpHandle &sock) { sock.close(); });

        srv.accept(*socket);
        socket->read();
    });

    client->once<uvw::WriteEvent>([](const uvw::WriteEvent &, uvw::TcpHandle &handle) {
        handle.close();
    });

    client->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::TcpHandle &handle) {
        auto dataTryWrite = std::unique_ptr<char[]>(new char[1]{ 'a' });
        handle.tryWrite(std::move(dataTryWrite), 1);
        auto dataWrite = std::unique_ptr<char[]>(new char[2]{ 'b', 'c' });
        handle.write(std::move(dataWrite), 2);
    });

    server->bind("127.0.0.1", 4242);
    server->listen();
    client->connect("127.0.0.1", 4242);

    loop->run();
}


TEST(Tcp, SockPeer) {
    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::TcpHandle>();
    auto client = loop->resource<uvw::TcpHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::TcpHandle &srv) {
        std::shared_ptr<uvw::TcpHandle> socket = srv.loop().resource<uvw::TcpHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TcpHandle &) { FAIL(); });
        socket->on<uvw::CloseEvent>([&srv](const uvw::CloseEvent &, uvw::TcpHandle &) { srv.close(); });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TcpHandle &sock) { sock.close(); });

        srv.accept(*socket);
        socket->read();

        uvw::Addr local = srv.sock();
        uvw::Addr remote = socket->peer();

        ASSERT_EQ(local.ip, "127.0.0.1");
        ASSERT_EQ(remote.ip, "127.0.0.1");
        ASSERT_EQ(local.port, decltype(local.port){4242});
        ASSERT_NE(remote.port, decltype(remote.port){0});
    });

    client->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::TcpHandle &handle) {
        handle.close();
    });

    server->bind("127.0.0.1", 4242);
    server->listen();
    client->connect("127.0.0.1", 4242);

    loop->run();
}
