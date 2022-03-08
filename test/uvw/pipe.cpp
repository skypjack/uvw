#include <gtest/gtest.h>
#include <uvw/pipe.h>

TEST(Pipe, ReadWrite) {
#ifdef _MSC_VER
    const std::string sockname{"\\\\.\\pipe\\test.sock"};
#else
    const std::string sockname = std::string{TARGET_PIPE_DIR} + std::string{"/test.sock"};
#endif

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::PipeHandle>();
    auto client = loop->resource<uvw::PipeHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

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

        auto dataWrite = std::unique_ptr<char[]>(new char[2]{'x', 'y'});
        handle.write(std::move(dataWrite), 2);
    });

    server->bind(sockname);
    server->listen();
    client->connect(sockname);

    loop->run();
}

TEST(Pipe, SockPeer) {
#ifdef _MSC_VER
    const std::string sockname{"\\\\.\\pipe\\test.sock"};
    const std::string peername{"\\\\?\\pipe\\test.sock"};
#else
    const std::string sockname = std::string{TARGET_PIPE_DIR} + std::string{"/test.sock"};
    const auto peername = sockname;
#endif

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::PipeHandle>();
    auto client = loop->resource<uvw::PipeHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    server->once<uvw::ListenEvent>([&peername](const uvw::ListenEvent &, uvw::PipeHandle &handle) {
        std::shared_ptr<uvw::PipeHandle> socket = handle.loop().resource<uvw::PipeHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::PipeHandle &) { FAIL(); });
        socket->on<uvw::CloseEvent>([&handle](const uvw::CloseEvent &, uvw::PipeHandle &) { handle.close(); });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();

        ASSERT_EQ(handle.sock(), peername);
    });

    client->once<uvw::ConnectEvent>([&peername](const uvw::ConnectEvent &, uvw::PipeHandle &handle) {
        ASSERT_EQ(handle.peer(), peername);

        handle.close();
    });

    server->bind(sockname);
    server->listen();
    client->connect(sockname);

    loop->run();
}

TEST(Pipe, Shutdown) {
#ifdef _MSC_VER
    const std::string sockname{"\\\\.\\pipe\\test.sock"};
#else
    const std::string sockname = std::string{TARGET_PIPE_DIR} + std::string{"/test.sock"};
#endif

    auto data = std::unique_ptr<char[]>(new char[3]{'a', 'b', 'c'});

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
