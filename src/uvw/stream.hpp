#pragma once


#include <uv.h>
#include "resource.hpp"
#include "loop.hpp"


namespace uvw {


class Connection final: public Resource {
    // TODO proxy on a stream client

public:
    explicit Connection(std::shared_ptr<Loop> ref, uv_stream_t *srv)
        : Resource{HandleType<uv_stream_t>{}, ref}
    {
        // TODO initialized... HOW????
        uv_tcp_init(parent(), get<uv_tcp_t>());get<uv_stream_t>()
        auto err = uv_accept(srv, get<uv_stream_t>());
    }
};


class Stream: public Resource {
    static void protoListen(uv_stream_t* srv, int status) {
        // TODO invoke accept

        Stream &stream = *(static_cast<Stream*>(srv->data));

        if(status) {
            stream.listenCallback(UVWError{status}, Handle<Connection>{});
        } else {
            stream.listenCallback(UVWError{}, loop()->handle(get<uv_stream_t>()));
        }
    }

protected:
    using Resource::Resource;

public:
    using CallbackListen = std::function<void(UVWError, Handle<Connection>)>;

    // TODO shutdown

    void listen(int backlog, CallbackListen cb) noexcept {
        listenCallback = cb;
        auto err = uv_listen(get<uv_stream_t>(), backlog, &protoListen);

        if(err) {
            listenCallback(UVWError{err}, Handle<Connection>{});
        }
    }

    // TODO read
    // TODO stop
    // TODO write
    // TODO tryWrite

    bool readable() const noexcept {
        return (uv_is_readable(get<uv_stream_t>()) == 1);
    }

    bool writable() const noexcept {
        return (uv_is_writable(get<uv_stream_t>()) == 1);
    }

private:
    CallbackListen listenCallback;
};


}
