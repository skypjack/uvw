#pragma once


#include <uv.h>
#include "resource.hpp"
#include "loop.hpp"


namespace uvw {


template<typename T>
class Stream: public Resource<T> {
    static void listenCallback(T &t, std::function<void(UVWError, T &)> &cb, uv_stream_t *, int status) {
        cb(UVWError{status}, t);
    }

protected:
    using Resource<T>::Resource;

public:
    // TODO shutdown

    void listen(int backlog, std::function<void(UVWError, T &)> cb) noexcept {
        using CB = typename Resource<T>::template Callback<void(uv_stream_t *, int)>;
        auto func = CB::on<&Stream<T>::listenCallback>(*static_cast<T*>(this), cb);
        auto err = uv_listen(this->template get<uv_stream_t>(), backlog, func);
        if(err) { cb(UVWError{err}, *static_cast<T*>(this)); }
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
    std::function<void(UVWError)> listenCb;
};


}
