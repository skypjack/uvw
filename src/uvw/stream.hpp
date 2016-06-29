#pragma once


#include <iterator>
#include <cstddef>
#include <memory>
#include <array>
#include <uv.h>
#include "handle.hpp"
#include "loop.hpp"


namespace uvw {


template<typename T>
class Stream;


class Buffer final {
    template<typename>
    friend class Stream;

    uv_buf_t uvBuf() const noexcept {
        return uv_buf_init(data.get(), size);
    }

public:
    Buffer(std::unique_ptr<char[]> dt, std::size_t s)
        : data{std::move(dt)}, size{s}
    { }

    Buffer(Buffer &&) = default;
    Buffer& operator=(Buffer &&) = default;

    void reset(std::unique_ptr<char[]> dt, std::size_t s) noexcept {
        data.swap(dt);
        size = s;
    }

private:
    std::unique_ptr<char[]> data;
    std::size_t size;
};


template<typename T>
class Stream: public Handle<T> {
    static constexpr unsigned int DEFAULT_BACKLOG = 128;

    static void shutdownCallback(T &t, std::function<void(UVWError, T &)> &cb, uv_shutdown_t *, int status) {
        cb(UVWError{status}, t);
    }

    static void listenCallback(T &t, std::function<void(UVWError, T &)> &cb, uv_stream_t *, int status) {
        cb(UVWError{status}, t);
    }

protected:
    using Handle<T>::Handle;

public:
    void shutdown(std::function<void(UVWError, T &)> cb) noexcept {
        using CBF = typename Handle<T>::template CallbackFactory<void(uv_shutdown_t *, int)>;
        auto func = CBF::template on<&Stream<T>::shutdownCallback>(*static_cast<T*>(this), cb);
        auto err = uv_shutdown(sdown.get(), this->template get<uv_stream_t>(), func);
        if(err) { Stream<T>::error(err); }
    }

    void listen(int backlog, std::function<void(UVWError, T &)> cb) noexcept {
        using CBF = typename Handle<T>::template CallbackFactory<void(uv_stream_t *, int)>;
        auto func = CBF::template on<&Stream<T>::listenCallback>(*static_cast<T*>(this), cb);
        auto err = uv_listen(this->template get<uv_stream_t>(), backlog, func);
        if(err) { Stream<T>::error(err); }
    }

    void listen(std::function<void(UVWError, T &)> cb) noexcept {
        listen(DEFAULT_BACKLOG, std::move(cb));
    }

    // TODO read

    UVWError stop() noexcept {
        return UVWError{uv_read_stop(this->template get<uv_stream_t>())};
    }

    // TODO write

    UVWOptionalData<int> tryWrite(Buffer buf) noexcept {
        Buffer data[] = { std::move(buf) };
        return tryWrite(std::begin(data), std::end(data));
    }

    template<typename It>
    UVWOptionalData<int> tryWrite(It first, It last) noexcept {
        uv_buf_t data[last - first];
        std::size_t pos = 0;
        while(first != last) { data[pos++] = (first++)->uvBuf(); }
        auto bw = uv_try_write(this->template get<uv_stream_t>(), data, pos);
        return (bw >= 0 ? bw : UVWError{bw});
    }

    bool readable() const noexcept {
        return (uv_is_readable(this->template get<uv_stream_t>()) == 1);
    }

    bool writable() const noexcept {
        return (uv_is_writable(this->template get<uv_stream_t>()) == 1);
    }

private:
    std::unique_ptr<uv_shutdown_t> sdown;
};


}
