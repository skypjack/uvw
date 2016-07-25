#pragma once


#include <iterator>
#include <cstddef>
#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "handle.hpp"
#include "util.hpp"


namespace uvw {


namespace details {


class Connect final: public Request<Connect, uv_connect_t> {
    using Request::Request;

public:
    template<typename... Args>
    static std::shared_ptr<Connect> create(Args&&... args) {
        return std::shared_ptr<Connect>{new Connect{std::forward<Args>(args)...}};
    }

    template<typename F, typename... Args>
    void connect(F &&f, Args... args) {
        exec<uv_connect_t, ConnectEvent>(std::forward<F>(f), get<uv_connect_t>(), std::forward<Args>(args)...);
    }
};


class Shutdown final: public Request<Shutdown, uv_shutdown_t> {
    using Request::Request;

public:
    template<typename... Args>
    static std::shared_ptr<Shutdown> create(Args&&... args) {
        return std::shared_ptr<Shutdown>{new Shutdown{std::forward<Args>(args)...}};
    }

    void shutdown(uv_stream_t *handle) {
        exec<uv_shutdown_t, ShutdownEvent>(&uv_shutdown, get<uv_shutdown_t>(), handle);
    }
};


class Write final: public Request<Write, uv_write_t> {
    using Request::Request;

public:
    template<typename... Args>
    static std::shared_ptr<Write> create(Args&&... args) {
        return std::shared_ptr<Write>{new Write{std::forward<Args>(args)...}};
    }

    void write(uv_stream_t *handle, const uv_buf_t bufs[], unsigned int nbufs) {
        exec<uv_write_t, WriteEvent>(&uv_write, get<uv_write_t>(), handle, bufs, nbufs);
    }

    void write(uv_stream_t *handle, const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t *send) {
        exec<uv_write_t, WriteEvent>(&uv_write2, get<uv_write_t>(), handle, bufs, nbufs, send);
    }
};


}


template<typename T, typename U>
class Stream: public Handle<T, U> {
    static constexpr unsigned int DEFAULT_BACKLOG = 128;

    static void readCallback(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) {
        T &ref = *(static_cast<T*>(handle->data));
        // data will be destroyed no matter of what the value of nread is
        std::unique_ptr<const char[]> data{buf->base};

        if(nread == UV_EOF) {
            // end of stream
            ref.publish(EndEvent{});
        } else if(nread > 0) {
            // data available
            ref.publish(DataEvent{std::move(data), nread});
        } else {
            // transmission error
            ref.publish(ErrorEvent(nread));
        }
    }

    static void listenCallback(uv_stream_t *handle, int status) {
        T &ref = *(static_cast<T*>(handle->data));
        if(status) { ref.publish(ErrorEvent{status}); }
        else { ref.publish(ListenEvent{}); }
    }

protected:
    using Handle<T, U>::Handle;

public:
    void shutdown() {
        auto listener = [ptr = this->shared_from_this()](const auto &event, details::Shutdown &) {
            ptr->publish(event);
        };

        auto shutdown = this->loop().template resource<details::Shutdown>();
        shutdown->template once<ErrorEvent>(listener);
        shutdown->template once<ShutdownEvent>(listener);
        shutdown->shutdown(this->template get<uv_stream_t>());
    }

    void listen(int backlog) {
        this->invoke(&uv_listen, this->template get<uv_stream_t>(), backlog, &listenCallback);
    }

    void listen() {
        listen(DEFAULT_BACKLOG);
    }

    template<typename S>
    void accept(S &ref) {
        this->invoke(&uv_accept, this->template get<uv_stream_t>(), ref.template get<uv_stream_t>());
    }

    void read() {
        this->invoke(&uv_read_start, this->template get<uv_stream_t>(), &this->allocCallback, &readCallback);
    }

    void stop() {
        this->invoke(&uv_read_stop, this->template get<uv_stream_t>());
    }

    void write(char *data, ssize_t len) {
        uv_buf_t bufs[] = { uv_buf_init(data, len) };

        auto listener = [ptr = this->shared_from_this()](const auto &event, details::Write &) {
            ptr->publish(event);
        };

        auto write = this->loop().template resource<details::Write>();
        write->template once<ErrorEvent>(listener);
        write->template once<WriteEvent>(listener);
        write->write(this->template get<uv_stream_t>(), bufs, 1);
    }

    void write(std::unique_ptr<char[]> data, ssize_t len) {
        write(data.get(), len);
    }

    template<typename S>
    void write(S &send, char *data, ssize_t len) {
        uv_buf_t bufs[] = { uv_buf_init(data, len) };

        auto listener = [ptr = this->shared_from_this()](const auto &event, details::Write &) {
            ptr->publish(event);
        };

        auto write = this->loop().template resource<details::Write>();
        write->template once<ErrorEvent>(listener);
        write->template once<WriteEvent>(listener);
        write->write(this->template get<uv_stream_t>(), bufs, 1, send.template get<uv_stream_t>());
    }

    template<typename S>
    void write(S &send, std::unique_ptr<char[]> data, ssize_t len) {
        write(send, data.get(), len);
    }

    int tryWrite(char *data, ssize_t len) {
        uv_buf_t bufs[] = { uv_buf_init(data, len) };
        auto bw = uv_try_write(this->template get<uv_stream_t>(), bufs, 1);

        if(bw < 0) {
            this->publish(ErrorEvent{bw});
            bw = 0;
        }

        return bw;
    }

    int tryWrite(std::unique_ptr<char[]> data, ssize_t len) {
        return tryWrite(data.get(), len);
    }

    bool readable() const noexcept {
        return (uv_is_readable(this->template get<uv_stream_t>()) == 1);
    }

    bool writable() const noexcept {
        return (uv_is_writable(this->template get<uv_stream_t>()) == 1);
    }

    void blocking(bool enable = false) {
        this->invoke(&uv_stream_set_blocking, this->template get<uv_stream_t>(), enable);
    }
};


}
