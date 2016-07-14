#pragma once


#include <memory>


namespace uvw {


template<typename T>
struct Self: std::enable_shared_from_this<T> {
    void leak() noexcept { ptr = this->shared_from_this(); }
    void reset() noexcept { ptr.reset(); }
    bool self() const noexcept { return static_cast<bool>(ptr); }

private:
    std::shared_ptr<void> ptr{nullptr};
};


}
