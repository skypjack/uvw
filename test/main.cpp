#include <iostream>
#include <uvw.hpp>


void listen(uvw::Loop &loop) {
    uvw::Resource<uvw::Tcp> resource = loop.resource<uvw::Tcp>();

    auto cb = [](uvw::UVWError err, uvw::Tcp &srv) mutable {
        std::cout << "listen: " << ((bool)err) << std::endl;

        if(!err) {
            uvw::Resource<uvw::Tcp> resource = srv.loop().resource<uvw::Tcp>();
            uvw::Tcp &client = resource;

            err = srv.accept(client);
            std::cout << "accept: " << ((bool)err) << std::endl;

            auto local = srv.address<uvw::Tcp::IPv4>();

            if(local) {
                const uvw::Addr &ref = local;
                std::cout << "local: " << ref.first << " " << ref.second << std::endl;
            }

            auto remote = client.remote<uvw::Tcp::IPv4>();

            if(remote) {
                const uvw::Addr &ref = remote;
                std::cout << "remote: " << ref.first << " " << ref.second << std::endl;
            }

            if(!err) {
                client.close([resource = srv.resource()](uvw::UVWError err, uvw::Tcp &) mutable {
                    std::cout << "close: " << ((bool)err) << std::endl;

                    uvw::Tcp &srv = resource;
                    srv.close([](uvw::UVWError err, uvw::Tcp &) mutable {
                        std::cout << "close: " << ((bool)err) << std::endl;
                    });
                });
            }
        }
    };

    uvw::Tcp &tcp = resource;
    uvw::UVWError err = tcp.bind<uvw::Tcp::IPv4>("127.0.0.1", 4242);
    std::cout << "bind: " << ((bool)err) << std::endl;

    if(err) {
        tcp.close([](uvw::UVWError err, uvw::Tcp &) mutable {
            std::cout << "close: " << ((bool)err) << std::endl;
        });
    } else {
        tcp.listen(0, cb);
    }
}


void conn(uvw::Loop &loop) {
    uvw::Resource<uvw::Tcp> resource = loop.resource<uvw::Tcp>();

    auto cb = [](uvw::UVWError err, uvw::Tcp &tcp) mutable {
        std::cout << "connect: " << ((bool)err) << std::endl;

        auto data = std::unique_ptr<char[]>(new char[1]);
        data[0] = 42;
        uvw::Buffer buf{std::move(data), 1};
        uvw::UVWOptionalData<int> bw = tcp.tryWrite(std::move(buf));

        if(bw) {
            std::cout << "written: " << ((int)bw) << std::endl;
        } else {
            std::cout << "written err: " << ((int)bw) << std::endl;
        }

        auto cb = [](uvw::UVWError err, uvw::Tcp &) mutable {
            std::cout << "close: " << ((bool)err) << std::endl;
        };

        tcp.close(cb);
    };

    uvw::Tcp &tcp = resource;
    tcp.connect<uvw::Tcp::IPv4>(std::string{"127.0.0.1"}, 4242, cb);
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
