#include <uvw.hpp>
#include <cassert>
#include <iostream>
#include <memory>
#include <chrono>


void listen(uvw::Loop &loop) {
    std::shared_ptr<uvw::Tcp> tcp = loop.handle<uvw::Tcp>();

    tcp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::Tcp &) {
        std::cout << "error " << std::endl;
    });

    tcp->once<uvw::ListenEvent>([](const uvw::ListenEvent &event, uvw::Tcp &srv) mutable {
        std::cout << "listen" << std::endl;

        std::shared_ptr<uvw::Tcp> client = srv.loop().handle<uvw::Tcp>();

        client->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::Tcp &) {
            std::cout << "error " << std::endl;
        });

        client->on<uvw::CloseEvent>([ptr = srv.shared_from_this()](const uvw::CloseEvent &, uvw::Tcp &) mutable {
            std::cout << "close" << std::endl;

            uvw::Tcp &srv = *ptr;
            srv.close();
        });

        srv.accept(*client);

        uvw::Addr local = srv.address<uvw::Tcp::IPv4>();
        std::cout << "local: " << local.first << " " << local.second << std::endl;

        uvw::Addr remote = client->remote<uvw::Tcp::IPv4>();
        std::cout << "remote: " << remote.first << " " << remote.second << std::endl;

        client->on<uvw::DataEvent>([](const uvw::DataEvent &event, uvw::Tcp &) {
            std::cout.write(event.data(), event.length()) << std::endl;
            std::cout << "data length: " << event.length() << std::endl;
        });

        client->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::Tcp &client) {
            std::cout << "end" << std::endl;
            client.close();
        });

        client->read();
    });

    tcp->once<uvw::CloseEvent>([](const uvw::CloseEvent &, uvw::Tcp &) mutable {
        std::cout << "close" << std::endl;
    });

    tcp->bind<uvw::Tcp::IPv4>("127.0.0.1", 4242);
    tcp->listen();
}


void conn(uvw::Loop &loop) {
    auto tcp = loop.handle<uvw::Tcp>();

    tcp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::Tcp &) {
        std::cout << "error " << std::endl;
    });

    tcp->once<uvw::WriteEvent>([](const uvw::WriteEvent &, uvw::Tcp &tcp) mutable {
        std::cout << "write" << std::endl;
        tcp.close();
    });

    tcp->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::Tcp &tcp) mutable {
        std::cout << "connect" << std::endl;

        auto dataTryWrite = std::unique_ptr<char[]>(new char[1]{ 'a' });
        int bw = tcp.tryWrite(std::move(dataTryWrite), 1);
        std::cout << "written: " << ((int)bw) << std::endl;

        auto dataWrite = std::unique_ptr<char[]>(new char[2]{ 'b', 'c' });
        tcp.write(std::move(dataWrite), 2);
    });

    tcp->once<uvw::CloseEvent>([](const uvw::CloseEvent &, uvw::Tcp &) mutable {
        std::cout << "close" << std::endl;
    });

    tcp->connect<uvw::Tcp::IPv4>(std::string{"127.0.0.1"}, 4242);
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
