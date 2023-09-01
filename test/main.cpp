#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <uvw.hpp>

void listen(uvw::loop &loop) {
    std::shared_ptr<uvw::tcp_handle> tcp = loop.resource<uvw::tcp_handle>();
    tcp->on<uvw::error_event>([](const uvw::error_event &, uvw::tcp_handle &) { assert(false); });

    tcp->on<uvw::listen_event>([](const uvw::listen_event &, uvw::tcp_handle &srv) {
        std::cout << "listen" << std::endl;

        std::shared_ptr<uvw::tcp_handle> client = srv.parent().resource<uvw::tcp_handle>();
        client->on<uvw::error_event>([](const uvw::error_event &, uvw::tcp_handle &) { assert(false); });

        client->on<uvw::close_event>([ptr = srv.shared_from_this()](const uvw::close_event &, uvw::tcp_handle &) {
            std::cout << "close" << std::endl;
            ptr->close();
        });

        srv.accept(*client);

        uvw::socket_address local = srv.sock();
        std::cout << "local: " << local.ip << " " << local.port << std::endl;

        uvw::socket_address remote = client->peer();
        std::cout << "remote: " << remote.ip << " " << remote.port << std::endl;

        client->on<uvw::data_event>([](const uvw::data_event &event, uvw::tcp_handle &) {
            std::cout.write(event.data.get(), static_cast<std::streamsize>(event.length)) << std::endl;
            std::cout << "data length: " << event.length << std::endl;
        });

        client->on<uvw::end_event>([](const uvw::end_event &, uvw::tcp_handle &handle) {
            std::cout << "end" << std::endl;
            int count = 0;
            handle.parent().walk([&count](auto &) { ++count; });
            std::cout << "still alive: " << count << " handles" << std::endl;
            handle.close();
        });

        client->read();
    });

    tcp->on<uvw::close_event>([](const uvw::close_event &, uvw::tcp_handle &) {
        std::cout << "close" << std::endl;
    });

    tcp->bind("127.0.0.1", 4242);
    tcp->listen();
}

void conn(uvw::loop &loop) {
    auto tcp = loop.resource<uvw::tcp_handle>();
    tcp->on<uvw::error_event>([](const uvw::error_event &, uvw::tcp_handle &) { assert(false); });

    tcp->on<uvw::write_event>([](const uvw::write_event &, uvw::tcp_handle &handle) {
        std::cout << "write" << std::endl;
        handle.close();
    });

    tcp->on<uvw::connect_event>([](const uvw::connect_event &, uvw::tcp_handle &handle) {
        std::cout << "connect" << std::endl;

        auto dataTryWrite = std::unique_ptr<char[]>(new char[1]{'a'});
        int bw = handle.try_write(std::move(dataTryWrite), 1);
        std::cout << "written: " << static_cast<int>(bw) << std::endl;

        auto dataWrite = std::unique_ptr<char[]>(new char[2]{'b', 'c'});
        handle.write(std::move(dataWrite), 2);
    });

    tcp->on<uvw::close_event>([](const uvw::close_event &, uvw::tcp_handle &) {
        std::cout << "close" << std::endl;
    });

    tcp->connect("127.0.0.1", 4242);
}

void g() {
    auto loop = uvw::loop::get_default();
    listen(*loop);
    conn(*loop);
    loop->run();
    loop = nullptr;
}

int main() {
    g();
}
