//
//  OSL
//

#ifndef DECLARATIONS_HPP
#define DECLARATIONS_HPP

#include "Construct.hpp"
#include "Attributes.hpp"
#include "Types.hpp"

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
            Type,
            Field,
            Callable,
            Variable,
            Parameter
        };

        Declaration(const Declaration&) = delete;

        Declaration(Kind initDeclarationKind,
                    std::vector<const Attribute*> initAttributes):
            Construct(Construct::Kind::Declaration),
            declarationKind(initDeclarationKind),
            attributes(std::move(initAttributes)) {}

        Declaration(Kind initDeclarationKind,
                    const std::string& initName,
                    std::vector<const Attribute*> initAttributes):
            Construct(Construct::Kind::Declaration),
            declarationKind(initDeclarationKind),
            name(initName),
            attributes(std::move(initAttributes)) {}

        Declaration& operator=(const Declaration&) = delete;

        const Kind declarationKind;
        const std::string name;
        Declaration* firstDeclaration = nullptr;
        Declaration* previousDeclaration = nullptr;
        Declaration* definition = nullptr;
        const std::vector<const Attribute*> attributes;
    };

    class FieldDeclaration final: public Declaration
    {
    public:
        FieldDeclaration(const std::string& initName,
                         const QualifiedType& initQualifiedType,
                         std::vector<const Attribute*> initAttributes) noexcept:
            Declaration(Declaration::Kind::Field, initName, std::move(initAttributes)),
            qualifiedType(initQualifiedType)
        {
            definition = this;
        }

        const QualifiedType qualifiedType;
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
                             InputModifier initInputModifier,
                             std::vector<const Attribute*> initAttributes) noexcept:
            Declaration(Declaration::Kind::Parameter, std::move(initAttributes)),
            inputModifier(initInputModifier),
            qualifiedType(initQualifiedType)
        {
            definition = this;
        }

        ParameterDeclaration(const std::string& initName,
                             const QualifiedType& initQualifiedType,
                             InputModifier initInputModifier,
                             std::vector<const Attribute*> initAttributes) noexcept:
            Declaration(Declaration::Kind::Parameter, initName, std::move(initAttributes)),
            inputModifier(initInputModifier),
            qualifiedType(initQualifiedType)
        {
            definition = this;
        }

        const InputModifier inputModifier = InputModifier::In;
        const QualifiedType qualifiedType;
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
                            StorageClass initStorageClass,
                            std::vector<const Attribute*> initAttributes,
                            std::vector<ParameterDeclaration*> initParameterDeclarations):
            Declaration(Declaration::Kind::Callable, std::move(initAttributes)),
            callableDeclarationKind(initCallableDeclarationKind),
            storageClass(initStorageClass),
            parameterDeclarations(std::move(initParameterDeclarations)) {}

        CallableDeclaration(Kind initCallableDeclarationKind,
                            const std::string& initName,
                            StorageClass initStorageClass,
                            std::vector<const Attribute*> initAttributes,
                            std::vector<ParameterDeclaration*> initParameterDeclarations):
            Declaration(Declaration::Kind::Callable, initName, std::move(initAttributes)),
            callableDeclarationKind(initCallableDeclarationKind),
            storageClass(initStorageClass),
            parameterDeclarations(std::move(initParameterDeclarations)) {}

        const Kind callableDeclarationKind;
        const StorageClass storageClass = StorageClass::Auto;
        const std::vector<ParameterDeclaration*> parameterDeclarations;
        const Statement* body = nullptr;
    };

    class FunctionDeclaration final: public CallableDeclaration
    {
    public:
        enum class Qualifier
        {
            None,
            Fragment,
            Vertex
        };

        FunctionDeclaration(const std::string& initName,
                            const QualifiedType& initQualifiedType,
                            StorageClass initStorageClass,
                            std::vector<const Attribute*> initAttributes,
                            std::vector<ParameterDeclaration*> initParameterDeclarations,
                            Qualifier initQualifier,
                            bool initIsBuiltin = false):
            CallableDeclaration(CallableDeclaration::Kind::Function, initName,
                                initStorageClass,
                                std::move(initAttributes),
                                std::move(initParameterDeclarations)),
            qualifier(initQualifier),
            resultType(initQualifiedType),
            isBuiltin(initIsBuiltin) {}

        const Qualifier qualifier = Qualifier::None;
        const QualifiedType resultType;
        const bool isBuiltin = false;
    };

    class ConstructorDeclaration final: public CallableDeclaration
    {
    public:
        ConstructorDeclaration(StorageClass initStorageClass,
                               std::vector<const Attribute*> initAttributes,
                               std::vector<ParameterDeclaration*> initParameterDeclarations):
            CallableDeclaration(CallableDeclaration::Kind::Constructor,
                                initStorageClass,
                                std::move(initAttributes),
                                std::move(initParameterDeclarations)) {}
    };

    class MethodDeclaration final: public CallableDeclaration
    {
    public:
        MethodDeclaration(const std::string& initName,
                          const QualifiedType& initQualifiedType,
                          StorageClass initStorageClass,
                          std::vector<const Attribute*> initAttributes,
                          std::vector<ParameterDeclaration*> initParameterDeclarations,
                          bool initIsBuiltin = false):
            CallableDeclaration(CallableDeclaration::Kind::Method, initName,
                                initStorageClass,
                                std::move(initAttributes),
                                std::move(initParameterDeclarations)),
            resultType(initQualifiedType),
            isBuiltin(initIsBuiltin) {}

        const QualifiedType resultType;
        const bool isBuiltin = false;
    };

    class VariableDeclaration final: public Declaration
    {
    public:
        VariableDeclaration(const std::string& initName,
                            const QualifiedType& initQualifiedType,
                            StorageClass initStorageClass,
                            const Expression* initInitialization = nullptr) noexcept:
            Declaration(Declaration::Kind::Variable, initName, {}),
            storageClass(initStorageClass),
            qualifiedType(initQualifiedType),
            initialization(initInitialization)
        {
            definition = this;
        }

        const StorageClass storageClass = StorageClass::Auto;
        const QualifiedType qualifiedType;
        const Expression* initialization = nullptr;
    };

    class TypeDeclaration final: public Declaration
    {
    public:
        TypeDeclaration(const std::string& initName, const Type& initType) noexcept:
            Declaration(Declaration::Kind::Type, initName, {}),
            type(initType) {}

        const Type& type;
    };
}

#endif // DECLARATIONS_HPP
