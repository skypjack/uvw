#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "emitter.hpp"
#include "self.hpp"
#include "loop.hpp"


namespace uvw {


template<typename>
struct ResourceType;

template<> struct ResourceType<uv_async_t> { };
template<> struct ResourceType<uv_check_t> { };
template<> struct ResourceType<uv_idle_t> { };
template<> struct ResourceType<uv_prepare_t> { };
template<> struct ResourceType<uv_shutdown_t> { };
template<> struct ResourceType<uv_signal_t> { };
template<> struct ResourceType<uv_tcp_t> { };
template<> struct ResourceType<uv_timer_t> { };
template<> struct ResourceType<uv_tty_t> { };
template<> struct ResourceType<uv_work_t> { };


template<typename T>
class Resource: public Emitter<T>, public Self<T> {
    template<typename U>
    friend class Resource;

    struct BaseWrapper {
        virtual ~BaseWrapper() = default;
        virtual void * get() const noexcept = 0;
    };

    template<typename U>
    struct Wrapper: BaseWrapper {
        Wrapper(): resource{std::make_unique<U>()} { }
        void * get() const noexcept override { return resource.get(); }
    private:
        std::unique_ptr<U> resource;
    };

protected:
    template<typename U>
    explicit Resource(ResourceType<U>, std::shared_ptr<Loop> ref)
        : Emitter<T>{},
          Self<T>{},
          wrapper{std::make_unique<Wrapper<U>>()},
          pLoop{std::move(ref)}
    {
        this->template get<U>()->data = static_cast<T*>(this);
    }

    uv_loop_t* parent() const noexcept { return pLoop->loop.get(); }

    template<typename U>
    U* get() const noexcept { return reinterpret_cast<U*>(wrapper->get()); }

    template<typename F, typename... Args>
    auto invoke(F &&f, Args&&... args) {
        auto err = std::forward<F>(f)(std::forward<Args>(args)...);
        if(err) { Emitter<T>::publish(ErrorEvent{err}); }
        return err;
    }

public:
    virtual ~Resource() {
        static_assert(std::is_base_of<Resource<T>, T>::value, "!");
    }

    Loop& loop() const noexcept { return *pLoop; }

private:
    std::unique_ptr<BaseWrapper> wrapper;
    std::shared_ptr<Loop> pLoop;
};


}
