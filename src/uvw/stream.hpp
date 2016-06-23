#pragma once


#include <uv.h>
#include "resource.hpp"
#include "loop.hpp"


namespace uvw {


template<typename T>
class Stream: public Resource<T> {
    static void protoListen(uv_stream_t* srv, int status) {
        Stream &stream = *(static_cast<Stream*>(srv->data));

        if(status) {
            stream.listenCallback(UVWError{status});
        } else {
            stream.tryAccept();
        }
    }

    void tryAccept() const noexcept {
        Handle<Stream> handle = accept();

        if(handle) {
            // TODO invoke cb with handle
        } else {
            // TODO invoke cb with error
        }
    }

    virtual Handle<Stream> accept() const noexcept = 0;

protected:
    using Resource<T>::Resource;

public:
    using CallbackListen = std::function<void(UVWError)>;

    // TODO shutdown

    void listen(int backlog, CallbackListen cb) noexcept {
        listenCallback = cb;
        this->template get<uv_stream_t>()->data = this;
        auto err = uv_listen(this->template get<uv_stream_t>(), backlog, &protoListen);

        if(err) {
            listenCallback(UVWError{err});
        }
    }

    // TODO read
    // TODO stop
    // TODO write
    // TODO tryWrite

    bool readable() const noexcept {
        return (uv_is_readable(this->template get<uv_stream_t>()) == 1);
    }

    bool writable() const noexcept {
        return (uv_is_writable(this->template get<uv_stream_t>()) == 1);
    }

private:
    CallbackListen listenCallback;
};


}
