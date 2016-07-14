#pragma once


#include <cstddef>
#include <utility>
#include <memory>
#include <uv.h>
#include "resource.hpp"


namespace uvw {


template<typename T>
class Handle: public BaseHandle, public Resource<T> {
    static void closeCallback(uv_handle_t *handle) {
        Handle<T> &ref = *(static_cast<T*>(handle->data));
        auto ptr = ref.shared_from_this();
        ptr->reset();
        ref.publish(CloseEvent{});
    }

protected:
    template<typename U>
    explicit Handle(ResourceType<U> rt, std::shared_ptr<Loop> ref)
        : BaseHandle{},
          Resource<T>{std::move(rt), std::move(ref)}
    { }

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
