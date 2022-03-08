#ifndef UVW_TYPE_INFO_INCLUDE_HPP
#define UVW_TYPE_INFO_INCLUDE_HPP

#include <cstddef>
#include <string_view>

namespace uvw {

/**
 * @cond TURN_OFF_DOXYGEN
 * Internal details not to be documented.
 */

namespace internal {

// Fowler-Noll-Vo hash function v. 1a - the good
[[nodiscard]] static constexpr std::uint32_t fnv1a(const char *curr) noexcept {
    constexpr std::uint32_t offset = 2166136261;
    constexpr std::uint32_t prime = 16777619;
    auto value = offset;

    while(*curr != 0) {
        value = (value ^ static_cast<std::uint32_t>(*(curr++))) * prime;
    }

    return value;
}

[[nodiscard]] static inline std::uint32_t counter() noexcept {
    static std::uint32_t cnt{};
    return cnt++;
}

template<typename Type>
[[nodiscard]] static std::uint32_t fake() noexcept {
    static std::uint32_t local = counter();
    return local;
}

} // namespace internal

/**
 * Internal details not to be documented.
 * @endcond
 */

/**
 * @brief Returns a numerical identifier for a given type.
 * @tparam Type The type for which to return the numerical identifier.
 * @return The numerical identifier of the give type.
 */
template<typename Type>
[[nodiscard]] static constexpr std::uint32_t type() noexcept {
#if defined __clang__ || defined __GNUC__
    return internal::fnv1a(__PRETTY_FUNCTION__);
#elif defined _MSC_VER
    return internal::fnv1a(__FUNCSIG__);
#else
    return internal::fake();
#endif
}

} // namespace uvw

#endif // UVW_TYPE_INFO_INCLUDE_HPP
