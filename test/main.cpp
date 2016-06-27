#include <iostream>
#include <uvw.hpp>


void listen(uvw::Loop &loop) {
    uvw::Handle<uvw::Tcp> handle = loop.handle<uvw::Tcp>();

    auto cb = [](uvw::UVWError err, uvw::Tcp &srv) mutable {
        std::cout << "listen: " << ((bool)err) << std::endl;

        if(!err) {
            uvw::Handle<uvw::Tcp> handle = srv.loop().handle<uvw::Tcp>();
            uvw::Tcp &client = handle;

            err = srv.accept(client);
            std::cout << "accept: " << ((bool)err) << std::endl;

            if(!err) {
                client.close([handle = srv.handle()](uvw::UVWError err, uvw::Tcp &) mutable {
                    std::cout << "close: " << ((bool)err) << std::endl;

                    uvw::Tcp &srv = handle;
                    srv.close([](uvw::UVWError err, uvw::Tcp &) mutable {
                        std::cout << "close: " << ((bool)err) << std::endl;
                    });
                });
            }
        }
    };

    uvw::Tcp &tcp = handle;
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
    uvw::Handle<uvw::Tcp> handle = loop.handle<uvw::Tcp>();

    auto cb = [](uvw::UVWError err, uvw::Tcp &tcp) mutable {
        std::cout << "connect: " << ((bool)err) << std::endl;

        auto cb = [](uvw::UVWError err, uvw::Tcp &tcp) mutable {
            std::cout << "close: " << ((bool)err) << std::endl;
        };

        tcp.close(cb);
    };

    uvw::Tcp &tcp = handle;
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
