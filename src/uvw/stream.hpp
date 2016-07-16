#pragma once


#include <iterator>
#include <cstddef>
#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "util.hpp"
#include "shutdown.hpp"


namespace uvw {


template<typename T>
class Stream: public Handle<T> {
    static constexpr unsigned int DEFAULT_BACKLOG = 128;

    static void allocCallback(uv_handle_t *, std::size_t suggested, uv_buf_t *buf) {
        *buf = uv_buf_init(new char[suggested], suggested);
    }

    static void readCallback(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) {
        T &ref = *(static_cast<T*>(handle->data));
        // data will be destroyed no matter of what the value of nread is
        std::unique_ptr<const char[]> data{buf->base};

        if(nread == UV_EOF) {
            ref.publish(EndEvent{});
        } else if(nread > 0) {
            ref.publish(DataEvent{std::move(data), nread});
        } else {
            ref.publish(ErrorEvent(nread));
        }
    }

    static void writeCallback(uv_write_t *req, int status) {
        // TODO migrate to Request (see request.hpp for further details)
        T &ref = *(static_cast<T*>(req->handle->data));
        if(status) { ref.publish(ErrorEvent{status}); }
        else { ref.publish(WriteEvent{}); }
        delete req;
    }

    static void listenCallback(uv_stream_t *handle, int status) {
        T &ref = *(static_cast<T*>(handle->data));
        if(status) ref.publish(ErrorEvent{status});
        else ref.publish(ListenEvent{});
    }

protected:
    template<typename U>
    Stream(HandleType<U> rt, std::shared_ptr<Loop> ref)
        : Handle<T>{std::move(rt), std::move(ref)}
    { }

public:
    void shutdown() noexcept {
        std::weak_ptr<T> weak = this->shared_from_this();

        auto listener = [weak](const auto &event, Shutdown &) {
            auto ptr = weak.lock();
            if(ptr) { ptr->publish(event); }
        };

        auto shutdown = this->loop()->template resource<Shutdown>();
        shutdown->template once<ErrorEvent>(listener);
        shutdown->template once<ShutdownEvent>(listener);
        shutdown->shutdown(*this);
    }

    void listen(int backlog) noexcept {
        this->invoke(&uv_listen, this->template get<uv_stream_t>(), backlog, &listenCallback);
    }

    void listen() noexcept {
        listen(DEFAULT_BACKLOG);
    }

    void read() {
        this->invoke(&uv_read_start, this->template get<uv_stream_t>(), &allocCallback, &readCallback);
    }

    void stop() noexcept {
        this->invoke(&uv_read_stop, this->template get<uv_stream_t>());
    }

    void write(char *data, ssize_t length) {
        uv_buf_t bufs[] = { uv_buf_init(data, length) };
        // TODO migrate to Request (see request.hpp for further details)
        uv_write_t *req = new uv_write_t;

        auto err = uv_write(req, this->template get<uv_stream_t>(), bufs, 1, &Stream<T>::writeCallback);

        if(err) {
            delete req;
            this->publish(ErrorEvent{err});
        }
    }

    void write(std::unique_ptr<char[]> data, ssize_t length) {
        write(data.get(), length);
    }

    int tryWrite(char *data, ssize_t length) noexcept {
        uv_buf_t bufs[] = { uv_buf_init(data, length) };
        auto bw = uv_try_write(this->template get<uv_stream_t>(), bufs, 1);

        if(bw < 0) {
            this->publish(ErrorEvent{bw});
            bw = 0;
        }

        return bw;
    }

    int tryWrite(std::unique_ptr<char[]> data, ssize_t length) noexcept {
        return tryWrite(data.get(), length);
    }

    bool readable() const noexcept {
        return (uv_is_readable(this->template get<uv_stream_t>()) == 1);
    }

    bool writable() const noexcept {
        return (uv_is_writable(this->template get<uv_stream_t>()) == 1);
    }
};


}
