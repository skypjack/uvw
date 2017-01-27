#include <uvw.hpp>
#include <cassert>
#include <iostream>
#include <memory>
#include <chrono>


void listen(uvw::Loop &loop) {
    std::shared_ptr<uvw::TcpHandle> tcp = loop.resource<uvw::TcpHandle>();

    tcp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TcpHandle &) {
        std::cout << "error " << std::endl;
    });

    tcp->once<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::TcpHandle &srv) {
        std::cout << "listen" << std::endl;

        std::shared_ptr<uvw::TcpHandle> client = srv.loop().resource<uvw::TcpHandle>();

        client->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TcpHandle &) {
            std::cout << "error " << std::endl;
        });

        client->on<uvw::CloseEvent>([ptr = srv.shared_from_this()](const uvw::CloseEvent &, uvw::TcpHandle &) {
            std::cout << "close" << std::endl;
            ptr->close();
        });

        srv.accept(*client);

        uvw::Addr local = srv.sock();
        std::cout << "local: " << local.ip << " " << local.port << std::endl;

        uvw::Addr remote = client->peer();
        std::cout << "remote: " << remote.ip << " " << remote.port << std::endl;

        client->on<uvw::DataEvent>([](const uvw::DataEvent &event, uvw::TcpHandle &) {
            std::cout.write(event.data.get(), event.length) << std::endl;
            std::cout << "data length: " << event.length << std::endl;
        });

        client->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TcpHandle &handle) {
            std::cout << "end" << std::endl;
            int count = 0;
            handle.loop().walk([&count](uvw::BaseHandle &) { ++count; });
            std::cout << "still alive: " << count << " handles" << std::endl;
            handle.close();
        });

        client->read();
    });

    tcp->once<uvw::CloseEvent>([](const uvw::CloseEvent &, uvw::TcpHandle &) {
        std::cout << "close" << std::endl;
    });

    tcp->bind("127.0.0.1", 4242);
    tcp->listen();
}


void conn(uvw::Loop &loop) {
    auto tcp = loop.resource<uvw::TcpHandle>();

    tcp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TcpHandle &) {
        std::cout << "error " << std::endl;
    });

    tcp->once<uvw::WriteEvent>([](const uvw::WriteEvent &, uvw::TcpHandle &handle) {
        std::cout << "write" << std::endl;
        handle.close();
    });

    tcp->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::TcpHandle &handle) {
        std::cout << "connect" << std::endl;

        auto dataTryWrite = std::unique_ptr<char[]>(new char[1]{ 'a' });
        int bw = handle.tryWrite(std::move(dataTryWrite), 1);
        std::cout << "written: " << ((int)bw) << std::endl;

        auto dataWrite = std::unique_ptr<char[]>(new char[2]{ 'b', 'c' });
        handle.write(std::move(dataWrite), 2);
    });

    tcp->once<uvw::CloseEvent>([](const uvw::CloseEvent &, uvw::TcpHandle &) {
        std::cout << "close" << std::endl;
    });

    tcp->connect("127.0.0.1", 4242);
}

void g() {
    auto loop = uvw::Loop::getDefault();
    listen(*loop);
    conn(*loop);
    loop->run();
    loop = nullptr;
}

int main() {
    g();
}
