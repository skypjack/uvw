# uvw

**Please, note that `uvw` is almost done, but it lacks a proper test suite**.

`uvw` is a header-only, event based, tiny and easy to use C++ wrapper for *libuv*.  
The basic idea is to hide completely the *C-ish* interface of *libuv* behind a graceful C++ API. Currently, no `uv_*_t` data structure is actually exposed by the library.  
Note that `uvw` stays true to the API of *libuv* and it doesn't add anything to its interface. For the same reasons, users of the library must follow the same rules who are used to follow with *libuv*.  
As an example, a *handle* should be initialized before any other operation and closed once it is no longer in use.

## Code Example

```
#include <uvw.hpp>
#include <memory>

void listen(uvw::Loop &loop) {
    std::shared_ptr<uvw::Tcp> tcp = loop.resource<uvw::Tcp>();

    tcp->once<uvw::ListenEvent>([](const uvw::ListenEvent &event, uvw::Tcp &srv) mutable {
        std::shared_ptr<uvw::Tcp> client = srv.loop().resource<uvw::Tcp>();

        client->on<uvw::CloseEvent>([ptr = srv.shared_from_this()](const uvw::CloseEvent &, uvw::Tcp &) mutable { ptr->close(); });
        client->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::Tcp &client) { client.close(); });

        srv.accept(*client);
        client->read();
    });

    tcp->bind<uvw::Tcp::IPv4>("127.0.0.1", 4242);
    tcp->listen();
}

void conn(uvw::Loop &loop) {
    auto tcp = loop.resource<uvw::Tcp>();

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

## Motivation

The main reason for which `uvw` has been written is the fact that it does not exist a valid C++ wrapper for *libuv*. That's all.

# Build Instructions

## Library

`uvw` is a header-only library.  
This means that including the `uvw.hpp` header or one of the other `uvw/*.hpp` headers is enough to use it.  
It's a matter of adding the following line at the top of a file:

    #include <uvw.hpp>

Then pass the proper `-I` argument to the compiler to add the `src` directory to the include paths.  
Note that users are demanded to correctly setup include directories and libraries search paths for *libuv*.

## Documentation

### API Reference

The documentation is based on [`doxygen`](http://www.stack.nl/~dimitri/doxygen/). To build it:

* `$ cd build`
* `$ make docs`

The API reference will be created in HTML format within the directory `build/docs/html`.  
To navigate it with your favorite browser:

* `$ cd build`
* `$ your_favorite_browser docs/html/index.html`

### Crash Course

##### Vademecum

There is only one rule when using `uvw`: always initialize the resources and terminate them.

Resources belong to two main families: _handles_ and _requests_.  
Handles represent long-lived objects capable of performing certain operations while active.  
Requests represent (typically) short-lived operations performed either over a handle or standalone.

The following sections will explain in short what it means to initialize and terminate these kinds of resources.

###### Handles

Initialization is usually performed under the hood and can be even passed over, as far as handles are created using the `Loop::resource` member method.  
On the other side, handles keep themselves alive until one explicitly closes them. Because of that, leaks are possible if users simply forget about a handle.  
Thus the rule quickly becomes *always close your handles*. It's simple as calling the `close` member method on them.

###### Requests

Usually initialization is not required for requests objects. Anyway, the recommended way to create a request is still the `Loop::resource` member method.  
Requests will keep themselves alive as long as they are bound to unfinished underlying activities. This means that users have not to discard explicitly a request.  
Thus the rule quickly becomes *feel free to make a request and forget about it*. It's simple as calling a member method on them.

##### The Loop and the Resource

The first thing to do to use `uvw` is to create a loop. In case the default one is enough, it's easy as doing this:

    auto loop = uvw::Loop::getDefault();

Note that loop objects don't require to be closed explicitly, even if they offer the `close` member method in case an user wants to do that.  
Loops can be run using the `run`, `runOnce` and `runWait` member methods. Please refer to the documentation of *libuv* for further details.

In order to create a resource and to bind it to the given loop, just do the following:

    auto tcp = loop.resource<uvw::Tcp>();

The line above will create and initialize a tcp handle, thus a shared pointer to that resource will be returned.  
Users should check if pointers have been correctly initialized: in case of errors, they won't be.  
Another way to create a resource is:

    auto tcp = Tcp::create(loop);
    tcp->init();

Pretty annoying indeed. Using a loop is the recommended approach.

Remember from the previous section that a handle will keep itself alive until one invokes the `close` member method on it.  
To know what are the handles that are still alive and bound to a given loop, just do the following:

    loop.walk([](uvw::BaseHandle &){ /* application code here */ });

`BaseHandle` exposes a few methods and cannot be used to know the original type of the handle.  
Anyway, it can be used to close the handle that originated from it. As an example, all the pending handles can be closed easily as it follows:

    loop.walk([](uvw::BaseHandle &h){ h.close(); });

No need to keep track of them.

To know what are the available resources' types, please refer the API reference.

##### The event-based approach

For `uvw` offers an event-based approach, resources are small event emitters to which listeners can be attached.  
Attaching a listener to a resource is the reccomended way to be notified about changes.  
Listeners must be callable objects of type `void(const EventType &, ResourceType &)`, where:

* `EventType` is the type of the event for which they have been designed
* `ResourceType` is the type of the resource that has originated the event

Once more, please note that there is no need to keep around references to the resources: they will pass themselves as an argument whenever an event is published.

There exist two methods to attach an event to a resource:

* `resource.once<EventType>(listener)`: the listener will be automatically removed after the first event of the given type
* `resource.on<EventType>(listener)`: to be used for long-running listeners

Both of them return an object of type `ResourceType::Connection` (as an example, `Tcp::Connection`).  
A connection object can be used later as an argument to the `erase` member method of the resource to remove the listener.  
There exists also the `clear` member method to drop all the listeners at once.

The code below shows how to create a simple tcp server using `uvw`:

```
auto loop = uvw::Loop::getDefault();
auto tcp = loop.resource<uvw::Tcp>();

tcp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::Tcp &srv) { /* something went wrong */ });

tcp->on<uvw::ListenEvent>([](const uvw::ListenEvent &event, uvw::Tcp &srv) mutable {
    std::shared_ptr<uvw::Tcp> client = srv.loop().resource<uvw::Tcp>();
    client->once<uvw::EndEvent>([](const uvw::EndEvent &, uvw::Tcp &client) { client.close(); });
    client->on<uvw::DataEvent>([](const uvw::DataEvent &, uvw::Tcp &) { /* data received */ });
    srv.accept(*client);
    client->read();
});

tcp->bind<uvw::Tcp::IPv4>("127.0.0.1", 4242);
tcp->listen();
```

The API reference is the reccomended documentation for further details about resources and their methods.

## Tests

To compile and run the tests, `uvw` requires *libuv* and *googletest*.  
Run the script `deps.sh` to download them. It is good practice to do it every time one pull the project.

Then, to build the tests:

* `$ cd build`
* `$ make`
* `$ make test`

# Contributors

Michele Caini aka [skypjack](https://github.com/skypjack)  
Federico Bertolucci aka [lessness](https://github.com/lessness)  
Luca Martini aka [lordlukas](https://github.com/lucamartini)  
Elia Mazzuoli aka [Zikoel](https://github.com/Zikoel)  
Francesco De Felice aka [fradefe](https://github.com/fradefe)

See [CONTRIBUTING.md](https://github.com/skypjack/uvw/blob/master/CONTRIBUTING.md) if you want to contribute.

# License

Code and documentation Copyright (c) 2016 Michele Caini.  
Code released under [the MIT license](https://github.com/skypjack/uvw/blob/master/LICENSE).
Docs released under [Creative Commons](https://github.com/skypjack/uvw/blob/master/docs/LICENSE).

# Note

This documentation is mostly inspired by the official [libuv API documentation](http://docs.libuv.org/en/v1.x/) for obvious reasons.
