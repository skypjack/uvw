#include <uvw.hpp>
#include <cassert>
#include <iostream>
#include <memory>
#include <chrono>


void listen(uvw::Loop &loop) {
    std::shared_ptr<uvw::Tcp> tcp = loop.resource<uvw::Tcp>();

    uvw::UVWError err = tcp->bind<uvw::Tcp::IPv4>("127.0.0.1", 4242);
    std::cout << "bind: " << ((bool)err) << std::endl;

    tcp->once<uvw::ListenEvent>([](const uvw::ListenEvent &event, uvw::Tcp &srv) mutable {
        std::cout << "listen: " << ((bool)event.error) << std::endl;

        if(!event.error) {
            std::shared_ptr<uvw::Tcp> client = srv.loop().resource<uvw::Tcp>();

            auto err = srv.accept(*client);
            std::cout << "accept: " << ((bool)err) << std::endl;

            auto local = srv.address<uvw::Tcp::IPv4>();

            if(local) {
                const uvw::Addr &ref = local;
                std::cout << "local: " << ref.first << " " << ref.second << std::endl;
            }

            auto remote = client->remote<uvw::Tcp::IPv4>();

            if(remote) {
                const uvw::Addr &ref = remote;
                std::cout << "remote: " << ref.first << " " << ref.second << std::endl;
            }

            client->on<uvw::CloseEvent>([resource = srv.shared_from_this()](uvw::CloseEvent, uvw::Tcp &) mutable {
                std::cout << "close" << std::endl;

                uvw::Tcp &srv = *resource;
                srv.close();
            });

            if(!err) {
                client->close();
            }
        }
    });

    tcp->once<uvw::CloseEvent>([](const uvw::CloseEvent &, uvw::Tcp &) mutable {
        std::cout << "close" << std::endl;
    });

    if(err) {
        tcp->close();
    } else {
        tcp->listen(0);
    }
}


void conn(uvw::Loop &loop) {
    auto tcp = loop.resource<uvw::Tcp>();

    tcp->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &event, uvw::Tcp &tcp) mutable {
        std::cout << "connect: " << ((bool)event.error) << std::endl;

        auto data = std::unique_ptr<char[]>(new char[1]);
        data[0] = 42;
        uvw::Buffer buf{std::move(data), 1};
        uvw::UVWOptionalData<int> bw = tcp.tryWrite(std::move(buf));

        if(bw) {
            std::cout << "written: " << ((int)bw) << std::endl;
        } else {
            std::cout << "written err: " << ((int)bw) << std::endl;
        }

        tcp.close();
    });

    tcp->once<uvw::CloseEvent>([](const uvw::CloseEvent &, uvw::Tcp &) mutable {
        std::cout << "close" << std::endl;
    });

    auto err = tcp->connect<uvw::Tcp::IPv4>(std::string{"127.0.0.1"}, 4242);
    std::cout << "connect: " << ((bool)err) << std::endl;
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
