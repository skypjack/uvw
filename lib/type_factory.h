#ifndef UVW_TYPE_FACTORY_H
#define UVW_TYPE_FACTORY_H

#include <uvw/config.h>
#include <cstddef>

namespace uvw::details {
    template<typename T>
    struct UVW_EXTERN type_factory {
        template<typename>
        static std::size_t event_type() noexcept;

        static std::size_t next_type() noexcept;
    };
}


#endif //UVW_TYPE_FACTORY_H
