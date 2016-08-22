#pragma once


#include <iterator>
#include <cstddef>
#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "handle.hpp"


namespace uvw {


/**
 * @brief ConnectEvent event.
 *
 * It will be emitted by StreamHandle according with its functionalities.
 */
struct ConnectEvent: Event<ConnectEvent> { };


/**
 * @brief EndEvent event.
 *
 * It will be emitted by StreamHandle according with its functionalities.
 */
struct EndEvent: Event<EndEvent> { };


/**
 * @brief ListenEvent event.
 *
 * It will be emitted by StreamHandle according with its functionalities.
 */
struct ListenEvent: Event<ListenEvent> { };


/**
 * @brief ShutdownEvent event.
 *
 * It will be emitted by StreamHandle according with its functionalities.
 */
struct ShutdownEvent: Event<ShutdownEvent> { };


/**
 * @brief WriteEvent event.
 *
 * It will be emitted by StreamHandle according with its functionalities.
 */
struct WriteEvent: Event<WriteEvent> { };


/**
 * @brief DataEvent event.
 *
 * It will be emitted by StreamHandle according with its functionalities.
 */
struct DataEvent: Event<DataEvent> {
    explicit DataEvent(std::unique_ptr<const char[]> ptr, ssize_t l) noexcept
        : dt{std::move(ptr)}, len{l}
    { }

    /**
     * @brief Gets the data read on the stream.
     * @return A bunch of data read on the stream.
     */
    const char * data() const noexcept { return dt.get(); }

    /**
     * @brief Gets the amount of data read on the stream.
     * @return The amount of data read on the stream.
     */
    ssize_t length() const noexcept { return len; }

private:
    std::unique_ptr<const char[]> dt;
    const ssize_t len;
};


namespace details {


class ConnectReq final: public Request<ConnectReq, uv_connect_t> {
    using Request::Request;

public:
    template<typename... Args>
    static std::shared_ptr<ConnectReq> create(Args&&... args) {
        return std::shared_ptr<ConnectReq>{new ConnectReq{std::forward<Args>(args)...}};
    }

    template<typename F, typename... Args>
    void connect(F &&f, Args... args) {
        invoke(std::forward<F>(f), get<uv_connect_t>(), std::forward<Args>(args)..., &defaultCallback<uv_connect_t, ConnectEvent>);
    }
};


class ShutdownReq final: public Request<ShutdownReq, uv_shutdown_t> {
    using Request::Request;

public:
    template<typename... Args>
    static std::shared_ptr<ShutdownReq> create(Args&&... args) {
        return std::shared_ptr<ShutdownReq>{new ShutdownReq{std::forward<Args>(args)...}};
    }

    void shutdown(uv_stream_t *handle) {
        invoke(&uv_shutdown, get<uv_shutdown_t>(), handle, &defaultCallback<uv_shutdown_t, ShutdownEvent>);
    }
};


class WriteReq final: public Request<WriteReq, uv_write_t> {
    using Request::Request;

public:
    template<typename... Args>
    static std::shared_ptr<WriteReq> create(Args&&... args) {
        return std::shared_ptr<WriteReq>{new WriteReq{std::forward<Args>(args)...}};
    }

    void write(uv_stream_t *handle, const uv_buf_t bufs[], unsigned int nbufs) {
        invoke(&uv_write, get<uv_write_t>(), handle, bufs, nbufs, &defaultCallback<uv_write_t, WriteEvent>);
    }

    void write(uv_stream_t *handle, const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t *send) {
        invoke(&uv_write2, get<uv_write_t>(), handle, bufs, nbufs, send, &defaultCallback<uv_write_t, WriteEvent>);
    }
};


}


/**
 * @brief The StreamHandle handle.
 *
 * Stream handles provide an abstraction of a duplex communication channel.
 * StreamHandle is an intermediate type, `uvw` provides three stream
 * implementations: TcpHandle, PipeHandle and TTYHandle.
 */
template<typename T, typename U>
class StreamHandle: public Handle<T, U> {
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
    /**
     * @brief Shutdowns the outgoing (write) side of a duplex stream.
     *
     * It waits for pending write requests to complete. The handle should refer
     * to a initialized stream.<br/>
     * A ShutdownEvent event will be emitted after shutdown is complete.
     */
    void shutdown() {
        auto listener = [ptr = this->shared_from_this()](const auto &event, details::ShutdownReq &) {
            ptr->publish(event);
        };

        auto shutdown = this->loop().template resource<details::ShutdownReq>();
        shutdown->template once<ErrorEvent>(listener);
        shutdown->template once<ShutdownEvent>(listener);
        shutdown->shutdown(this->template get<uv_stream_t>());
    }

    /**
     * @brief Starts listening for incoming connections.
     *
     * When a new incoming connection is received, a ConnectEvent event is
     * emitted.<br/>
     * An ErrorEvent event will be emitted in case of errors.
     *
     * @param backlog Indicates the number of connections the kernel might
     * queue, same as listen(2).
     */
    void listen(int backlog = DEFAULT_BACKLOG) {
        this->invoke(&uv_listen, this->template get<uv_stream_t>(), backlog, &listenCallback);
    }

    /**
     * @brief Accepts incoming connections.
     *
     * This call is used in conjunction with `listen()` to accept incoming
     * connections. Call this function after receiving a ConnectEvent event to
     * accept the connection. Before calling this function, the submitted handle
     * must be initialized.<br>
     * An ErrorEvent event will be emitted in case of errors.
     *
     * When the ConnectEvent event is emitted it is guaranteed that this
     * function will complete successfully the first time. If you attempt to use
     * it more than once, it may fail.<br/>
     * It is suggested to only call this function once per ConnectEvent event.
     *
     * **Note**: both the handles must be running on the same loop.
     *
     * @param ref An initialized handle to be used to accept the connection.
     */
    template<typename S>
    void accept(S &ref) {
        this->invoke(&uv_accept, this->template get<uv_stream_t>(), ref.template get<uv_stream_t>());
    }

    /**
     * @brief Starts reading data from an incoming stream.
     *
     * A ReadEvent event will be emitted several times until there is no more
     * data to read or `stop()` is called.<br/>
     * An EndEvent event will be emitted when there is no more data to read.
     */
    void read() {
        this->invoke(&uv_read_start, this->template get<uv_stream_t>(), &this->allocCallback, &readCallback);
    }

    /**
     * @brief Stops reading data from the stream.
     *
     * This function is idempotent and may be safely called on a stopped stream.
     */
    void stop() {
        this->invoke(&uv_read_stop, this->template get<uv_stream_t>());
    }

    /**
     * @brief Writes data to the stream.
     *
     * Data are written in order.<br/>
     * A WriteEvent event will be emitted when the data have been written.<br/>
     * An ErrorEvent event will be emitted in case of errors.
     *
     * @param data The data to be written to the stream.
     * @param len The lenght of the submitted data.
     */
    void write(std::unique_ptr<char[]> data, ssize_t len) {
        uv_buf_t bufs[] = { uv_buf_init(data.get(), len) };

        auto listener = [ptr = this->shared_from_this()](const auto &event, details::WriteReq &) {
            ptr->publish(event);
        };

        auto write = this->loop().template resource<details::WriteReq>();
        write->template once<ErrorEvent>(listener);
        write->template once<WriteEvent>(listener);
        write->write(this->template get<uv_stream_t>(), bufs, 1);
    }


    /**
     * @brief Extended write function for sending handles over a pipe handle.
     *
     * The pipe must be initialized with `ipc == true`.
     *
     * `send` must be a TcpHandle or PipeHandle handle, which is a server or a
     * connection (listening or connected state). Bound sockets or pipes will be
     * assumed to be servers.
     *
     * A WriteEvent event will be emitted when the data have been written.<br/>
     * An ErrorEvent wvent will be emitted in case of errors.
     *
     * @param send The handle over which to write data.
     * @param data The data to be written to the stream.
     * @param len The lenght of the submitted data.
     */
    template<typename S>
    void write(S &send, std::unique_ptr<char[]> data, ssize_t len) {
        uv_buf_t bufs[] = { uv_buf_init(data.get(), len) };

        auto listener = [ptr = this->shared_from_this()](const auto &event, details::WriteReq &) {
            ptr->publish(event);
        };

        auto write = this->loop().template resource<details::WriteReq>();
        write->template once<ErrorEvent>(listener);
        write->template once<WriteEvent>(listener);
        write->write(this->template get<uv_stream_t>(), bufs, 1, send.template get<uv_stream_t>());
    }

    /**
     * @brief Queues a write request if it can be completed immediately.
     *
     * Same as `write()`, but won’t queue a write request if it can’t be
     * completed immediately.<br/>
     * An ErrorEvent event will be emitted in case of errors.
     *
     * @param data The data to be written to the stream.
     * @param len The lenght of the submitted data.
     * @return Number of bytes written.
     */
    int tryWrite(std::unique_ptr<char[]> data, ssize_t len) {
        uv_buf_t bufs[] = { uv_buf_init(data.get(), len) };
        auto bw = uv_try_write(this->template get<uv_stream_t>(), bufs, 1);

        if(bw < 0) {
            this->publish(ErrorEvent{bw});
            bw = 0;
        }

        return bw;
    }

    /**
     * @brief Checks if the stream is readable.
     * @return True if the stream is readable, false otherwise.
     */
    bool readable() const noexcept {
        return (uv_is_readable(this->template get<uv_stream_t>()) == 1);
    }

    /**
     * @brief Checks if the stream is writable.
     * @return True if the stream is writable, false otherwise.
     */
    bool writable() const noexcept {
        return (uv_is_writable(this->template get<uv_stream_t>()) == 1);
    }

    /**
     * @brief Enables or disables blocking mode for a stream.
     *
     * When blocking mode is enabled all writes complete synchronously. The
     * interface remains unchanged otherwise, e.g. completion or failure of the
     * operation will still be reported through events which are emitted
     * asynchronously.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/stream.html#c.uv_stream_set_blocking)
     * for further details.
     *
     * @param enable True to enable blocking mode, false otherwise.
     */
    void blocking(bool enable = false) {
        this->invoke(&uv_stream_set_blocking, this->template get<uv_stream_t>(), enable);
    }
};


}
