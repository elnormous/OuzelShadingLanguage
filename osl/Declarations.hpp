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
                    const QualifiedType& initQualifiedType,
                    std::vector<const Attribute*> initAttributes = {}):
            Construct(Construct::Kind::Declaration),
            qualifiedType(initQualifiedType),
            declarationKind(initDeclarationKind),
            attributes(std::move(initAttributes)) {}

        Declaration(Kind initDeclarationKind,
                    const std::string& initName,
                    const QualifiedType& initQualifiedType,
                    std::vector<const Attribute*> initAttributes = {}):
            Construct(Construct::Kind::Declaration),
            name(initName),
            qualifiedType(initQualifiedType),
            declarationKind(initDeclarationKind),
            attributes(std::move(initAttributes)) {}

        Declaration& operator=(const Declaration&) = delete;

        inline Kind getDeclarationKind() const noexcept { return declarationKind; }

        std::string name;
        QualifiedType qualifiedType;
        Declaration* firstDeclaration = nullptr;
        Declaration* previousDeclaration = nullptr;
        Declaration* definition = nullptr;
        std::vector<const Attribute*> attributes;

    private:
        const Kind declarationKind;
    };

    class FieldDeclaration final: public Declaration
    {
    public:
        FieldDeclaration(const std::string& initName,
                         const QualifiedType& initQualifiedType,
                         std::vector<const Attribute*> initAttributes) noexcept:
            Declaration(Declaration::Kind::Field, initName, initQualifiedType, std::move(initAttributes))
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
            Declaration(Declaration::Kind::Parameter, initQualifiedType, {}),
            inputModifier(initInputModifier)
        {
            definition = this;
        }

        ParameterDeclaration(const std::string& initName,
                             const QualifiedType& initQualifiedType,
                             InputModifier initInputModifier) noexcept:
            Declaration(Declaration::Kind::Parameter, initName, initQualifiedType, {}),
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
                            StorageClass initStorageClass,
                            std::vector<const Attribute*> initAttributes,
                            std::vector<ParameterDeclaration*> initParameterDeclarations):
            Declaration(Declaration::Kind::Callable, initQualifiedType, std::move(initAttributes)),
            callableDeclarationKind(initCallableDeclarationKind),
            storageClass(initStorageClass),
            parameterDeclarations(std::move(initParameterDeclarations)) {}

        CallableDeclaration(Kind initCallableDeclarationKind,
                            const std::string& initName,
                            const QualifiedType& initQualifiedType,
                            StorageClass initStorageClass,
                            std::vector<const Attribute*> initAttributes,
                            std::vector<ParameterDeclaration*> initParameterDeclarations):
            Declaration(Declaration::Kind::Callable, initName, initQualifiedType, std::move(initAttributes)),
            callableDeclarationKind(initCallableDeclarationKind),
            storageClass(initStorageClass),
            parameterDeclarations(std::move(initParameterDeclarations)) {}

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
                                initQualifiedType,
                                initStorageClass,
                                std::move(initAttributes),
                                std::move(initParameterDeclarations)),
            qualifier(initQualifier),
            isBuiltin(initIsBuiltin) {}

        Qualifier qualifier = Qualifier::None;
        bool isBuiltin = false;
    };

    class ConstructorDeclaration final: public CallableDeclaration
    {
    public:
        ConstructorDeclaration(StorageClass initStorageClass,
                               std::vector<const Attribute*> initAttributes,
                               std::vector<ParameterDeclaration*> initParameterDeclarations):
            CallableDeclaration(CallableDeclaration::Kind::Constructor,
                                QualifiedType{nullptr},
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
                                initQualifiedType,
                                initStorageClass,
                                std::move(initAttributes),
                                std::move(initParameterDeclarations)),
            isBuiltin(initIsBuiltin) {}

        bool isBuiltin = false;
    };

    class VariableDeclaration final: public Declaration
    {
    public:
        VariableDeclaration(const std::string& initName,
                            const QualifiedType& initQualifiedType,
                            StorageClass initStorageClass,
                            const Expression* initInitialization = nullptr) noexcept:
            Declaration(Declaration::Kind::Variable, initName, initQualifiedType, {}),
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
        TypeDeclaration(const std::string& initName, const Type& initType) noexcept:
            Declaration(Declaration::Kind::Type, initName, QualifiedType{&initType}, {}),
            type(initType) {}

        const Type& type;
    };
}

#endif // DECLARATIONS_HPP
