#pragma once


#include <cstddef>
#include <utility>
#include <memory>
#include <uv.h>
#include "resource.hpp"


namespace uvw {


template<typename>
struct HandleType;

template<> struct HandleType<uv_async_t> { };
template<> struct HandleType<uv_check_t> { };
template<> struct HandleType<uv_fs_poll_t> { };
template<> struct HandleType<uv_idle_t> { };
template<> struct HandleType<uv_pipe_t> { };
template<> struct HandleType<uv_poll_t> { };
template<> struct HandleType<uv_prepare_t> { };
template<> struct HandleType<uv_signal_t> { };
template<> struct HandleType<uv_tcp_t> { };
template<> struct HandleType<uv_timer_t> { };
template<> struct HandleType<uv_tty_t> { };
template<> struct HandleType<uv_udp_t> { };


template<typename T>
class Handle: public BaseHandle, public Resource<T>
{
    static void closeCallback(uv_handle_t *handle) {
        Handle<T> &ref = *(static_cast<T*>(handle->data));
        auto ptr = ref.shared_from_this();
        (void)ptr;
        ref.reset();
        ref.publish(CloseEvent{});
    }

protected:
    using Resource<T>::Resource;

    static void allocCallback(uv_handle_t *, std::size_t suggested, uv_buf_t *buf) {
        *buf = uv_buf_init(new char[suggested], suggested);
    }

    template<typename U, typename F, typename... Args>
    bool initialize(F &&f, Args&&... args) {
        if(!this->self()) {
            auto err = std::forward<F>(f)(this->parent(), this->template get<U>(), std::forward<Args>(args)...);

            if(err) {
                this->publish(ErrorEvent{err});
            } else {
                this->leak();
            }
        }

        return this->self();
    }

public:
    bool active() const noexcept override {
        return !(uv_is_active(this->template get<uv_handle_t>()) == 0);
    }

    bool closing() const noexcept override {
        return !(uv_is_closing(this->template get<uv_handle_t>()) == 0);
    }

    void close() noexcept override {
        if(!closing()) {
            uv_close(this->template get<uv_handle_t>(), &Handle<T>::closeCallback);
        }
    }

    void reference() noexcept override {
        uv_ref(this->template get<uv_handle_t>());
    }

    void unreference() noexcept override {
        uv_unref(this->template get<uv_handle_t>());
    }

    bool referenced() const noexcept override {
        return !(uv_has_ref(this->template get<uv_handle_t>()) == 0);
    }

    std::size_t size() const noexcept {
        return uv_handle_size(this->template get<uv_handle_t>()->type);
    }
};


}
