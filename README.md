![uvw - libuv wrapper in modern C++](https://user-images.githubusercontent.com/1812216/46069406-c977a600-c17b-11e8-9a47-9bba6f412c57.png)

<!--
@cond TURN_OFF_DOXYGEN
-->
[![Build Status](https://github.com/skypjack/uvw/workflows/build/badge.svg)](https://github.com/skypjack/uvw/actions)
[![Coverage](https://codecov.io/gh/skypjack/uvw/branch/master/graph/badge.svg)](https://codecov.io/gh/skypjack/uvw)
[![Documentation](https://img.shields.io/badge/docs-docsforge-blue)](http://uvw.docsforge.com/)
[![Download](https://api.bintray.com/packages/skypjack/conan/uvw%3Askypjack/images/download.svg)](https://bintray.com/skypjack/conan/uvw%3Askypjack/_latestVersion)
[![Gitter chat](https://badges.gitter.im/skypjack/uvw.png)](https://gitter.im/skypjack/uvw)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.me/skypjack)

Do you have a **question** that doesn't require you to open an issue? Join the
[gitter channel](https://gitter.im/skypjack/uvw).<br/>
If you use `uvw` and you want to say thanks or support the project, please
**consider becoming a
[sponsor](https://github.com/users/skypjack/sponsorship)**.<br/>
You can help me make the difference.
[Many thanks](https://skypjack.github.io/sponsorship/) to those who supported me
and still support me today.
<!--
@endcond TURN_OFF_DOXYGEN
-->

# Introduction

`uvw` started as a header-only, event based, tiny and easy to use wrapper for
[`libuv`](https://github.com/libuv/libuv) written in modern C++.<br/>
Now it's finally available also as a compilable static library.

The basic idea is to hide completely the *C-ish* interface of `libuv` behind a
graceful C++ API. Currently, no `uv_*_t` data structure is actually exposed by
the library.<br/>
Note that `uvw` stays true to the API of `libuv` and it doesn't add anything to
its interface. For the same reasons, users of the library must follow the same
rules which are used with `libuv`.<br/>
As an example, a *handle* should be initialized before any other operation and
closed once it is no longer in use.

## Code Example

```cpp
#include <uvw.hpp>
#include <memory>

void listen(uvw::Loop &loop) {
    std::shared_ptr<uvw::TCPHandle> tcp = loop.resource<uvw::TCPHandle>();

    tcp->once<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::TCPHandle &srv) {
        std::shared_ptr<uvw::TCPHandle> client = srv.loop().resource<uvw::TCPHandle>();

        client->on<uvw::CloseEvent>([ptr = srv.shared_from_this()](const uvw::CloseEvent &, uvw::TCPHandle &) { ptr->close(); });
        client->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TCPHandle &client) { client.close(); });

        srv.accept(*client);
        client->read();
    });

    tcp->bind("127.0.0.1", 4242);
    tcp->listen();
}

void conn(uvw::Loop &loop) {
    auto tcp = loop.resource<uvw::TCPHandle>();

    tcp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TCPHandle &) { /* handle errors */ });

    tcp->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::TCPHandle &tcp) {
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

The main reason for which `uvw` has been written is the fact that there does not
exist a valid `libuv` wrapper in C++. That's all.

# Build Instructions

## Requirements

To be able to use `uvw`, users must provide the following system-wide tools:

* A full-featured compiler that supports at least C++17.
* `libuv` (which version depends on the tag of `uvw` in use).

The requirements below are mandatory to compile the tests and to extract the
documentation:

* CMake version 3.13 or later.
* Doxygen version 1.8 or later.

Note that `libuv` is part of the dependencies of the project and may be cloned
by `CMake` in some cases (see below for further details).<br/>
Because of that, users don't have to install it to run the tests or when `uvw`
libraries are compiled through `CMake`.

## Library

`uvw` is a dual-mode library. It can be used in its header-only form or as a
compiled static library.<br/>
The following sections describe what to do in both cases to get `uvw` up and
runningin your own project.

### Header-only

To use `uvw` as a header-only library, all is needed is to include the `uvw.hpp`
header or one of the other `uvw/*.hpp` files.<br/>
It's a matter of adding the following line at the top of a file:

```cpp
#include <uvw.hpp>
```

Then pass the proper `-I` argument to the compiler to add the `src` directory to
the include paths.<br/>
Note that users are required to correctly setup the include directories and
libraries search paths for `libuv` in this case.

When used through `CMake`, the `uvw::uvw` target is exported for convenience.

### Static

To use `uvw` as a compiled library, set the `BUILD_UVW_LIBS` options in cmake
before including the project.<br/>
This option triggers the generation of a targets named
`uvw::uvw-static`. The matching version of `libuv` is also
compiled and exported as `uv::uv-static` for convenience.

In case you don't use or don't want to use `CMake`, you can still compile all
`.cpp` files and include all `.h` files to get the job done. In this case, users
are required to correctly setup the include directories and libraries search
paths for `libuv`.

## Versioning

Starting with tag _v1.12.0_ of `libuv`, `uvw` follows the
[semantic versioning](http://semver.org/) scheme.<br/>
The problem is that any version of `uvw` also requires to track explicitly the
version of `libuv` to which it is bound.<br/>
Because of that, the latter wil be appended to the version of `uvw`. As an
example:

    vU.V.W_libuv-vX.Y

In particular, the following applies:

* _U.V.W_ are major, minor and patch versions of `uvw`.
* _X.Y_ is the version of `libuv` to which to refer (where any patch version is
  valid).

In other terms, tags will look like this from now on:

    v1.0.0_libuv-v1.12

Branch `master` of `uvw` will be a work in progress branch that follows branch
_v1.x_ of `libuv` (at least as long as it remains their _master_ branch).<br/>

## Documentation

The documentation is based on
[`doxygen`](http://www.stack.nl/~dimitri/doxygen/). To build it:

* `$ cd build`
* `$ cmake ..`
* `$ make docs`

The API reference will be created in HTML format within the directory
`build/docs/html`.<br/>
To navigate it with your favorite browser:

* `$ cd build`
* `$ your_favorite_browser docs/html/index.html`

The same version is also available [online](https://skypjack.github.io/uvw/)
for the latest release, that is the last stable tag. If you are looking for
something more pleasing to the eye, consider reading the nice-looking version
available on [docsforge](https://uvw.docsforge.com/): same documentation, much
more pleasant to read.

### Note

The documentation is mostly inspired by the official
[libuv API documentation](http://docs.libuv.org/en/v1.x/) for obvious
reasons.

## Tests

To compile and run the tests, `uvw` requires `libuv` and `googletest`.<br/>
`CMake` will download and compile both the libraries before compiling anything
else.

To build the tests:

* `$ cd build`
* `$ cmake .. -DBUILD_TESTING=ON`
* `$ make`
* `$ ctest -j4 -R uvw`

Omit `-R uvw` if you also want to test `libuv` and other dependencies.

# Crash Course

## Vademecum

There is only one rule when using `uvw`: always initialize the resources and
terminate them.

Resources belong mainly to two families: _handles_ and _requests_.<br/>
Handles represent long-lived objects capable of performing certain operations
while active.<br/>
Requests represent (typically) short-lived operations performed either over a
handle or standalone.

The following sections will explain in short what it means to initialize and
terminate these kinds of resources.<br/>
For more details, please refer to the
[online documentation](https://skypjack.github.io/uvw/).

## Handles

Initialization is usually performed under the hood and can be even passed over,
as far as handles are created using the `Loop::resource` member function.<br/>
On the other side, handles keep themselves alive until one explicitly closes
them. Because of that, memory usage will grow if users simply forget about a
handle.<br/>
Therefore the rule quickly becomes *always close your handles*. It's as simple
as calling the `close` member function on them.

## Requests

Usually initializing a request object is not required. Anyway, the recommended
way to create a request is still through the `Loop::resource` member
function.<br/>
Requests will keep themselves alive as long as they are bound to unfinished
underlying activities. This means that users don't have to discard a
request explicitly .<br/>
Therefore the rule quickly becomes *feel free to make a request and forget about
it*. It's as simple as calling a member function on them.

## The Loop and the Resource

The first thing to do to use `uvw` is to create a loop. In case the default one
is enough, it's easy as doing this:

```cpp
auto loop = uvw::Loop::getDefault();
```

Note that loop objects don't require being closed explicitly, even if they offer
the `close` member function in case a user wants to do that.<br/>
Loops can be started using the `run` member function. The two calls below are
equivalent:

```cpp
loop->run();
loop->run<uvw::Loop::Mode::DEFAULT>();
```

Available modes are: `DEFAULT`, `ONCE`, `NOWAIT`. Please refer to the
documentation of `libuv` for further details.

In order to create a resource and to bind it to the given loop, just do the
following:

```cpp
auto tcp = loop->resource<uvw::TCPHandle>();
```

The line above will create and initialize a tcp handle, then a shared pointer to
that resource will be returned.<br/>
Users should check if pointers have been correctly initialized: in case of
errors, they won't be.<br/>
Another way to create a resource is:

```cpp
auto tcp = TCPHandle::create(loop);
tcp->init();
```

Pretty annoying indeed. Using a loop is the recommended approach.

The resources also accept arbitrary user-data that won't be touched in any
case.<br/>
Users can set and get them through the `data` member function as it follows:

```cpp
resource->data(std::make_shared<int>(42));
std::shared_ptr<void> data = resource->data();
```

Resources expect a `std::shared_pointer<void>` and return it, therefore any kind
of data is welcome.<br/>
Users can explicitly specify a type other than `void` when calling the `data`
member function:

```cpp
std::shared_ptr<int> data = resource->data<int>();
```

Remember from the previous section that a handle will keep itself alive until
one invokes the `close` member function on it.<br/>
To know what are the handles that are still alive and bound to a given loop,
there exists the `walk` member function. It returns handles with their types.
Therefore, the use of `Overloaded` is recommended to be able to intercept all
types of interest:

```cpp
handle.loop().walk(uvw::Overloaded{
    [](uvw::TimerHandle &h){ /* application code for timers here */ },
    [](auto &&){ /* ignore all other types */ }
});
```

This function can also be used for a completely generic approach. For example,
all the pending handles can be closed easily as it follows:

```cpp
loop->walk([](auto &&h){ h.close(); });
```

No need to keep track of them.

## The event-based approach

`uvw` offers an event-based approach, so resources are small event emitters
to which listeners can be attached.<br/>
Attaching a listener to a resource is the recommended way to be notified about
changes.<br/>
Listeners must be callable objects of type `void(EventType &, ResourceType &)`,
where:

* `EventType` is the type of the event for which they have been designed.
* `ResourceType` is the type of the resource that has originated the event.

It means that the following function types are all valid:

* `void(EventType &, ResourceType &)`
* `void(const EventType &, ResourceType &)`
* `void(EventType &, const ResourceType &)`
* `void(const EventType &, const ResourceType &)`

Please note that there is no need to keep around references to the resources:
they will pass themselves as an argument whenever an event is published.

There exist two methods to attach a listener to a resource:

* `resource.once<EventType>(listener)`: the listener will be automatically
  removed after the first event of the given type.
* `resource.on<EventType>(listener)`: to be used for long-running listeners.

Both of them return an object of type `ResourceType::Connection` (as an example,
`TCPHandle::Connection`).<br/>
A connection object can be used later as an argument to the `erase` member
function of the resource to remove the listener.<br/>
There exists also the `clear` member function to drop all the listeners at once.
Note that `clear` should only be invoked on non-active handles. The handles
exploit the same event mechanism made available to users to satisfy pending
requests. Invoking `clear` on an active handle, for example with requests still
in progress, risks leading to memory leaks or unexpected behavior.

Almost all the resources emit `ErrorEvent` in case of errors.<br/>
All the other events are specific for the given resource and documented in the
API reference.

The code below shows how to create a simple tcp server using `uvw`:

```cpp
auto loop = uvw::Loop::getDefault();
auto tcp = loop->resource<uvw::TCPHandle>();

tcp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::TCPHandle &) { /* something went wrong */ });

tcp->on<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::TCPHandle &srv) {
    std::shared_ptr<uvw::TCPHandle> client = srv.loop().resource<uvw::TCPHandle>();
    client->once<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TCPHandle &client) { client.close(); });
    client->on<uvw::DataEvent>([](const uvw::DataEvent &, uvw::TCPHandle &) { /* data received */ });
    srv.accept(*client);
    client->read();
});

tcp->bind("127.0.0.1", 4242);
tcp->listen();
```

Note also that `uvw::TCPHandle` already supports _IPv6_ out-of-the-box. The
statement above is equivalent to `tcp->bind<uvw::IPv4>("127.0.0.1", 4242)`.<br/>
It's sufficient to explicitly specify `uvw::IPv6` as the underlying protocol to
use it.

The API reference is the recommended documentation for further details about
resources and their methods.

## Going raw

In case users need to use functionalities not wrapped yet by `uvw` or if they
want to get the underlying data structures as defined by `libuv` for some other
reasons, almost all the classes in `uvw` give direct access to them.<br/>
Please, note that this functions should not be used directly unless users know
exactly what they are doing and what are the risks. Going raw is dangerous,
mainly because the lifetime management of a loop, a handle or a request is
completely controlled by the library and working around it could quickly break
things.

That being said, _going raw_ is a matter of using the `raw` member functions:

```cpp
auto loop = uvw::Loop::getDefault();
auto tcp = loop->resource<uvw::TCPHandle>();

uv_loop_t *raw = loop->raw();
uv_tcp_t *handle = tcp->raw();
```

Go the raw way at your own risk, but do not expect any support in case of bugs.

# Contributors

If you want to contribute, please send patches as pull requests against the
branch master.<br/>
Check the
[contributors list](https://github.com/skypjack/uvw/blob/master/AUTHORS) to see
who has partecipated so far.

# License

Code and documentation Copyright (c) 2016-2022 Michele Caini.<br/>
Logo Copyright (c) 2018-2021 Richard Caseres.

Code and documentation released under
[the MIT license](https://github.com/skypjack/uvw/blob/master/LICENSE).<br/>
Logo released under
[CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).

<!--
@cond TURN_OFF_DOXYGEN
-->
# Support

If you want to support this project, you can
[offer me](https://github.com/users/skypjack/sponsorship) an espresso.<br/>
If you find that it's not enough, feel free to
[help me](https://www.paypal.me/skypjack) the way you prefer.
<!--
@endcond TURN_OFF_DOXYGEN
-->
