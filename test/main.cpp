#include <iostream>
#include <uvw.hpp>


void f(uvw::Loop &loop) {
    uvw::Handle<uvw::Tcp> handle = loop.handle<uvw::Tcp>();

    auto cb = [](uvw::UVWError err, uvw::Tcp &tcp) mutable {
        std::cout << "---" << ((bool)err) << std::endl;
        tcp.close([](uvw::UVWError err, uvw::Tcp &) mutable {
            std::cout << "---" << ((bool)err) << std::endl;
        });
    };

    uvw::Tcp &tcp = handle;
    tcp.connect<uvw::Tcp::IPv4>(std::string{"127.0.0.1"}, 80, cb);
}

void g() {
    auto loop = uvw::Loop::getDefault();
    f(*loop);
    loop->run();
    loop = nullptr;
}

int main() {
    g();
}
