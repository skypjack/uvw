![cynny-logo](https://web.cynny.com/live/static/favicon/favicon-16.png) **Sponsored and contributed by [Cynny SpA](https://www.cynny.com/).**

# uvw

[![Build Status](https://travis-ci.org/skypjack/uvw.svg?branch=master)](https://travis-ci.org/skypjack/uvw)
[![Build status](https://ci.appveyor.com/api/projects/status/m5ndm8gnu8isg2to?svg=true)](https://ci.appveyor.com/project/skypjack/uvw)

# Introduction

`uvw` is a header-only, event based, tiny and easy to use *libuv* wrapper in modern C++.<br/>
The basic idea is to hide completely the *C-ish* interface of *libuv* behind a graceful C++ API. Currently, no `uv_*_t` data structure is actually exposed by the library.<br/>
Note that `uvw` stays true to the API of *libuv* and it doesn't add anything to its interface. For the same reasons, users of the library must follow the same rules who are used to follow with *libuv*.<br/>
As an example, a *handle* should be initialized before any other operation and closed once it is no longer in use.

## Code Example

```
#include <uvw.hpp>
#include <memory>

void listen(uvw::Loop &loop) {
    std::shared_ptr<uvw::TcpHandle> tcp = loop.resource<uvw::TcpHandle>();

    tcp->once<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::TcpHandle &srv) {
        std::shared_ptr<uvw::TcpHandle> client = srv.loop().resource<uvw::TcpHandle>();

        client->on<uvw::CloseEvent>([ptr = srv.shared_from_this()](const uvw::CloseEvent &, uvw::TcpHandle &) { ptr->close(); });
        client->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TcpHandle &client) { client.close(); });

        srv.accept(*client);
        client->read();
    });

    tcp->bind("127.0.0.1", 4242);
    tcp->listen();
}

void conn(uvw::Loop &loop) {
    auto tcp = loop.resource<uvw::TcpHandle>();

    tcp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TcpHandle &) { /* handle errors */ });

    tcp->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::TcpHandle &tcp) {
        auto dataWrite = std::unique_ptr<char[]>(new char[2]{ 'b', 'c' });
        tcp.write(std::move(dataWrite), 2);
        tcp.close();
    });

    tcp->connect(std::string{"127.0.0.1"}, 4242);
}

int main() {
    auto loop = uvw::Loop::getDefault();
    listen(*loop);
    conn(*loop);
    loop->run();
}
```

## Motivation

The main reason for which `uvw` has been written is the fact that it does not exist a valid *libuv* wrapper in C++. That's all.

# Build Instructions

## Requirements

To be able to use `uvw`, users must provide the following system-wide tools:

* A full-featured compiler that supports at least C++14
* `libuv` version 1.9.0 or later

The requirements below are mandatory to compile the tests and to extract the documentation:

* CMake version 3.4 or later
* Doxygen version 1.8 or later

Note that `libuv` is part of the dependencies of the project and it will be cloned by executing the script `deps.sh` (see below for further details).<br/>
Because of that, users have not to install it to compile and execute the tests.

## Library

`uvw` is a header-only library.<br/>
This means that including the `uvw.hpp` header or one of the other `uvw/*.hpp` headers is enough to use it.<br/>
It's a matter of adding the following line at the top of a file:

    #include <uvw.hpp>

Then pass the proper `-I` argument to the compiler to add the `src` directory to the include paths.<br/>
Note that users are demanded to correctly setup include directories and libraries search paths for *libuv*.

## Documentation

### API Reference

The documentation is based on [`doxygen`](http://www.stack.nl/~dimitri/doxygen/). To build it:

* `$ cd build`
* `$ cmake ..`
* `$ make docs`

The API reference will be created in HTML format within the directory `build/docs/html`.<br/>
To navigate it with your favorite browser:

* `$ cd build`
* `$ your_favorite_browser docs/html/index.html`

### Crash Course

#### Vademecum

There is only one rule when using `uvw`: always initialize the resources and terminate them.

Resources belong mainly to two families: _handles_ and _requests_.<br/>
Handles represent long-lived objects capable of performing certain operations while active.<br/>
Requests represent (typically) short-lived operations performed either over a handle or standalone.

The following sections will explain in short what it means to initialize and terminate these kinds of resources.

#### Handles

Initialization is usually performed under the hood and can be even passed over, as far as handles are created using the `Loop::resource` member function.<br/>
On the other side, handles keep themselves alive until one explicitly closes them. Because of that, memory usage will grow up if users simply forget about a handle.<br/>
Therefore the rule quickly becomes *always close your handles*. It's simple as calling the `close` member function on them.

#### Requests

Usually initializing a request object is not required. Anyway, the recommended way to create a request is still through the `Loop::resource` member function.<br/>
Requests will keep themselves alive as long as they are bound to unfinished underlying activities. This means that users have not to discard explicitly a request.<br/>
Therefore the rule quickly becomes *feel free to make a request and forget about it*. It's simple as calling a member function on them.

#### The Loop and the Resource

The first thing to do to use `uvw` is to create a loop. In case the default one is enough, it's easy as doing this:

    auto loop = uvw::Loop::getDefault();

Note that loop objects don't require to be closed explicitly, even if they offer the `close` member function in case an user wants to do that.<br/>
Loops can be started using the `run` member function. The two calls below are equivalent:

    loop->run();
    loop->run<uvw::Loop::Mode::DEFAULT>

Available modes are: `DEFAULT`, `ONCE`, `NOWAIT`. Please refer to the documentation of *libuv* for further details.

In order to create a resource and to bind it to the given loop, just do the following:

    auto tcp = loop.resource<uvw::TcpHandle>();

The line above will create and initialize a tcp handle, then a shared pointer to that resource will be returned.<br/>
Users should check if pointers have been correctly initialized: in case of errors, they won't be.<br/>
Another way to create a resource is:

    auto tcp = TcpHandle::create(loop);
    tcp->init();

Pretty annoying indeed. Using a loop is the recommended approach.

The resources also accept arbitrary user-data that won't be touched in any case.<br/>
Users can set and get them through the `data` member function as it follows:

    resource->data(std::make_shared<int>(42));
    std::shared_ptr<void> data = resource->data();

Resources expect a `std::shared_pointer<void>` and return it, therefore any kind of data is welcome.<br/>
Users can explicitly specify a type other than `void` when calling the `data` member function:

    std::shared_ptr<int> data = resource->data<int>();

Remember from the previous section that a handle will keep itself alive until one invokes the `close` member function on it.<br/>
To know what are the handles that are still alive and bound to a given loop, just do the following:

    loop.walk([](uvw::BaseHandle &){ /* application code here */ });

`BaseHandle` exposes a few methods and cannot be used to know the original type of the handle.<br/>
Anyway, it can be used to close the handle that originated from it. As an example, all the pending handles can be closed easily as it follows:

    loop.walk([](uvw::BaseHandle &h){ h.close(); });

No need to keep track of them.

To know what are the available resources' types, please refer the API reference.

#### The event-based approach

For `uvw` offers an event-based approach, resources are small event emitters to which listeners can be attached.<br/>
Attaching a listener to a resource is the recommended way to be notified about changes.<br/>
Listeners must be callable objects of type `void(EventType &, ResourceType &)`, where:

* `EventType` is the type of the event for which they have been designed
* `ResourceType` is the type of the resource that has originated the event

It means that the following function types are all valid:

* `void(EventType &, ResourceType &)`
* `void(const EventType &, ResourceType &)`
* `void(EventType &, const ResourceType &)`
* `void(const EventType &, const ResourceType &)`

Once more, please note that there is no need to keep around references to the resources: they will pass themselves as an argument whenever an event is published.

There exist two methods to attach an event to a resource:

* `resource.once<EventType>(listener)`: the listener will be automatically removed after the first event of the given type
* `resource.on<EventType>(listener)`: to be used for long-running listeners

Both of them return an object of type `ResourceType::Connection` (as an example, `TcpHandle::Connection`).<br/>
A connection object can be used later as an argument to the `erase` member function of the resource to remove the listener.<br/>
There exists also the `clear` member function to drop all the listeners at once.

Almost all the resources use to emit `ErrorEvent` events in case of errors.<br/>
All the other events are specific for the given resource and documented in the API reference.

The code below shows how to create a simple tcp server using `uvw`:

```
auto loop = uvw::Loop::getDefault();
auto tcp = loop.resource<uvw::TcpHandle>();

tcp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TcpHandle &) { /* something went wrong */ });

tcp->on<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::TcpHandle &srv) {
    std::shared_ptr<uvw::TcpHandle> client = srv.loop().resource<uvw::TcpHandle>();
    client->once<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TcpHandle &client) { client.close(); });
    client->on<uvw::DataEvent>([](const uvw::DataEvent &, uvw::TcpHandle &) { /* data received */ });
    srv.accept(*client);
    client->read();
});

tcp->bind("127.0.0.1", 4242);
tcp->listen();
```

Note also that `uvw::TcpHandle` already supports _IPv6_ out-of-the-box. The statement above is equivalent to `tcp->bind<uvw::IPv4>("127.0.0.1", 4242)`.<br/>
It's suffice to explicitly specify `uvw::IPv6` as the underlying protocol to use it.

The API reference is the recommended documentation for further details about resources and their methods.

## Tests

To compile and run the tests, `uvw` requires *libuv* and *googletest*.<br/>
Run the script `deps.sh` to download them. It is good practice to do it every time one pull the project.

Then, to build the tests:

* `$ cd build`
* `$ cmake ..`
* `$ make`
* `$ make test`

# Contributors

Michele Caini aka [skypjack](https://github.com/skypjack)<br/>
Federico Bertolucci aka [lessness](https://github.com/lessness)<br/>
Luca Martini aka [lordlukas](https://github.com/lucamartini)<br/>
Elia Mazzuoli aka [Zikoel](https://github.com/Zikoel)<br/>
Francesco De Felice aka [fradefe](https://github.com/fradefe)<br/>
Tushar Maheshwari aka [tusharpm](https://github.com/tusharpm)<br/>
Paolo Monteverde aka [morbo84](https://github.com/morbo84)<br/>
Jan Vcelak aka [fcelda](https://github.com/fcelda)<br/>
Raoul Hecky aka raoul [raoulh](https://github.com/raoulh)

If you want to contribute, please send patches as pull requests against the branch master.

# License

Code and documentation Copyright (c) 2017 Michele Caini.<br/>
Code released under [the MIT license](https://github.com/skypjack/uvw/blob/master/LICENSE).<br/>
Docs released under [Creative Commons](https://github.com/skypjack/uvw/blob/master/docs/LICENSE).

# Note

This documentation is mostly inspired by the official [libuv API documentation](http://docs.libuv.org/en/v1.x/) for obvious reasons.
