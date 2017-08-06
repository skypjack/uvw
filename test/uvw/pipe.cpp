#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Pipe, ReadWrite) {
    const std::string sockname = std::string{TARGET_PIPE_DIR} + std::string{"/test.sock"};

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::PipeHandle>();
    auto client = loop->resource<uvw::PipeHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::PipeHandle &handle) {
        std::shared_ptr<uvw::PipeHandle> socket = handle.loop().resource<uvw::PipeHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::PipeHandle &) { FAIL(); });
        socket->on<uvw::CloseEvent>([&handle](const uvw::CloseEvent &, uvw::PipeHandle &) { handle.close(); });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();
    });

    client->once<uvw::WriteEvent>([](const uvw::WriteEvent &, uvw::PipeHandle &handle) {
        handle.close();
    });

    client->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::PipeHandle &handle) {
        ASSERT_TRUE(handle.writable());
        ASSERT_TRUE(handle.readable());

        auto dataTryWrite = std::unique_ptr<char[]>(new char[1]{ 'a' });
        handle.tryWrite(std::move(dataTryWrite), 1);
        auto dataWrite = std::unique_ptr<char[]>(new char[2]{ 'b', 'c' });
        handle.write(std::move(dataWrite), 2);
    });

    server->bind(sockname);
    server->listen();
    client->connect(sockname);

    loop->run();
}


TEST(Pipe, SockPeer) {
    const std::string sockname = std::string{TARGET_PIPE_DIR} + std::string{"/test.sock"};

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::PipeHandle>();
    auto client = loop->resource<uvw::PipeHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::ListenEvent>([&sockname](const uvw::ListenEvent &, uvw::PipeHandle &handle) {
        std::shared_ptr<uvw::PipeHandle> socket = handle.loop().resource<uvw::PipeHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::PipeHandle &) { FAIL(); });
        socket->on<uvw::CloseEvent>([&handle](const uvw::CloseEvent &, uvw::PipeHandle &) { handle.close(); });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();

        ASSERT_EQ(handle.sock(), sockname);
    });

    client->once<uvw::ConnectEvent>([&sockname](const uvw::ConnectEvent &, uvw::PipeHandle &handle) {
        ASSERT_EQ(handle.peer(), sockname);

        handle.close();
    });

    server->bind(sockname);
    server->listen();
    client->connect(sockname);

    loop->run();
}


TEST(Pipe, Shutdown) {
    const std::string sockname = std::string{TARGET_PIPE_DIR} + std::string{"/test.sock"};
    auto data = std::unique_ptr<char[]>(new char[3]{ 'a', 'b', 'c' });

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::PipeHandle>();
    auto client = loop->resource<uvw::PipeHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::PipeHandle &handle) {
        std::shared_ptr<uvw::PipeHandle> socket = handle.loop().resource<uvw::PipeHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::PipeHandle &) { FAIL(); });
        socket->on<uvw::CloseEvent>([&handle](const uvw::CloseEvent &, uvw::PipeHandle &) { handle.close(); });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();
    });

    client->once<uvw::ShutdownEvent>([](const uvw::ShutdownEvent &, uvw::PipeHandle &handle) {
        handle.close();
    });

    client->once<uvw::ConnectEvent>([&data](const uvw::ConnectEvent &, uvw::PipeHandle &handle) {
        handle.write(data.get(), 3);
        handle.shutdown();
    });

    server->bind(sockname);
    server->listen();
    client->connect(sockname);

    loop->run();
}


TEST(Pipe, TODO) {
    auto loop = uvw::Loop::getDefault();
    auto handle = uvw::PipeHandle::create(loop);

    handle = nullptr;

    // TODO
}
