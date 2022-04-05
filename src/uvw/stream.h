#ifndef UVW_STREAM_INCLUDE_H
#define UVW_STREAM_INCLUDE_H

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>
#include <uv.h>
#include "config.h"
#include "handle.hpp"
#include "loop.h"
#include "request.hpp"

namespace uvw {

/*! @brief Connect event. */
struct connect_event {};

/*! @brief End event. */
struct end_event {};

/*! @brief Listen event. */
struct listen_event {};

/*! @brief Shutdown event. */
struct shutdown_event {};

/*! @brief Write event. */
struct write_event {};

/*! @brief Data event. */
struct data_event {
    explicit data_event(std::unique_ptr<char[]> buf, std::size_t len) UVW_NOEXCEPT;

    std::unique_ptr<char[]> data; /*!< A bunch of data read on the stream. */
    std::size_t length;           /*!< The amount of data read on the stream. */
};

namespace details {

class connect_req final: public request<connect_req, uv_connect_t, connect_event> {
    static void connect_callback(uv_connect_t *req, int status);

public:
    using request::request;

    template<typename F, typename... Args>
    auto connect(F &&f, Args &&...args) -> std::enable_if_t<std::is_same_v<decltype(std::forward<F>(f)(raw(), std::forward<Args>(args)..., &connect_callback)), void>> {
        std::forward<F>(f)(raw(), std::forward<Args>(args)..., &connect_callback);
        this->leak_if(0);
    }

    template<typename F, typename... Args>
    auto connect(F &&f, Args &&...args) -> std::enable_if_t<!std::is_same_v<decltype(std::forward<F>(f)(raw(), std::forward<Args>(args)..., &connect_callback)), void>> {
        if(auto err = this->leak_if(std::forward<F>(f)(raw(), std::forward<Args>(args)..., &connect_callback)); err) {
            publish(error_event{err});
        }
    }
};

class shutdown_req final: public request<shutdown_req, uv_shutdown_t, shutdown_event> {
    static void shoutdown_callback(uv_shutdown_t *req, int status);

public:
    using request::request;

    void shutdown(uv_stream_t *hndl);
};

template<typename Deleter>
class write_req final: public request<write_req<Deleter>, uv_write_t, write_event> {
    static void write_callback(uv_write_t *req, int status) {
        if(auto ptr = request<write_req<Deleter>, uv_write_t, write_event>::reserve(req); status) {
            ptr->publish(error_event{status});
        } else {
            ptr->publish(write_event{});
        }
    }

public:
    write_req(loop::token token, std::shared_ptr<loop> parent, std::unique_ptr<char[], Deleter> dt, unsigned int len)
        : request<write_req<Deleter>, uv_write_t, write_event>{token, std::move(parent)},
          data{std::move(dt)},
          buf{uv_buf_init(data.get(), len)} {}

    void write(uv_stream_t *hndl) {
        if(auto err = this->leak_if(uv_write(this->raw(), hndl, &buf, 1, &write_callback)); err != 0) {
            this->publish(error_event{err});
        }
    }

    void write(uv_stream_t *hndl, uv_stream_t *send) {
        if(auto err = this->leak_if(uv_write2(this->raw(), hndl, &buf, 1, send, &write_callback)); err != 0) {
            this->publish(error_event{err});
        }
    }

private:
    std::unique_ptr<char[], Deleter> data;
    uv_buf_t buf;
};

} // namespace details

/**
 * @brief The stream handle.
 *
 * Stream handles provide an abstraction of a duplex communication channel.
 * The stream handle is an intermediate type, `uvw` provides three stream
 * implementations: tcp, pipe and tty handles.
 */
template<typename T, typename U, typename... E>
class stream_handle: public handle<T, U, write_event, E...> {
    template<typename, typename, typename...>
    friend class stream_handle;

    static constexpr unsigned int DEFAULT_BACKLOG = 128;

    static void read_callback(uv_stream_t *hndl, ssize_t nread, const uv_buf_t *buf) {
        T &ref = *(static_cast<T *>(hndl->data));
        // data will be destroyed no matter of what the value of nread is
        std::unique_ptr<char[]> data{buf->base};

        // nread == 0 is ignored (see http://docs.libuv.org/en/v1.x/stream.html)
        // equivalent to EAGAIN/EWOULDBLOCK, it shouldn't be treated as an error
        // for we don't have data to emit though, it's fine to suppress it

        if(nread == UV_EOF) {
            // end of stream
            ref.publish(end_event{});
        } else if(nread > 0) {
            // data available
            ref.publish(data_event{std::move(data), static_cast<std::size_t>(nread)});
        } else if(nread < 0) {
            // transmission error
            ref.publish(error_event(nread));
        }
    }

    static void listen_callback(uv_stream_t *hndl, int status) {
        if(T &ref = *(static_cast<T *>(hndl->data)); status) {
            ref.publish(error_event{status});
        } else {
            ref.publish(listen_event{});
        }
    }

    uv_stream_t *as_uv_stream() {
        return reinterpret_cast<uv_stream_t *>(this->raw());
    }

    const uv_stream_t *as_uv_stream() const {
        return reinterpret_cast<const uv_stream_t *>(this->raw());
    }

public:
#ifdef _MSC_VER
    stream_handle(loop::token token, std::shared_ptr<loop> ref)
        : handle<T, U, write_event, E...>{token, std::move(ref)} {}
#else
    using handle<T, U, write_event, E...>::handle;
#endif

    /**
     * @brief Shutdowns the outgoing (write) side of a duplex stream.
     *
     * It waits for pending write requests to complete. The handle should refer
     * to a initialized stream.<br/>
     * A shutdown event will be emitted after shutdown is complete.
     */
    void shutdown() {
        auto listener = [ptr = this->shared_from_this()](const auto &event, const auto &) {
            ptr->publish(event);
        };

        auto shutdown = this->parent().template resource<details::shutdown_req>();
        shutdown->template on<error_event>(listener);
        shutdown->template on<shutdown_event>(listener);
        shutdown->shutdown(as_uv_stream());
    }

    /**
     * @brief Starts listening for incoming connections.
     *
     * When a new incoming connection is received, a listen event is
     * emitted.<br/>
     * An error event will be emitted in case of errors.
     *
     * @param backlog Indicates the number of connections the kernel might
     * queue, same as listen(2).
     */
    void listen(int backlog = DEFAULT_BACKLOG) {
        if(auto err = uv_listen(as_uv_stream(), backlog, &listen_callback); err != 0) {
            this->publish(error_event{err});
        }
    }

    /**
     * @brief Accepts incoming connections.
     *
     * This call is used in conjunction with `listen()` to accept incoming
     * connections. Call this function after receiving a listen event to accept
     * the connection. Before calling this function, the submitted handle must
     * be initialized.<br>
     * An error event will be emitted in case of errors.
     *
     * When the listen event is emitted it is guaranteed that this function will
     * complete successfully the first time. If you attempt to use it more than
     * once, it may fail.<br/>
     * It is suggested to only call this function once per listen event.
     *
     * @note
     * Both the handles must be running on the same loop.
     *
     * @param ref An initialized handle to be used to accept the connection.
     */
    template<typename S>
    void accept(S &ref) {
        if(auto err = uv_accept(as_uv_stream(), ref.as_uv_stream()); err != 0) {
            this->publish(error_event{err});
        }
    }

    /**
     * @brief Starts reading data from an incoming stream.
     *
     * A data event will be emitted several times until there is no more data to
     * read or `stop()` is called.<br/>
     * An end event will be emitted when there is no more data to read.
     */
    void read() {
        if(auto err = uv_read_start(as_uv_stream(), &details::common_alloc_callback, &read_callback); err != 0) {
            this->publish(error_event{err});
        }
    }

    /**
     * @brief Stops reading data from the stream.
     *
     * This function is idempotent and may be safely called on a stopped stream.
     */
    void stop() {
        if(auto err = uv_read_stop(as_uv_stream()); err != 0) {
            this->publish(error_event{err});
        }
    }

    /**
     * @brief Writes data to the stream.
     *
     * Data are written in order. The handle takes the ownership of the data and
     * it is in charge of delete them.
     *
     * A write event will be emitted when the data have been written.<br/>
     * An error event will be emitted in case of errors.
     *
     * @param data The data to be written to the stream.
     * @param len The lenght of the submitted data.
     */
    template<typename Deleter>
    void write(std::unique_ptr<char[], Deleter> data, unsigned int len) {
        auto req = this->parent().template resource<details::write_req<Deleter>>(std::move(data), len);
        auto listener = [ptr = this->shared_from_this()](const auto &event, const auto &) {
            ptr->publish(event);
        };

        req->template on<error_event>(listener);
        req->template on<write_event>(listener);
        req->write(as_uv_stream());
    }

    /**
     * @brief Writes data to the stream.
     *
     * Data are written in order. The handle doesn't take the ownership of the
     * data. Be sure that their lifetime overcome the one of the request.
     *
     * A write event will be emitted when the data have been written.<br/>
     * An error event will be emitted in case of errors.
     *
     * @param data The data to be written to the stream.
     * @param len The lenght of the submitted data.
     */
    void write(char *data, unsigned int len) {
        auto req = this->parent().template resource<details::write_req<void (*)(char *)>>(std::unique_ptr<char[], void (*)(char *)>{data, [](char *) {}}, len);
        auto listener = [ptr = this->shared_from_this()](const auto &event, const auto &) {
            ptr->publish(event);
        };

        req->template on<error_event>(listener);
        req->template on<write_event>(listener);
        req->write(as_uv_stream());
    }

    /**
     * @brief Extended write function for sending handles over a pipe handle.
     *
     * The pipe must be initialized with `ipc == true`.
     *
     * `send` must be a tcp or pipe handle, which is a server or a connection
     * (listening or connected state). Bound sockets or pipes will be assumed to
     * be servers.
     *
     * The handle takes the ownership of the data and it is in charge of delete
     * them.
     *
     * A write event will be emitted when the data have been written.<br/>
     * An error event will be emitted in case of errors.
     *
     * @param send The handle over which to write data.
     * @param data The data to be written to the stream.
     * @param len The lenght of the submitted data.
     */
    template<typename S, typename Deleter>
    void write(S &send, std::unique_ptr<char[], Deleter> data, unsigned int len) {
        auto req = this->parent().template resource<details::write_req<Deleter>>(std::move(data), len);
        auto listener = [ptr = this->shared_from_this()](const auto &event, const auto &) {
            ptr->publish(event);
        };

        req->template on<error_event>(listener);
        req->template on<write_event>(listener);
        req->write(as_uv_stream(), send.as_uv_stream());
    }

    /**
     * @brief Extended write function for sending handles over a pipe handle.
     *
     * The pipe must be initialized with `ipc == true`.
     *
     * `send` must be a tcp or pipe handle, which is a server or a connection
     * (listening or connected state). Bound sockets or pipes will be assumed to
     * be servers.
     *
     * The handle doesn't take the ownership of the data. Be sure that their
     * lifetime overcome the one of the request.
     *
     * A write event will be emitted when the data have been written.<br/>
     * An error event will be emitted in case of errors.
     *
     * @param send The handle over which to write data.
     * @param data The data to be written to the stream.
     * @param len The lenght of the submitted data.
     */
    template<typename S>
    void write(S &send, char *data, unsigned int len) {
        auto req = this->parent().template resource<details::write_req<void (*)(char *)>>(std::unique_ptr<char[], void (*)(char *)>{data, [](char *) {}}, len);
        auto listener = [ptr = this->shared_from_this()](const auto &event, const auto &) {
            ptr->publish(event);
        };

        req->template on<error_event>(listener);
        req->template on<write_event>(listener);
        req->write(as_uv_stream(), send.as_uv_stream());
    }

    /**
     * @brief Queues a write request if it can be completed immediately.
     *
     * Same as `write()`, but won’t queue a write request if it can’t be
     * completed immediately.<br/>
     * An error event will be emitted in case of errors.
     *
     * @param data The data to be written to the stream.
     * @param len The lenght of the submitted data.
     * @return Number of bytes written.
     */
    int try_write(std::unique_ptr<char[]> data, unsigned int len) {
        uv_buf_t bufs[] = {uv_buf_init(data.get(), len)};
        auto bw = uv_try_write(as_uv_stream(), bufs, 1);

        if(bw < 0) {
            this->publish(error_event{bw});
            bw = 0;
        }

        return bw;
    }

    /**
     * @brief Queues a write request if it can be completed immediately.
     *
     * Same as `try_write` for sending handles over a pipe.<br/>
     * An error event will be emitted in case of errors.
     *
     * @param data The data to be written to the stream.
     * @param len The lenght of the submitted data.
     * @param send A valid handle suitable for the purpose.
     * @return Number of bytes written.
     */
    template<typename V, typename W>
    int try_write(std::unique_ptr<char[]> data, unsigned int len, stream_handle<V, W> &send) {
        uv_buf_t bufs[] = {uv_buf_init(data.get(), len)};
        auto bw = uv_try_write2(as_uv_stream(), bufs, 1, send.raw());

        if(bw < 0) {
            this->publish(error_event{bw});
            bw = 0;
        }

        return bw;
    }

    /**
     * @brief Queues a write request if it can be completed immediately.
     *
     * Same as `write()`, but won’t queue a write request if it can’t be
     * completed immediately.<br/>
     * An error event will be emitted in case of errors.
     *
     * @param data The data to be written to the stream.
     * @param len The lenght of the submitted data.
     * @return Number of bytes written.
     */
    int try_write(char *data, unsigned int len) {
        uv_buf_t bufs[] = {uv_buf_init(data, len)};
        auto bw = uv_try_write(as_uv_stream(), bufs, 1);

        if(bw < 0) {
            this->publish(error_event{bw});
            bw = 0;
        }

        return bw;
    }

    /**
     * @brief Queues a write request if it can be completed immediately.
     *
     * Same as `try_write` for sending handles over a pipe.<br/>
     * An error event will be emitted in case of errors.
     *
     * @param data The data to be written to the stream.
     * @param len The lenght of the submitted data.
     * @param send A valid handle suitable for the purpose.
     * @return Number of bytes written.
     */
    template<typename V, typename W>
    int try_write(char *data, unsigned int len, stream_handle<V, W> &send) {
        uv_buf_t bufs[] = {uv_buf_init(data, len)};
        auto bw = uv_try_write2(as_uv_stream(), bufs, 1, send.raw());

        if(bw < 0) {
            this->publish(error_event{bw});
            bw = 0;
        }

        return bw;
    }

    /**
     * @brief Checks if the stream is readable.
     * @return True if the stream is readable, false otherwise.
     */
    bool readable() const UVW_NOEXCEPT {
        return (uv_is_readable(as_uv_stream()) == 1);
    }

    /**
     * @brief Checks if the stream is writable.
     * @return True if the stream is writable, false otherwise.
     */
    bool writable() const UVW_NOEXCEPT {
        return (uv_is_writable(as_uv_stream()) == 1);
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
     * @return True in case of success, false otherwise.
     */
    bool blocking(bool enable = false) {
        return (0 == uv_stream_set_blocking(as_uv_stream(), enable));
    }

    /**
     * @brief Gets the amount of queued bytes waiting to be sent.
     * @return Amount of queued bytes waiting to be sent.
     */
    size_t write_queue_size() const UVW_NOEXCEPT {
        return uv_stream_get_write_queue_size(as_uv_stream());
    }
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "stream.cpp"
#endif

#endif // UVW_STREAM_INCLUDE_H
