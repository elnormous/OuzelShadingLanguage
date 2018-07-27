//
//  OSL
//

#pragma once

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
        NONE,
        EMPTY,
        TYPE,
        FIELD,
        CALLABLE,
        VARIABLE,
        PARAMETER
    };

    Declaration(Kind initDeclarationKind): Construct(Construct::Kind::DECLARATION), declarationKind(initDeclarationKind) {}

    inline Kind getDeclarationKind() const { return declarationKind; }

    Declaration* getFirstDeclaration()
    {
        Declaration* result = this;

        while (result->previousDeclaration) result = result->previousDeclaration;

        return result;
    }

    std::string name;
    Declaration* previousDeclaration = nullptr;
    Declaration* definition = nullptr;

protected:
    Kind declarationKind = Kind::NONE;
};

class TypeDeclaration: public Declaration
{
public:
    enum class Kind
    {
        NONE,
        ARRAY,
        SCALAR,
        STRUCT
        //TYPE_DEFINITION // typedef is not supported in GLSL
    };

    TypeDeclaration(Kind initTypeKind): Declaration(Declaration::Kind::TYPE), typeKind(initTypeKind) {}

    inline Kind getTypeKind() const { return typeKind; }

    bool isBuiltin = false;

protected:
    Kind typeKind = Kind::NONE;
};

class ArrayTypeDeclaration: public TypeDeclaration
{
public:
    ArrayTypeDeclaration(): TypeDeclaration(TypeDeclaration::Kind::ARRAY)
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
        NONE,
        BOOLEAN,
        INTEGER,
        FLOATING_POINT
    };

    ScalarTypeDeclaration(Kind initScalarTypeKind): TypeDeclaration(TypeDeclaration::Kind::SCALAR), scalarTypeKind(initScalarTypeKind)
    {
        definition = this;
    }

    inline Kind getScalarTypeKind() const { return scalarTypeKind; }

    bool isUnsigned = false;
    
protected:
    Kind scalarTypeKind;
};

class FieldDeclaration: public Declaration
{
public:
    FieldDeclaration(): Declaration(Declaration::Kind::FIELD)
    {
        definition = this;
    }

    QualifiedType qualifiedType;

    Semantic semantic = Semantic::NONE;
};

class ParameterDeclaration: public Declaration
{
public:
    ParameterDeclaration(): Declaration(Declaration::Kind::PARAMETER)
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
    enum Kind
    {
        NONE,
        FUNCTION,
        CONSTRUCTOR,
        METHOD,
        OPERATOR
    };

    CallableDeclaration(Kind initCallableDeclarationKind): Declaration(Declaration::Kind::CALLABLE), callableDeclarationKind(initCallableDeclarationKind) {}

    inline Kind getCallableDeclarationKind() const { return callableDeclarationKind; }

    QualifiedType qualifiedType;
    std::vector<ParameterDeclaration*> parameterDeclarations;
    Statement* body = nullptr;

protected:
    Kind callableDeclarationKind;
};

class FunctionDeclaration: public CallableDeclaration
{
public:
    FunctionDeclaration(): CallableDeclaration(CallableDeclaration::Kind::FUNCTION) {}

    bool isInline = false;
    bool isStatic = false;
    bool isBuiltin = false;
    Program program = Program::NONE;
};

class ConstructorDeclaration: public CallableDeclaration
{
public:
    ConstructorDeclaration(): CallableDeclaration(CallableDeclaration::Kind::CONSTRUCTOR) {}
};

class OperatorDeclaration: public CallableDeclaration
{
public:
    OperatorDeclaration(): CallableDeclaration(CallableDeclaration::Kind::OPERATOR) {}

    Operator op;
};

class StructDeclaration: public TypeDeclaration
{
public:
    StructDeclaration(): TypeDeclaration(TypeDeclaration::Kind::STRUCT) {}

    ConstructorDeclaration* findConstructorDeclaration(const std::vector<QualifiedType>& parameters) const
    {
        for (Declaration* declaration : memberDeclarations)
        {
            if (declaration->getDeclarationKind() == Declaration::Kind::CALLABLE)
            {
                CallableDeclaration* callableDeclaration = static_cast<CallableDeclaration*>(declaration);

                if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::CONSTRUCTOR)
                {
                    ConstructorDeclaration* constructorDeclaration = static_cast<ConstructorDeclaration*>(callableDeclaration);

                    if (constructorDeclaration->parameterDeclarations.size() == parameters.size())
                    {
                        if (std::equal(parameters.begin(), parameters.end(),
                                       constructorDeclaration->parameterDeclarations.begin(),
                                       [](const QualifiedType& qualifiedType,
                                          const ParameterDeclaration* parameterDeclaration) {
                                           return qualifiedType.typeDeclaration->getFirstDeclaration() == parameterDeclaration->qualifiedType.typeDeclaration->getFirstDeclaration(); // TODO: type promotion
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

    Declaration* findMemberDeclaration(const std::string& name) const
    {
        for (Declaration* memberDeclaration : memberDeclarations)
        {
            if (memberDeclaration->name == name) return memberDeclaration;
        }

        return nullptr;
    }

    std::vector<Declaration*> memberDeclarations;
};

class VariableDeclaration: public Declaration
{
public:
    VariableDeclaration(): Declaration(Declaration::Kind::VARIABLE)
    {
        definition = this;
    }

    QualifiedType qualifiedType;
    Expression* initialization = nullptr;

    bool isStatic = false;
};
