#include <gtest/gtest.h>
#include <uvw/tcp.h>

TEST(TCP, Functionalities) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::TCPHandle>();

    ASSERT_TRUE(handle->noDelay(true));
    ASSERT_TRUE(handle->keepAlive(true, uvw::TCPHandle::Time{128}));
    ASSERT_TRUE(handle->simultaneousAccepts());

    handle->close();
    loop->run();
}

TEST(TCP, ReadWrite) {
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::TCPHandle>();
    auto client = loop->resource<uvw::TCPHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::TCPHandle &handle) {
        std::shared_ptr<uvw::TCPHandle> socket = handle.loop().resource<uvw::TCPHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TCPHandle &) { FAIL(); });
        socket->on<uvw::CloseEvent>([&handle](const uvw::CloseEvent &, uvw::TCPHandle &) { handle.close(); });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TCPHandle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();
    });

    client->once<uvw::WriteEvent>([](const uvw::WriteEvent &, uvw::TCPHandle &handle) {
        handle.close();
    });

    client->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::TCPHandle &handle) {
        ASSERT_TRUE(handle.writable());
        ASSERT_TRUE(handle.readable());

        auto dataTryWrite = std::unique_ptr<char[]>(new char[1]{'a'});
        handle.tryWrite(std::move(dataTryWrite), 1);
        auto dataWrite = std::unique_ptr<char[]>(new char[2]{'b', 'c'});
        handle.write(std::move(dataWrite), 2);
    });

    server->bind(address, port);
    server->listen();
    client->connect(address, port);

    loop->run();
}

TEST(TCP, SockPeer) {
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::TCPHandle>();
    auto client = loop->resource<uvw::TCPHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::ListenEvent>([&address](const uvw::ListenEvent &, uvw::TCPHandle &handle) {
        std::shared_ptr<uvw::TCPHandle> socket = handle.loop().resource<uvw::TCPHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TCPHandle &) { FAIL(); });
        socket->on<uvw::CloseEvent>([&handle](const uvw::CloseEvent &, uvw::TCPHandle &) { handle.close(); });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TCPHandle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();

        uvw::Addr addr = handle.sock();

        ASSERT_EQ(addr.ip, address);
    });

    client->once<uvw::ConnectEvent>([&address](const uvw::ConnectEvent &, uvw::TCPHandle &handle) {
        uvw::Addr addr = handle.peer();

        ASSERT_EQ(addr.ip, address);

        handle.close();
    });

    server->bind(uvw::Addr{address, port});
    server->listen();
    client->connect(uvw::Addr{address, port});

    loop->run();
}

TEST(TCP, Shutdown) {
    const std::string address = std::string{"127.0.0.1"};
    const unsigned int port = 4242;

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::TCPHandle>();
    auto client = loop->resource<uvw::TCPHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::TCPHandle &handle) {
        std::shared_ptr<uvw::TCPHandle> socket = handle.loop().resource<uvw::TCPHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TCPHandle &) { FAIL(); });
        socket->on<uvw::CloseEvent>([&handle](const uvw::CloseEvent &, uvw::TCPHandle &) { handle.close(); });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TCPHandle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();
    });

    client->once<uvw::ShutdownEvent>([](const uvw::ShutdownEvent &, uvw::TCPHandle &handle) {
        handle.close();
    });

    client->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::TCPHandle &handle) {
        handle.shutdown();
    });

    server->bind(address, port);
    server->listen();
    client->connect(address, port);

    loop->run();
}

TEST(TCP, WriteError) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::TCPHandle>();

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
