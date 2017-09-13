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
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::TcpHandle>();
    auto client = loop->resource<uvw::TcpHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::TcpHandle &handle) {
        std::shared_ptr<uvw::TcpHandle> socket = handle.loop().resource<uvw::TcpHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TcpHandle &) { FAIL(); });
        socket->on<uvw::CloseEvent>([&handle](const uvw::CloseEvent &, uvw::TcpHandle &) { handle.close(); });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TcpHandle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();
    });

    client->once<uvw::WriteEvent>([](const uvw::WriteEvent &, uvw::TcpHandle &handle) {
        handle.close();
    });

    client->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::TcpHandle &handle) {
        ASSERT_TRUE(handle.writable());
        ASSERT_TRUE(handle.readable());

        auto dataTryWrite = std::unique_ptr<char[]>(new char[1]{ 'a' });
        handle.tryWrite(std::move(dataTryWrite), 1);
        auto dataWrite = std::unique_ptr<char[]>(new char[2]{ 'b', 'c' });
        handle.write(std::move(dataWrite), 2);
    });

    server->bind(address, port);
    server->listen();
    client->connect(address, port);

    loop->run();
}


TEST(Tcp, SockPeer) {
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::TcpHandle>();
    auto client = loop->resource<uvw::TcpHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::ListenEvent>([&address, port](const uvw::ListenEvent &, uvw::TcpHandle &handle) {
        std::shared_ptr<uvw::TcpHandle> socket = handle.loop().resource<uvw::TcpHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TcpHandle &) { FAIL(); });
        socket->on<uvw::CloseEvent>([&handle](const uvw::CloseEvent &, uvw::TcpHandle &) { handle.close(); });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TcpHandle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();

        uvw::Addr addr = handle.sock();

        ASSERT_EQ(addr.ip, address);
        ASSERT_EQ(addr.port, decltype(addr.port){port});
    });

    client->once<uvw::ConnectEvent>([&address](const uvw::ConnectEvent &, uvw::TcpHandle &handle) {
        uvw::Addr addr = handle.peer();

        ASSERT_EQ(addr.ip, address);
        ASSERT_NE(addr.port, decltype(addr.port){0});

        handle.close();
    });

    server->bind(uvw::Addr{ address, port });
    server->listen();
    client->connect(uvw::Addr{ address, port });

    loop->run();
}


TEST(Tcp, Shutdown) {
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::TcpHandle>();
    auto client = loop->resource<uvw::TcpHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::TcpHandle &handle) {
        std::shared_ptr<uvw::TcpHandle> socket = handle.loop().resource<uvw::TcpHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TcpHandle &) { FAIL(); });
        socket->on<uvw::CloseEvent>([&handle](const uvw::CloseEvent &, uvw::TcpHandle &) { handle.close(); });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TcpHandle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();
    });

    client->once<uvw::ShutdownEvent>([](const uvw::ShutdownEvent &, uvw::TcpHandle &handle) {
        handle.close();
    });

    client->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::TcpHandle &handle) {
        handle.shutdown();
    });

    server->bind(address, port);
    server->listen();
    client->connect(address, port);

    loop->run();
}


TEST(Tcp, WriteError) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::TcpHandle>();

    bool checkWriteSmartPtrErrorEvent = false;
    bool checkWriteNakedPtrErrorEvent = false;
    bool checkTryWriteSmartPtrErrorEvent = false;
    bool checkTryWriteNakedPtrErrorEvent = false;

    handle->close();
    handle->once<uvw::ErrorEvent>([&checkWriteSmartPtrErrorEvent](const auto &, auto &) { checkWriteSmartPtrErrorEvent = true; });
    handle->write(std::unique_ptr<char[]>{}, 0);
    handle->once<uvw::ErrorEvent>([&checkWriteNakedPtrErrorEvent](const auto &, auto &) { checkWriteNakedPtrErrorEvent = true; });
    handle->write(nullptr, 0);
    handle->once<uvw::ErrorEvent>([&checkTryWriteSmartPtrErrorEvent](const auto &, auto &) { checkTryWriteSmartPtrErrorEvent = true; });
    handle->tryWrite(std::unique_ptr<char[]>{}, 0);
    handle->once<uvw::ErrorEvent>([&checkTryWriteNakedPtrErrorEvent](const auto &, auto &) { checkTryWriteNakedPtrErrorEvent = true; });
    handle->tryWrite(nullptr, 0);

    loop->run();

    ASSERT_TRUE(checkWriteSmartPtrErrorEvent);
    ASSERT_TRUE(checkWriteNakedPtrErrorEvent);
    ASSERT_TRUE(checkTryWriteSmartPtrErrorEvent);
    ASSERT_TRUE(checkTryWriteNakedPtrErrorEvent);
}
