#ifndef UVW_DEBUG_INSTANTIATIONS_HPP
#define UVW_DEBUG_INSTANTIATIONS_HPP

namespace uvw {
    template std::size_t details::type_factory<TestEmitter>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<TestEmitter>::event_type<FakeEvent>() noexcept;
}


#endif //UVW_DEBUG_INSTANTIATIONS_HPP
