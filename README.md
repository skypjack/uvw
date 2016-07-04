# uvw

`uvw` is a header-only, event based, tiny and easy to use C++ wrapper for *libuv*.  
The basic idea is to hide completely the *C-ish* interace of *libuv* behind a graceful C++ API. Currently, no *uv_***_t* data structure is actually exposed by the library.  
Note that `uvw` stays true to the API of *libuv* and it doesn't add anything to its interface. For the same reason, users of the library must follow the same rules who are used to follow with *libuv*.  
As an example, a *handle* should be initialized before any other operation and closed once it is no longer in use.  

# Code Example

```
#include <uvw.hpp>
#include <memory>

void listen(uvw::Loop &loop) {
    std::shared_ptr<uvw::Tcp> tcp = loop.handle<uvw::Tcp>();

    tcp->once<uvw::ListenEvent>([](const uvw::ListenEvent &event, uvw::Tcp &srv) mutable {
        std::shared_ptr<uvw::Tcp> client = srv.loop().handle<uvw::Tcp>();

        client->on<uvw::CloseEvent>([ptr = srv.shared_from_this()](const uvw::CloseEvent &, uvw::Tcp &) mutable { ptr->close(); });
        client->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::Tcp &client) { client.close(); });

        srv.accept(*client);
        client->read();
    });

    tcp->bind<uvw::Tcp::IPv4>("127.0.0.1", 4242);
    tcp->listen();
}

void conn(uvw::Loop &loop) {
    auto tcp = loop.handle<uvw::Tcp>();

    tcp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::Tcp &) { /* handle errors */ });

    tcp->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::Tcp &tcp) mutable {
        auto dataWrite = std::unique_ptr<char[]>(new char[2]{ 'b', 'c' });
        tcp.write(std::move(dataWrite), 2);
        tcp.close();
    });

    tcp->connect<uvw::Tcp::IPv4>(std::string{"127.0.0.1"}, 4242);
}

int main() {
    auto loop = uvw::Loop::getDefault();
    listen(*loop);
    conn(*loop);
    loop->run();
}
```

# Motivation

The main reason for which `uvw` has been written is the fact that it does not exist a valid C++ wrapper for *libuv*. That's all.

# Installation

`uvw` is a header-only library.  
This means that include the `uvw.hpp` header or one of the other `uvw/*.hpp` headers is enough to use it.  
It's a matter of adding the following line at the top of a file:

    #include <uvw.hpp>

Then pass the proper `-I` argument to the compiler to add the `src` directory to the include paths.

# API Reference

TODO

# Tests

TODO

# Contributors

Michele Caini aka [skypjack](https://github.com/skypjack)

See [CONTRIBUTING.md](https://github.com/skypjack/uvw/blob/master/CONTRIBUTING.md) if you want to contribute.

# License

Code and documentation Copyright (c) 2016 Michele Caini.  
Code released under [the MIT license](https://github.com/skypjack/uvw/blob/master/LICENSE).
Docs released under [Creative Commons](https://github.com/skypjack/uvw/blob/master/docs/LICENSE).
