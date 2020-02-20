//
//  OSL
//

#ifndef QUALIFIEDTYPE_HPP
#define QUALIFIEDTYPE_HPP

#include <type_traits>

namespace ouzel
{
    class Type;

    enum class Qualifiers
    {
        None = 0x00,
        Const = 0x01,
        Volatile = 0x10
    };

    inline constexpr Qualifiers operator&(const Qualifiers a, const Qualifiers b) noexcept
    {
        return static_cast<Qualifiers>(static_cast<std::underlying_type_t<Qualifiers>>(a) & static_cast<std::underlying_type_t<Qualifiers>>(b));
    }
    inline constexpr Qualifiers operator|(const Qualifiers a, const Qualifiers b) noexcept
    {
        return static_cast<Qualifiers>(static_cast<std::underlying_type_t<Qualifiers>>(a) | static_cast<std::underlying_type_t<Qualifiers>>(b));
    }
    inline constexpr Qualifiers operator^(const Qualifiers a, const Qualifiers b) noexcept
    {
        return static_cast<Qualifiers>(static_cast<std::underlying_type_t<Qualifiers>>(a) ^ static_cast<std::underlying_type_t<Qualifiers>>(b));
    }
    inline constexpr Qualifiers operator~(const Qualifiers a) noexcept
    {
        return static_cast<Qualifiers>(~static_cast<std::underlying_type_t<Qualifiers>>(a));
    }
    inline constexpr Qualifiers& operator|=(Qualifiers& a, const Qualifiers b) noexcept
    {
        a = static_cast<Qualifiers>(static_cast<std::underlying_type_t<Qualifiers>>(a) | static_cast<std::underlying_type_t<Qualifiers>>(b));
        return a;
    }
    inline constexpr Qualifiers& operator&=(Qualifiers& a, const Qualifiers b) noexcept
    {
        a = static_cast<Qualifiers>(static_cast<std::underlying_type_t<Qualifiers>>(a) & static_cast<std::underlying_type_t<Qualifiers>>(b));
        return a;
    }
    inline constexpr Qualifiers& operator^=(Qualifiers& a, const Qualifiers b) noexcept
    {
        a = static_cast<Qualifiers>(static_cast<std::underlying_type_t<Qualifiers>>(a) ^ static_cast<std::underlying_type_t<Qualifiers>>(b));
        return a;
    }
    inline constexpr bool operator==(const Qualifiers a, const Qualifiers b) noexcept
    {
        return static_cast<std::underlying_type_t<Qualifiers>>(a) == static_cast<std::underlying_type_t<Qualifiers>>(b);
    }
    inline constexpr bool operator!=(const Qualifiers a, const Qualifiers b) noexcept
    {
        return static_cast<std::underlying_type_t<Qualifiers>>(a) != static_cast<std::underlying_type_t<Qualifiers>>(b);
    }
    inline constexpr bool operator>(const Qualifiers a, const Qualifiers b) noexcept
    {
        return static_cast<std::underlying_type_t<Qualifiers>>(a) > static_cast<std::underlying_type_t<Qualifiers>>(b);
    }
    inline constexpr bool operator<(const Qualifiers a, const Qualifiers b) noexcept
    {
        return static_cast<std::underlying_type_t<Qualifiers>>(a) < static_cast<std::underlying_type_t<Qualifiers>>(b);
    }
    inline constexpr bool operator>=(const Qualifiers a, const Qualifiers b) noexcept
    {
        return static_cast<std::underlying_type_t<Qualifiers>>(a) >= static_cast<std::underlying_type_t<Qualifiers>>(b);
    }
    inline constexpr bool operator<=(const Qualifiers a, const Qualifiers b) noexcept
    {
        return static_cast<std::underlying_type_t<Qualifiers>>(a) <= static_cast<std::underlying_type_t<Qualifiers>>(b);
    }
    inline constexpr bool operator!(const Qualifiers a) noexcept
    {
        return !static_cast<std::underlying_type_t<Qualifiers>>(a);
    }

    class QualifiedType final
    {
    public:
        bool operator<(const QualifiedType& other) const noexcept
        {
            if (type != other.type)
                return type < other.type;
            else if ((qualifiers & Qualifiers::Const) != (other.qualifiers & Qualifiers::Const))
                return (qualifiers & Qualifiers::Const) < (other.qualifiers & Qualifiers::Const);
            else if ((qualifiers & Qualifiers::Volatile) != (other.qualifiers & Qualifiers::Volatile))
                return (qualifiers & Qualifiers::Volatile) < (other.qualifiers & Qualifiers::Volatile);
            else return true;
        }

        const Type* type = nullptr;
        Qualifiers qualifiers = Qualifiers::None;
    };
}

#endif // QUALIFIEDTYPE_HPP
