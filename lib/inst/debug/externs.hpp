#ifndef UVW_DEBUG_EXTERN_HPP
#define UVW_DEBUG_EXTERN_HPP

namespace uvw {
    extern template UVW_EXTERN std::size_t details::type_factory<TestEmitter>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TestEmitter>::event_type<FakeEvent>() noexcept;
}


#endif //UVW_DEBUG_EXTERN_HPP
