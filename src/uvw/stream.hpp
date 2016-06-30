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
        ShutdownEvent event;
        event.error = UVWError{status};
        ref.publish(std::move(event));
    }

    static void listenCallback(T &ref, uv_stream_t *, int status) {
        ListenEvent event;
        event.error = UVWError{status};
        ref.publish(std::move(event));
    }

protected:
    template<typename U>
    Stream(ResourceType<U> rt, std::shared_ptr<Loop> ref)
        : Handle<T>{std::move(rt), std::move(ref)}, sdown{std::make_unique<uv_shutdown_t>()}
    { }

public:
    UVWError shutdown() noexcept {
        using CBF = typename Handle<T>::template CallbackFactory<void(uv_shutdown_t *, int)>;
        auto func = CBF::template create<&Stream<T>::shutdownCallback>(*static_cast<T*>(this));
        return UVWError{uv_shutdown(sdown.get(), this->template get<uv_stream_t>(), func)};
    }

    UVWError listen(int backlog) noexcept {
        using CBF = typename Handle<T>::template CallbackFactory<void(uv_stream_t *, int)>;
        auto func = CBF::template create<&Stream<T>::listenCallback>(*static_cast<T*>(this));
        return UVWError{uv_listen(this->template get<uv_stream_t>(), backlog, func)};
    }

    UVWError listen() noexcept {
        listen(DEFAULT_BACKLOG);
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
