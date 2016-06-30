#pragma once


#include <memory>


namespace uvw {


template<typename>
struct ResourceType { };


class ResourceWrapper {
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

protected:
    template<typename U>
    explicit ResourceWrapper(ResourceType<U>)
        : wrapper{std::make_unique<Wrapper<U>>()}
    { }

    virtual ~ResourceWrapper() = default;

    template<typename U>
    U* get() const noexcept { return reinterpret_cast<U*>(wrapper->get()); }

private:
    std::unique_ptr<BaseWrapper> wrapper;
};


}
