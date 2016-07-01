#pragma once


#include <iterator>
#include <cstddef>
#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"


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
    Stream(ResourceType<U> rt, std::shared_ptr<Loop> ref)
        : Handle<T>{std::move(rt), std::move(ref)}, sdown{std::make_unique<uv_shutdown_t>()}
    { }

public:
    void shutdown() noexcept {
        using CBF = typename Handle<T>::template CallbackFactory<void(uv_shutdown_t *, int)>;
        auto func = CBF::template create<&Stream<T>::shutdownCallback>(*static_cast<T*>(this));
        auto err = uv_shutdown(sdown.get(), this->template get<uv_stream_t>(), func);
        if(err) this->publish(ErrorEvent{err});
    }

    void listen(int backlog) noexcept {
        using CBF = typename Handle<T>::template CallbackFactory<void(uv_stream_t *, int)>;
        auto func = CBF::template create<&Stream<T>::listenCallback>(*static_cast<T*>(this));
        auto err = uv_listen(this->template get<uv_stream_t>(), backlog, func);
        if(err) this->publish(ErrorEvent{err});
    }

    void listen() noexcept {
        listen(DEFAULT_BACKLOG);
    }

    // TODO read

    void stop() noexcept {
        auto err = uv_read_stop(this->template get<uv_stream_t>());
        if(err) this->publish(ErrorEvent{err});
    }

    // TODO write

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
