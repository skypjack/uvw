#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "emitter.hpp"
#include "self.hpp"
#include "loop.hpp"


namespace uvw {


template<typename>
struct HandleType;

template<> struct HandleType<uv_timer_t> { };
template<> struct HandleType<uv_prepare_t> { };
template<> struct HandleType<uv_check_t> { };
template<> struct HandleType<uv_idle_t> { };
template<> struct HandleType<uv_tcp_t> { };


template<typename T>
class Handle: public Emitter<T>, public Self<T> {
    struct BaseWrapper {
        virtual ~BaseWrapper() = default;
        virtual void * get() const noexcept = 0;
    };

    template<typename U>
    struct Wrapper: BaseWrapper {
        Wrapper(): handle{std::make_unique<U>()} { }
        void * get() const noexcept override { return handle.get(); }
    private:
        std::unique_ptr<U> handle;
    };

    static void closeCallback(uv_handle_t *handle) {
        T &ref = *(static_cast<T*>(handle->data));
        ref.publish(CloseEvent{});
        ref.reset();
    }

protected:
    template<typename U>
    explicit Handle(HandleType<U>, std::shared_ptr<Loop> ref)
        : Emitter<T>{}, Self<T>{},
          wrapper{std::make_unique<Wrapper<U>>()},
          pLoop{std::move(ref)}
    {
        this->template get<uv_handle_t>()->data = static_cast<T*>(this);
    }

    uv_loop_t* parent() const noexcept { return pLoop->loop.get(); }

    template<typename U>
    U* get() const noexcept { return reinterpret_cast<U*>(wrapper->get()); }

    template<typename U, typename F>
    bool initialize(F &&f) {
        bool ret = true;

        if(!active()) {
            auto err = std::forward<F>(f)(parent(), get<U>());

            if(err) {
                this->publish(ErrorEvent{err});
                ret = false;
            } else {
                this->leak();
            }
        }

        return ret;
    }

    template<typename F, typename... Args>
    void invoke(F &&f, Args&&... args) {
        auto err = std::forward<F>(f)(std::forward<Args>(args)...);
        if(err) { Emitter<T>::publish(ErrorEvent{err}); }
    }

public:
    virtual ~Handle() {
        static_assert(std::is_base_of<Handle<T>, T>::value, "!");
    }

    Loop& loop() const noexcept { return *pLoop; }

    bool active() const noexcept { return !(uv_is_active(get<uv_handle_t>()) == 0); }
    bool closing() const noexcept { return !(uv_is_closing(get<uv_handle_t>()) == 0); }

    void reference() noexcept { uv_ref(get<uv_handle_t>()); }
    void unreference() noexcept { uv_unref(get<uv_handle_t>()); }
    bool referenced() const noexcept { return !(uv_has_ref(get<uv_handle_t>()) == 0); }

    void close() noexcept {
        if(!closing()) {
            uv_close(get<uv_handle_t>(), &Handle<T>::closeCallback);
        }
    }

private:
    std::unique_ptr<BaseWrapper> wrapper;
    std::shared_ptr<Loop> pLoop;
};


}
