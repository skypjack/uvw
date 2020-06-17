#include "type_factory.h"

#ifdef UVW_AS_LIB
#include <uvw.hpp>
#endif

namespace uvw::details {
    template<typename T>
    std::size_t type_factory<T>::next_type() noexcept {
        static std::size_t counter = 0;
        return counter++;
    }

    template<typename T>
    template<typename>
    std::size_t type_factory<T>::event_type() noexcept {
        static std::size_t value = next_type();
        return value;
    }
}

#ifdef UVW_AS_LIB
#include "inst/instantiations.hpp"
#endif