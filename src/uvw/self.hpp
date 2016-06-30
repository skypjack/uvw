#pragma once


#include <memory>


namespace uvw {


template<typename T>
struct Self: public std::enable_shared_from_this<T> {
    virtual ~Self() {
        static_assert(std::is_base_of<Self<T>, T>::value, "!");
    }

protected:
    void leak() noexcept { self = this->shared_from_this(); }
    void reset() noexcept { self.reset(); }

private:
    std::shared_ptr<void> self;
};


}
