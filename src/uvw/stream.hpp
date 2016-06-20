#pragma once


namespace uvw {


template<class T>
class Connection: public Resource<T> {
    using Resource<T>::Resource;

public:
    // TODO read
    // TODO stop
    // TODO write
    // TODO tryWrite
};


template<class T>
class Stream: public Connection<T> {
    using Connection<T>::Connection;

    static void protoListen(uv_stream_t* srv, int status) {
        // TODO
    }

protected:
    template<typename U>
    explicit Stream(U *u)
        : Connection<T>{u},
          handle{reinterpret_cast<uv_stream_t*>(u)}
    { }

public:
    // TODO shutdown
    // TODO listen

    bool readable() const noexcept {
        return (uv_is_readable(handle) == 1);
    }

    bool writable() const noexcept {
        return (uv_is_writable(handle) == 1);
    }

private:
    uv_stream_t *handle;
};


}
