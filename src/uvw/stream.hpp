#pragma once


#include <iterator>
#include <cstddef>
#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


template<typename T>
class Stream: public Handle<T> {
    static constexpr unsigned int DEFAULT_BACKLOG = 128;

    static void allocCallback(T &, uv_handle_t *, std::size_t suggested, uv_buf_t *buf) {
        buf->base = new char[suggested];
        buf->len = suggested;
    }

    static void readCallback(T &ref, uv_stream_t *, ssize_t nread, const uv_buf_t *cbuf) {
        uv_buf_t *buf = const_cast<uv_buf_t *>(cbuf);

        if(nread == UV_EOF) {
            ref.publish(EndEvent{});
            delete[] buf->base;
        } else if(nread > 0) {
            std::unique_ptr<char[]> data{buf->base};
            DataEvent event;
            event.buffer.reset(std::move(data), nread);
            ref.publish(std::move(event));
        } else {
            ref.publish(ErrorEvent(nread));
            delete[] buf->base;
        }

        buf->base = nullptr;
        buf->len = 0;
    }

    static void writeCallback(T &ref, uv_write_t *req, int status) {
        if(status) {
            ref.publish(ErrorEvent{status});
        } else {
            ref.publish(WriteEvent{});
        }

        delete req;
    }

    static void shutdownCallback(T &ref, uv_shutdown_t *, int status) {
        if(status) ref.publish(ErrorEvent{status});
        else ref.publish(ShutdownEvent{});
    }

    static void listenCallback(T &ref, uv_stream_t *, int status) {
        if(status) ref.publish(ErrorEvent{status});
        else ref.publish(ListenEvent{});
    }

protected:
    template<typename U>
    Stream(HandleType<U> rt, std::shared_ptr<Loop> ref)
        : Handle<T>{std::move(rt), std::move(ref)}, sdown{std::make_unique<uv_shutdown_t>()}
    { }

public:
    void shutdown() noexcept {
        using CBF = typename Handle<T>::template CallbackFactory<void(uv_shutdown_t *, int)>;
        auto func = &CBF::template proto<&Stream<T>::shutdownCallback>;
        auto err = uv_shutdown(sdown.get(), this->template get<uv_stream_t>(), func);
        if(err) this->publish(ErrorEvent{err});
    }

    void listen(int backlog) noexcept {
        using CBF = typename Handle<T>::template CallbackFactory<void(uv_stream_t *, int)>;
        auto func = &CBF::template proto<&Stream<T>::listenCallback>;
        auto err = uv_listen(this->template get<uv_stream_t>(), backlog, func);
        if(err) this->publish(ErrorEvent{err});
    }

    void listen() noexcept {
        listen(DEFAULT_BACKLOG);
    }

    void read() {
        using CBFAlloc = typename Handle<T>::template CallbackFactory<void(uv_handle_t *, std::size_t, uv_buf_t *)>;
        using CBFRead = typename Handle<T>::template CallbackFactory<void(uv_stream_t *, ssize_t, const uv_buf_t *)>;
        auto allocFunc = &CBFAlloc::template proto<&Stream<T>::allocCallback>;
        auto readFunc = &CBFRead::template proto<&Stream<T>::readCallback>;
        auto err = uv_read_start(this->template get<uv_stream_t>(), allocFunc, readFunc);
        if(err) this->publish(ErrorEvent{err});
    }

    void stop() noexcept {
        auto err = uv_read_stop(this->template get<uv_stream_t>());
        if(err) this->publish(ErrorEvent{err});
    }

    void write(Buffer buf) {
        using CBF = typename Handle<T>::template CallbackFactory<void(uv_write_t *, int)>;
        auto func = &CBF::template proto<&Stream<T>::writeCallback>;
        uv_buf_t data[] = { buf.uvBuf() };
        uv_write_t *req = new uv_write_t;
        auto err = uv_write(req, this->template get<uv_stream_t>(), data, 1, func);
        if(err) {
            delete req;
            this->publish(ErrorEvent{err});
        }
    }

    int tryWrite(Buffer buf) noexcept {
        uv_buf_t data[] = { buf.uvBuf() };
        auto bw = uv_try_write(this->template get<uv_stream_t>(), data, 1);

        if(bw < 0) {
            this->publish(ErrorEvent{bw});
            bw = 0;
        }

        return bw;
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
