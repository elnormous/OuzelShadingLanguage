//
//  OSL
//

#ifndef TYPES_HPP
#define TYPES_HPP

#include <type_traits>

namespace ouzel
{
    class TypeDeclaration;

    class Type
    {
    public:
        enum class Kind
        {
            Void,
            Array,
            Scalar,
            Struct,
            Vector,
            Matrix
        };

        enum class Qualifiers: std::uint8_t
        {
            None = 0x00,
            Const = 0x01,
            Volatile = 0x10
        };

        Type(const Type&) = delete;

        explicit Type(Kind initTypeKind):
            typeKind(initTypeKind) {}

        Type(Kind initTypeKind, const std::string& initName):
            typeKind(initTypeKind),
            name(initName) {}

        Type& operator=(const Type&) = delete;

        const Kind typeKind;
        const std::string name;
    };

    inline constexpr Type::Qualifiers operator&(const Type::Qualifiers a, const Type::Qualifiers b) noexcept
    {
        return static_cast<Type::Qualifiers>(static_cast<std::underlying_type<Type::Qualifiers>::type>(a) & static_cast<std::underlying_type<Type::Qualifiers>::type>(b));
    }
    inline constexpr Type::Qualifiers operator|(const Type::Qualifiers a, const Type::Qualifiers b) noexcept
    {
        return static_cast<Type::Qualifiers>(static_cast<std::underlying_type<Type::Qualifiers>::type>(a) | static_cast<std::underlying_type<Type::Qualifiers>::type>(b));
    }
    inline constexpr Type::Qualifiers operator^(const Type::Qualifiers a, const Type::Qualifiers b) noexcept
    {
        return static_cast<Type::Qualifiers>(static_cast<std::underlying_type<Type::Qualifiers>::type>(a) ^ static_cast<std::underlying_type<Type::Qualifiers>::type>(b));
    }
    inline constexpr Type::Qualifiers operator~(const Type::Qualifiers a) noexcept
    {
        return static_cast<Type::Qualifiers>(~static_cast<std::underlying_type<Type::Qualifiers>::type>(a));
    }
    inline constexpr Type::Qualifiers& operator|=(Type::Qualifiers& a, const Type::Qualifiers b) noexcept
    {
        return a = static_cast<Type::Qualifiers>(static_cast<std::underlying_type<Type::Qualifiers>::type>(a) | static_cast<std::underlying_type<Type::Qualifiers>::type>(b));
    }
    inline constexpr Type::Qualifiers& operator&=(Type::Qualifiers& a, const Type::Qualifiers b) noexcept
    {
        return a = static_cast<Type::Qualifiers>(static_cast<std::underlying_type<Type::Qualifiers>::type>(a) & static_cast<std::underlying_type<Type::Qualifiers>::type>(b));
    }
    inline constexpr Type::Qualifiers& operator^=(Type::Qualifiers& a, const Type::Qualifiers b) noexcept
    {
        return a = static_cast<Type::Qualifiers>(static_cast<std::underlying_type<Type::Qualifiers>::type>(a) ^ static_cast<std::underlying_type<Type::Qualifiers>::type>(b));
    }
    inline constexpr bool operator!(const Type::Qualifiers a) noexcept
    {
        return !static_cast<std::underlying_type<Type::Qualifiers>::type>(a);
    }

    class QualifiedType final
    {
    public:
        QualifiedType(const Type& initType, Type::Qualifiers initQualifiers = Type::Qualifiers::None) noexcept:
            type(initType), qualifiers(initQualifiers) {}

        bool operator<(const QualifiedType& other) const noexcept
        {
            if (&type != &other.type)
                return &type < &other.type;
            else if ((qualifiers & Type::Qualifiers::Const) != (other.qualifiers & Type::Qualifiers::Const))
                return (qualifiers & Type::Qualifiers::Const) < (other.qualifiers & Type::Qualifiers::Const);
            else if ((qualifiers & Type::Qualifiers::Volatile) != (other.qualifiers & Type::Qualifiers::Volatile))
                return (qualifiers & Type::Qualifiers::Volatile) < (other.qualifiers & Type::Qualifiers::Volatile);
            else return true;
        }

        const Type& type;
        const Type::Qualifiers qualifiers = Type::Qualifiers::None;
    };

    class ArrayType final: public Type
    {
    public:
        ArrayType(const QualifiedType& initElementType,
                  std::size_t initSize):
            Type(Type::Kind::Array),
            elementType(initElementType),
            size(initSize) {}

        const QualifiedType elementType;
        const std::size_t size = 0;
    };

    class ScalarType final: public Type
    {
    public:
        enum class Kind
        {
            Boolean,
            Integer,
            FloatingPoint
        };

        ScalarType(const std::string& initName,
                   Kind initScalarTypeKind,
                   bool initIsUnsigned):
            Type(Type::Kind::Scalar, initName),
            scalarTypeKind(initScalarTypeKind),
            isUnsigned(initIsUnsigned)
        {
        }

        const Kind scalarTypeKind;
        const bool isUnsigned = false;
    };

    class Declaration;

    class StructType final: public Type
    {
    public:
        StructType(const std::string& initName,
                   std::vector<const Declaration*> initMemberDeclarations):
            Type(Type::Kind::Struct, initName),
            memberDeclarations(std::move(initMemberDeclarations)) {}

        const std::vector<const Declaration*> memberDeclarations;
    };

    class VectorType final: public Type
    {
    public:
        VectorType(const std::string& initName,
                   const ScalarType& initComponentType,
                   std::size_t initComponentCount):
            Type(Type::Kind::Vector, initName),
            componentType(initComponentType),
            componentCount(initComponentCount) {}

        const ScalarType& componentType;
        const std::size_t componentCount = 1;
    };

    class MatrixType final: public Type
    {
    public:
        MatrixType(const std::string& initName,
                   const VectorType& initRowType,
                   std::size_t initRowCount):
            Type(Type::Kind::Matrix, initName),
            rowType(initRowType),
            rowCount(initRowCount) {}

        const VectorType& rowType;
        const std::size_t rowCount = 1;
    };
}

#endif // DECLARATIONS_HPP
