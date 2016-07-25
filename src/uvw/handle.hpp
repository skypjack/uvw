#pragma once


#include <cstddef>
#include <utility>
#include <memory>
#include <uv.h>
#include "resource.hpp"


namespace uvw {


template<typename T, typename U>
class Handle: public BaseHandle, public Resource<T, U>
{
    static void closeCallback(uv_handle_t *handle) {
        Handle<T, U> &ref = *(static_cast<T*>(handle->data));
        auto ptr = ref.shared_from_this();
        (void)ptr;
        ref.reset();
        ref.publish(CloseEvent{});
    }

    template<typename F>
    int setBufferSize(F &&f) {
        int value = 0;

        if(0 != invoke(std::forward<F>(f), this->template get<uv_handle_t>(), &value)) {
            value = 0;
        }

        return 0;
    }

    template<typename F>
    void getBufferSize(F &&f, int value) {
        invoke(&std::forward<F>(f), this->template get<uv_handle_t>(), &value);
    }

protected:
    using Resource<T, U>::Resource;

    static void allocCallback(uv_handle_t *, std::size_t suggested, uv_buf_t *buf) {
        *buf = uv_buf_init(new char[suggested], suggested);
    }

    template<typename H, typename F, typename... Args>
    bool initialize(F &&f, Args&&... args) {
        if(!this->self()) {
            auto err = std::forward<F>(f)(this->parent(), this->template get<H>(), std::forward<Args>(args)...);

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
            uv_close(this->template get<uv_handle_t>(), &Handle<T, U>::closeCallback);
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

    int sendBufferSize() const {
        return setBufferSize(&uv_send_buffer_size);
    }

    void sendBufferSize(int value) {
        getBufferSize(&uv_send_buffer_size, value);
    }

    int recvBufferSize() const {
        return setBufferSize(&uv_recv_buffer_size);
    }

    void recvBufferSize(int value) {
        getBufferSize(&uv_recv_buffer_size, value);
    }

    OSFileDescriptor fileno() const {
        uv_os_fd_t fd;
        invoke(&uv_fileno, this->template get<uv_handle_t>(), &fd);
        return fd;
    }
};


}
