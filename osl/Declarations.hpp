//
//  OSL
//

#ifndef DECLARATIONS_HPP
#define DECLARATIONS_HPP

#include "Construct.hpp"
#include "QualifiedType.hpp"
#include "Attributes.hpp"

namespace ouzel
{
    class Expression;
    class Statement;

    enum class StorageClass
    {
        Auto,
        Extern,
        Static
    };

    class Declaration: public Construct
    {
    public:
        enum class Kind
        {
            Empty,
            Type,
            Field,
            Callable,
            Variable,
            Parameter
        };

        Declaration(Kind initDeclarationKind):
            Construct(Construct::Kind::Declaration),
            declarationKind(initDeclarationKind) {}

        inline Kind getDeclarationKind() const noexcept { return declarationKind; }

        std::string name;
        QualifiedType qualifiedType;
        Declaration* firstDeclaration = nullptr;
        Declaration* previousDeclaration = nullptr;
        Declaration* definition = nullptr;
        std::vector<Attribute*> attributes;

    private:
        const Kind declarationKind;
    };

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

        Type(Kind initTypeKind, size_t initSize):
            typeKind(initTypeKind),
            size(initSize) {}

        inline Kind getTypeKind() const noexcept { return typeKind; }

        std::string name;
        size_t size = 0;
        TypeDeclaration* declaration = nullptr;

    private:
        Kind typeKind;
    };

    class ArrayType final: public Type
    {
    public:
        ArrayType(QualifiedType initElementType, size_t initCount):
            Type(Type::Kind::Array, initElementType.type->size * initCount),
            elementType(initElementType),
            count(initCount) {}

        QualifiedType elementType;
        size_t count = 0;
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

        ScalarType(Kind initScalarTypeKind, size_t initSize, bool initIsUnsigned):
            Type(Type::Kind::Scalar, initSize),
            isUnsigned(initIsUnsigned),
            scalarTypeKind(initScalarTypeKind)
        {
        }

        inline Kind getScalarTypeKind() const noexcept { return scalarTypeKind; }

        bool isUnsigned = false;

    private:
        const Kind scalarTypeKind;
    };

    class FieldDeclaration final: public Declaration
    {
    public:
        FieldDeclaration() noexcept: Declaration(Declaration::Kind::Field)
        {
            definition = this;
        }
    };

    enum class InputModifier
    {
        In,
        Inout,
        Out
    };

    class ParameterDeclaration final: public Declaration
    {
    public:
        ParameterDeclaration() noexcept: Declaration(Declaration::Kind::Parameter)
        {
            definition = this;
        }

        InputModifier inputModifier = InputModifier::In;
    };

    class CallableDeclaration: public Declaration
    {
    public:
        enum class Kind
        {
            Function,
            Constructor,
            Method
        };

        CallableDeclaration(Kind initCallableDeclarationKind): Declaration(Declaration::Kind::Callable), callableDeclarationKind(initCallableDeclarationKind) {}

        inline Kind getCallableDeclarationKind() const { return callableDeclarationKind; }

        StorageClass storageClass = StorageClass::Auto;
        std::vector<ParameterDeclaration*> parameterDeclarations;
        const Statement* body = nullptr;

    private:
        const Kind callableDeclarationKind;
    };

    class FunctionDeclaration final: public CallableDeclaration
    {
    public:
        FunctionDeclaration(): CallableDeclaration(CallableDeclaration::Kind::Function) {}

        bool isBuiltin = false;
    };

    class ConstructorDeclaration final: public CallableDeclaration
    {
    public:
        ConstructorDeclaration(): CallableDeclaration(CallableDeclaration::Kind::Constructor) {}
    };

    class MethodDeclaration final: public CallableDeclaration
    {
    public:
        MethodDeclaration(): CallableDeclaration(CallableDeclaration::Kind::Method) {}

        bool isBuiltin = false;
    };

    class StructType final: public Type
    {
    public:
        StructType(size_t initSize): Type(Type::Kind::Struct, initSize) {}

        ConstructorDeclaration* findConstructorDeclaration(const std::vector<QualifiedType>& parameters) const noexcept
        {
            for (Declaration* declaration : memberDeclarations)
            {
                if (declaration->getDeclarationKind() == Declaration::Kind::Callable)
                {
                    CallableDeclaration* callableDeclaration = static_cast<CallableDeclaration*>(declaration);

                    if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::Constructor)
                    {
                        ConstructorDeclaration* constructorDeclaration = static_cast<ConstructorDeclaration*>(callableDeclaration);

                        if (constructorDeclaration->parameterDeclarations.size() == parameters.size() &&
                            std::equal(parameters.begin(), parameters.end(),
                                       constructorDeclaration->parameterDeclarations.begin(),
                                       [](const QualifiedType& qualifiedType,
                                          const ParameterDeclaration* parameterDeclaration) {
                                           return qualifiedType.type == parameterDeclaration->qualifiedType.type; // TODO: overload resolution
                                       }))
                            return constructorDeclaration;
                    }
                }
            }

            return nullptr;
        }

        Declaration* findMemberDeclaration(const std::string& name) const noexcept
        {
            for (Declaration* memberDeclaration : memberDeclarations)
                if (memberDeclaration->name == name) return memberDeclaration;

            return nullptr;
        }

        std::vector<Declaration*> memberDeclarations;
    };

    class VectorType final: public Type
    {
    public:
        VectorType(const ScalarType* initComponentType,
                   size_t initComponentCount):
            Type(Type::Kind::Vector, initComponentType->size * initComponentCount),
            componentType(initComponentType),
            componentCount(initComponentCount) {}

        const ScalarType* componentType = nullptr;
        size_t componentCount = 1;
    };

    class MatrixType final: public Type
    {
    public:
        MatrixType(const ScalarType* initComponentType,
                   size_t initRowCount,
                   size_t initColumnCount):
            Type(Type::Kind::Matrix, initComponentType->size * initRowCount * initColumnCount),
            componentType(initComponentType),
            rowCount(initRowCount),
            columnCount(initColumnCount) {}

        const ScalarType* componentType = nullptr;
        uint8_t rowCount = 1;
        uint8_t columnCount = 1;
    };

    class VariableDeclaration final: public Declaration
    {
    public:
        VariableDeclaration() noexcept: Declaration(Declaration::Kind::Variable)
        {
            definition = this;
        }

        StorageClass storageClass = StorageClass::Auto;
        const Expression* initialization = nullptr;
    };

    class TypeDeclaration final: public Declaration
    {
    public:
        TypeDeclaration() noexcept: Declaration(Declaration::Kind::Type) {}

        Type* type = nullptr;
    };
}

#endif // DECLARATIONS_HPP
