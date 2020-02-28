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

        Declaration(Kind initDeclarationKind,
                    const QualifiedType& initQualifiedType):
            Construct(Construct::Kind::Declaration),
            qualifiedType(initQualifiedType),
            declarationKind(initDeclarationKind) {}

        Declaration(Kind initDeclarationKind,
                    const std::string& initName,
                    const QualifiedType& initQualifiedType):
            Construct(Construct::Kind::Declaration),
            name(initName),
            qualifiedType(initQualifiedType),
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

        Type(Kind initTypeKind):
            typeKind(initTypeKind) {}

        Type(Kind initTypeKind, const std::string& initName):
            name(initName),
            typeKind(initTypeKind) {}

        inline Kind getTypeKind() const noexcept { return typeKind; }

        std::string name;
        TypeDeclaration* declaration = nullptr;

    private:
        Kind typeKind;
    };

    class ArrayType final: public Type
    {
    public:
        ArrayType(const QualifiedType& initElementType,
                  size_t initSize):
            Type(Type::Kind::Array),
            elementType(initElementType),
            size(initSize) {}

        QualifiedType elementType;
        size_t size = 0;
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
        FieldDeclaration(const std::string& initName,
                         const QualifiedType& initQualifiedType) noexcept:
            Declaration(Declaration::Kind::Field, initName, initQualifiedType)
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
        ParameterDeclaration(const QualifiedType& initQualifiedType,
                             InputModifier initInputModifier) noexcept:
            Declaration(Declaration::Kind::Parameter, initQualifiedType),
            inputModifier(initInputModifier)
        {
            definition = this;
        }

        ParameterDeclaration(const std::string& initName,
                             const QualifiedType& initQualifiedType,
                             InputModifier initInputModifier) noexcept:
            Declaration(Declaration::Kind::Parameter, initName, initQualifiedType),
            inputModifier(initInputModifier)
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

        CallableDeclaration(Kind initCallableDeclarationKind,
                            const QualifiedType& initQualifiedType,
                            StorageClass initStorageClass):
            Declaration(Declaration::Kind::Callable, initQualifiedType),
            callableDeclarationKind(initCallableDeclarationKind),
            storageClass(initStorageClass) {}

        CallableDeclaration(Kind initCallableDeclarationKind,
                            const std::string& initName,
                            const QualifiedType& initQualifiedType,
                            StorageClass initStorageClass):
            Declaration(Declaration::Kind::Callable, initName, initQualifiedType),
            callableDeclarationKind(initCallableDeclarationKind),
            storageClass(initStorageClass) {}

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
        FunctionDeclaration(const std::string& initName,
                            const QualifiedType& initQualifiedType,
                            StorageClass initStorageClass,
                            bool initIsBuiltin = false):
            CallableDeclaration(CallableDeclaration::Kind::Function, initName,
                                initQualifiedType,
                                initStorageClass),
            isBuiltin(initIsBuiltin) {}

        bool isBuiltin = false;
    };

    class ConstructorDeclaration final: public CallableDeclaration
    {
    public:
        ConstructorDeclaration(StorageClass initStorageClass):
            CallableDeclaration(CallableDeclaration::Kind::Constructor,
                                QualifiedType{nullptr},
                                initStorageClass) {}
    };

    class MethodDeclaration final: public CallableDeclaration
    {
    public:
        MethodDeclaration(const std::string& initName,
                          const QualifiedType& initQualifiedType,
                          StorageClass initStorageClass,
                          bool initIsBuiltin = false):
            CallableDeclaration(CallableDeclaration::Kind::Method, initName,
                                initQualifiedType,
                                initStorageClass),
            isBuiltin(initIsBuiltin) {}

        bool isBuiltin = false;
    };

    class StructType final: public Type
    {
    public:
        StructType(const std::string& initName):
            Type(Type::Kind::Struct, initName) {}

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
        VectorType(const std::string& initName,
                   const ScalarType* initComponentType,
                   size_t initComponentCount):
            Type(Type::Kind::Vector, initName),
            componentType(initComponentType),
            componentCount(initComponentCount) {}

        const ScalarType* componentType = nullptr;
        size_t componentCount = 1;
    };

    class MatrixType final: public Type
    {
    public:
        MatrixType(const std::string& initName,
                   const ScalarType* initComponentType,
                   size_t initRowCount,
                   size_t initColumnCount):
            Type(Type::Kind::Matrix, initName),
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
        VariableDeclaration(const std::string& initName,
                            const QualifiedType& initQualifiedType,
                            StorageClass initStorageClass,
                            const Expression* initInitialization = nullptr) noexcept:
            Declaration(Declaration::Kind::Variable, initName, initQualifiedType),
            storageClass(initStorageClass),
            initialization(initInitialization)
        {
            definition = this;
        }

        StorageClass storageClass = StorageClass::Auto;
        const Expression* initialization = nullptr;
    };

    class TypeDeclaration final: public Declaration
    {
    public:
        TypeDeclaration(const std::string& initName, Type* initType) noexcept:
            Declaration(Declaration::Kind::Type, initName, QualifiedType{initType}),
            type(initType) {}

        Type* type = nullptr;
    };
}

#endif // DECLARATIONS_HPP
