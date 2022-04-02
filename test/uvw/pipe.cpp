#include <gtest/gtest.h>
#include <uvw/pipe.h>

TEST(Pipe, ReadWrite) {
#ifdef _MSC_VER
    const std::string sockname{"\\\\.\\pipe\\test.sock"};
#else
    const std::string sockname = std::string{TARGET_PIPE_DIR} + std::string{"/test.sock"};
#endif

    auto loop = uvw::loop::get_default();
    auto server = loop->resource<uvw::pipe_handle>();
    auto client = loop->resource<uvw::pipe_handle>();

    server->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::error_event>([](const auto &, auto &) {
        FAIL();
    });

    server->on<uvw::listen_event>([](const uvw::listen_event &, uvw::pipe_handle &handle) {
        std::shared_ptr<uvw::pipe_handle> socket = handle.parent().resource<uvw::pipe_handle>();

        socket->on<uvw::error_event>([](const uvw::error_event &, uvw::pipe_handle &) { FAIL(); });
        socket->on<uvw::close_event>([&handle](const uvw::close_event &, uvw::pipe_handle &) { handle.close(); });
        socket->on<uvw::end_event>([](const uvw::end_event &, uvw::pipe_handle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();
    });

    client->on<uvw::write_event>([](const uvw::write_event &, uvw::pipe_handle &handle) {
        handle.close();
    });

    client->on<uvw::connect_event>([](const uvw::connect_event &, uvw::pipe_handle &handle) {
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

    auto loop = uvw::loop::get_default();
    auto server = loop->resource<uvw::pipe_handle>();
    auto client = loop->resource<uvw::pipe_handle>();

    server->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    server->on<uvw::listen_event>([&peername](const uvw::listen_event &, uvw::pipe_handle &handle) {
        std::shared_ptr<uvw::pipe_handle> socket = handle.parent().resource<uvw::pipe_handle>();

        socket->on<uvw::error_event>([](const uvw::error_event &, uvw::pipe_handle &) { FAIL(); });
        socket->on<uvw::close_event>([&handle](const uvw::close_event &, uvw::pipe_handle &) { handle.close(); });
        socket->on<uvw::end_event>([](const uvw::end_event &, uvw::pipe_handle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();

        ASSERT_EQ(handle.sock(), peername);
    });

    client->on<uvw::connect_event>([&peername](const uvw::connect_event &, uvw::pipe_handle &handle) {
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

    auto loop = uvw::loop::get_default();
    auto server = loop->resource<uvw::pipe_handle>();
    auto client = loop->resource<uvw::pipe_handle>();

    server->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    client->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    server->on<uvw::listen_event>([](const uvw::listen_event &, uvw::pipe_handle &handle) {
        std::shared_ptr<uvw::pipe_handle> socket = handle.parent().resource<uvw::pipe_handle>();

        socket->on<uvw::error_event>([](const uvw::error_event &, uvw::pipe_handle &) { FAIL(); });
        socket->on<uvw::close_event>([&handle](const uvw::close_event &, uvw::pipe_handle &) { handle.close(); });
        socket->on<uvw::end_event>([](const uvw::end_event &, uvw::pipe_handle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();
    });

    client->on<uvw::shutdown_event>([](const uvw::shutdown_event &, uvw::pipe_handle &handle) {
        handle.close();
    });

    client->on<uvw::connect_event>([&data](const uvw::connect_event &, uvw::pipe_handle &handle) {
        handle.write(data.get(), 3);
        handle.shutdown();
    });

    server->bind(sockname);
    server->listen();
    client->connect(sockname);

    loop->run();
}
