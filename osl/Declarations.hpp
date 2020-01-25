//
//  OSL
//

#ifndef DECLARATIONS_HPP
#define DECLARATIONS_HPP

#include "Construct.hpp"
#include "Operator.hpp"
#include "QualifiedType.hpp"

class Expression;
class Statement;

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

    Declaration(Kind initDeclarationKind): Construct(Construct::Kind::Declaration), declarationKind(initDeclarationKind) {}

    inline Kind getDeclarationKind() const noexcept { return declarationKind; }

    Declaration* getFirstDeclaration() noexcept
    {
        Declaration* result = this;

        while (result->previousDeclaration) result = result->previousDeclaration;

        return result;
    }

    std::string name;
    Declaration* previousDeclaration = nullptr;
    Declaration* definition = nullptr;

protected:
    const Kind declarationKind;
};

class TypeDeclaration: public Declaration
{
public:
    enum class Kind
    {
        Array,
        Scalar,
        Struct,
        Vector
        //TypeDefinition // typedef is not supported in GLSL
    };

    TypeDeclaration(Kind initTypeKind) noexcept: Declaration(Declaration::Kind::Type), typeKind(initTypeKind) {}

    inline Kind getTypeKind() const noexcept { return typeKind; }

    uint32_t size = 0;
    bool isBuiltin = false;

protected:
    const Kind typeKind;
};

class ArrayTypeDeclaration: public TypeDeclaration
{
public:
    ArrayTypeDeclaration() noexcept: TypeDeclaration(TypeDeclaration::Kind::Array)
    {
        definition = this;
    }

    QualifiedType elementType;
    uint32_t size = 0;
};

class ScalarTypeDeclaration: public TypeDeclaration
{
public:
    enum class Kind
    {
        Boolean,
        Integer,
        FloatingPoint
    };

    ScalarTypeDeclaration(Kind initScalarTypeKind) noexcept: TypeDeclaration(TypeDeclaration::Kind::Scalar), scalarTypeKind(initScalarTypeKind)
    {
        definition = this;
    }

    inline Kind getScalarTypeKind() const noexcept { return scalarTypeKind; }

    bool isUnsigned = false;
    
protected:
    const Kind scalarTypeKind;
};

class FieldDeclaration: public Declaration
{
public:
    FieldDeclaration() noexcept: Declaration(Declaration::Kind::Field)
    {
        definition = this;
    }

    QualifiedType qualifiedType;

    Semantic semantic = Semantic::None;
};

class ParameterDeclaration: public Declaration
{
public:
    ParameterDeclaration() noexcept: Declaration(Declaration::Kind::Parameter)
    {
        definition = this;
    }

    QualifiedType qualifiedType;
};

/*class TypeDefinitionDeclaration: public TypeDeclaration
{
public:
    QualifiedType qualifiedType;
};*/

class CallableDeclaration: public Declaration
{
public:
    enum class Kind
    {
        Function,
        Constructor,
        Method,
        Operator
    };

    CallableDeclaration(Kind initCallableDeclarationKind): Declaration(Declaration::Kind::Callable), callableDeclarationKind(initCallableDeclarationKind) {}

    inline Kind getCallableDeclarationKind() const { return callableDeclarationKind; }

    QualifiedType qualifiedType;
    std::vector<ParameterDeclaration*> parameterDeclarations;
    Statement* body = nullptr;

protected:
    const Kind callableDeclarationKind;
};

class FunctionDeclaration: public CallableDeclaration
{
public:
    FunctionDeclaration(): CallableDeclaration(CallableDeclaration::Kind::Function) {}

    bool isInline = false;
    bool isStatic = false;
    bool isBuiltin = false;
    bool isProgram = false;
    Program program;
};

class ConstructorDeclaration: public CallableDeclaration
{
public:
    ConstructorDeclaration(): CallableDeclaration(CallableDeclaration::Kind::Constructor) {}
};

class MethodDeclaration: public CallableDeclaration
{
public:
    MethodDeclaration(): CallableDeclaration(CallableDeclaration::Kind::Method) {}

    bool isInline = false;
    bool isStatic = false;
    bool isBuiltin = false;
};

class OperatorDeclaration: public CallableDeclaration
{
public:
    OperatorDeclaration(): CallableDeclaration(CallableDeclaration::Kind::Operator) {}

    Operator op;
};

class StructDeclaration: public TypeDeclaration
{
public:
    StructDeclaration(): TypeDeclaration(TypeDeclaration::Kind::Struct) {}

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

                    if (constructorDeclaration->parameterDeclarations.size() == parameters.size())
                    {
                        if (std::equal(parameters.begin(), parameters.end(),
                                       constructorDeclaration->parameterDeclarations.begin(),
                                       [](const QualifiedType& qualifiedType,
                                          const ParameterDeclaration* parameterDeclaration) {
                                           return qualifiedType.typeDeclaration->getFirstDeclaration() == parameterDeclaration->qualifiedType.typeDeclaration->getFirstDeclaration(); // TODO: overload resolution
                                       }))
                        {
                            return constructorDeclaration;
                        }
                    }
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

class VectorTypeDeclaration: public TypeDeclaration
{
public:
    VectorTypeDeclaration() noexcept: TypeDeclaration(TypeDeclaration::Kind::Vector) {}

    TypeDeclaration* componentType = nullptr;
    uint8_t componentCount = 1;
};

class VariableDeclaration: public Declaration
{
public:
    enum class StorageClass
    {
        Auto,
        Extern,
        Static
    };

    VariableDeclaration() noexcept: Declaration(Declaration::Kind::Variable)
    {
        definition = this;
    }

    QualifiedType qualifiedType;
    Expression* initialization = nullptr;

    StorageClass storageClass = StorageClass::Auto;
};

#endif // DECLARATIONS_HPP
