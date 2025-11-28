#include <cassert>
#include <iostream>
#include <memory>
#include <utility>
#include <uvw.hpp>

namespace {

void listen(uvw::loop &loop) {
    const std::shared_ptr<uvw::tcp_handle> tcp = loop.resource<uvw::tcp_handle>();
    tcp->on<uvw::error_event>([](const uvw::error_event &, uvw::tcp_handle &) { assert(false); });

    tcp->on<uvw::listen_event>([](const uvw::listen_event &, uvw::tcp_handle &srv) {
        std::cout << "listen\n";

        const std::shared_ptr<uvw::tcp_handle> client = srv.parent().resource<uvw::tcp_handle>();
        client->on<uvw::error_event>([](const uvw::error_event &, uvw::tcp_handle &) { assert(false); });

        client->on<uvw::close_event>([ptr = srv.shared_from_this()](const uvw::close_event &, uvw::tcp_handle &) {
            std::cout << "close\n";
            ptr->close();
        });

        srv.accept(*client);

        const uvw::socket_address local = srv.sock();
        std::cout << "local: " << local.ip << " " << local.port << "\n";

        const uvw::socket_address remote = client->peer();
        std::cout << "remote: " << remote.ip << " " << remote.port << "\n";

        client->on<uvw::data_event>([](const uvw::data_event &event, uvw::tcp_handle &) {
            std::cout.write(event.data.get(), static_cast<std::streamsize>(event.length)) << "\n";
            std::cout << "data length: " << event.length << "\n";
        });

        client->on<uvw::end_event>([](const uvw::end_event &, uvw::tcp_handle &handle) {
            std::cout << "end\n";
            int count = 0;
            handle.parent().walk([&count](auto &) { ++count; });
            std::cout << "still alive: " << count << " handles\n";
            handle.close();
        });

        client->read();
    });

    tcp->on<uvw::close_event>([](const uvw::close_event &, uvw::tcp_handle &) {
        std::cout << "close\n";
    });

    tcp->bind("127.0.0.1", 4242);
    tcp->listen();
}

void conn(uvw::loop &loop) {
    auto tcp = loop.resource<uvw::tcp_handle>();
    tcp->on<uvw::error_event>([](const uvw::error_event &, uvw::tcp_handle &) { assert(false); });

    tcp->on<uvw::write_event>([](const uvw::write_event &, uvw::tcp_handle &handle) {
        std::cout << "write\n";
        handle.close();
    });

    tcp->on<uvw::connect_event>([](const uvw::connect_event &, uvw::tcp_handle &handle) {
        std::cout << "connect\n";

        auto dataTryWrite = std::unique_ptr<char[]>(new char[1]{'a'});
        const int bw = handle.try_write(std::move(dataTryWrite), 1);
        std::cout << "written: " << bw << "\n";

        auto dataWrite = std::unique_ptr<char[]>(new char[2]{'b', 'c'});
        handle.write(std::move(dataWrite), 2);
    });

    tcp->on<uvw::close_event>([](const uvw::close_event &, uvw::tcp_handle &) {
        std::cout << "close\n";
    });

    tcp->connect("127.0.0.1", 4242);
}

} // namespace

int main() {
    auto loop = uvw::loop::get_default();
    listen(*loop);
    conn(*loop);
    loop->run();
    loop = nullptr;
}
