#include <iostream>
#include <uvw.hpp>


void f(uvw::Loop &loop) {
    uvw::Handle<uvw::Tcp> handle = loop.handle<uvw::Tcp>();

    auto cb = [handle](uvw::UVWError err) mutable {
        std::cout << "---" << ((bool)err) << std::endl;
        uvw::Tcp &tcp = handle;
        tcp.close([handle](uvw::UVWError err) mutable {
            std::cout << "---" << ((bool)err) << std::endl;
            handle = uvw::Handle<uvw::Tcp>{};
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
