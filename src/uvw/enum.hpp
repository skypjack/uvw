#ifndef UVW_ENUM_INCLUDE_HPP
#define UVW_ENUM_INCLUDE_HPP

#include <type_traits>
#include "config.h"

/**
 * @brief Operator available for enums for which bitmask support is enabled.
 * @tparam Type Enum class type.
 * @param lhs The first value to use.
 * @param rhs The second value to use.
 * @return The result of invoking the operator on the underlying types of the
 * two values provided.
 */
template<typename Type>
[[nodiscard]] constexpr std::enable_if_t<std::is_enum_v<Type>, decltype(Type::_UVW_ENUM)>
operator|(const Type lhs, const Type rhs) noexcept {
    return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(lhs) | static_cast<std::underlying_type_t<Type>>(rhs));
}

/*! @copydoc operator| */
template<typename Type>
[[nodiscard]] constexpr std::enable_if_t<std::is_enum_v<Type>, decltype(Type::_UVW_ENUM)>
operator&(const Type lhs, const Type rhs) noexcept {
    return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(lhs) & static_cast<std::underlying_type_t<Type>>(rhs));
}

/*! @copydoc operator| */
template<typename Type>
[[nodiscard]] constexpr std::enable_if_t<std::is_enum_v<Type>, decltype(Type::_UVW_ENUM)>
operator^(const Type lhs, const Type rhs) noexcept {
    return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(lhs) ^ static_cast<std::underlying_type_t<Type>>(rhs));
}

/**
 * @brief Operator available for enums for which bitmask support is enabled.
 * @tparam Type Enum class type.
 * @param value The value to use.
 * @return The result of invoking the operator on the underlying types of the
 * value provided.
 */
template<typename Type>
[[nodiscard]] constexpr std::enable_if_t<std::is_enum_v<Type>, decltype(Type::_UVW_ENUM)>
operator~(const Type value) noexcept {
    return static_cast<Type>(~static_cast<std::underlying_type_t<Type>>(value));
}

/*! @copydoc operator~ */
template<typename Type>
[[nodiscard]] constexpr std::enable_if_t<std::is_enum_v<Type>, decltype(Type::_UVW_ENUM, bool{})>
operator!(const Type value) noexcept {
    return !static_cast<std::underlying_type_t<Type>>(value);
}

/*! @copydoc operator| */
template<typename Type>
constexpr std::enable_if_t<std::is_enum_v<Type>, decltype(Type::_UVW_ENUM) &>
operator|=(Type &lhs, const Type rhs) noexcept {
    return (lhs = (lhs | rhs));
}

/*! @copydoc operator| */
template<typename Type>
constexpr std::enable_if_t<std::is_enum_v<Type>, decltype(Type::_UVW_ENUM) &>
operator&=(Type &lhs, const Type rhs) noexcept {
    return (lhs = (lhs & rhs));
}

/*! @copydoc operator| */
template<typename Type>
constexpr std::enable_if_t<std::is_enum_v<Type>, decltype(Type::_UVW_ENUM) &>
operator^=(Type &lhs, const Type rhs) noexcept {
    return (lhs = (lhs ^ rhs));
}

#endif
