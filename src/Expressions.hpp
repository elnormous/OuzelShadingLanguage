//
//  OSL
//

#ifndef EXPRESSIONS_HPP
#define EXPRESSIONS_HPP

#include "Construct.hpp"
#include "QualifiedType.hpp"

class Expression: public Construct
{
public:
    enum class Kind
    {
        Call,
        Literal,
        DeclarationReference,
        Paren,
        Member,
        ArraySubscript,
        UnaryOperator,
        BinaryOperator,
        TernaryOperator,
        TemporaryObject,
        InitializerList,
        Cast,
        Sizeof
    };

    enum class Category: uint8_t
    {
        Lvalue,
        Rvalue
    };

    Expression(Kind initExpressionKind) noexcept: Construct(Construct::Kind::Expression), expressionKind(initExpressionKind) {}

    inline Kind getExpressionKind() const noexcept { return expressionKind; }

    QualifiedType qualifiedType;
    Category category = Category::Rvalue;

protected:
    const Kind expressionKind;
};

class LiteralExpression: public Expression
{
public:
    enum class Kind
    {
        Boolean,
        Integer,
        FloatingPoint,
        String
    };

    LiteralExpression(Kind initLiteralKind) noexcept: Expression(Expression::Kind::Literal), literalKind(initLiteralKind) {}

    inline Kind getLiteralKind() const noexcept { return literalKind; }

    TypeDeclaration* typeDeclaration = nullptr;

protected:
    const Kind literalKind;
};

class BooleanLiteralExpression: public LiteralExpression
{
public:
    BooleanLiteralExpression(): LiteralExpression(LiteralExpression::Kind::Boolean) {}
    bool value;
};

class IntegerLiteralExpression: public LiteralExpression
{
public:
    IntegerLiteralExpression(): LiteralExpression(LiteralExpression::Kind::Integer) {}

    int64_t value;
};

class FloatingPointLiteralExpression: public LiteralExpression
{
public:
    FloatingPointLiteralExpression(): LiteralExpression(LiteralExpression::Kind::FloatingPoint) {}

    double value;
};

class StringLiteralExpression: public LiteralExpression
{
public:
    StringLiteralExpression(): LiteralExpression(LiteralExpression::Kind::String) {}

    std::string value;
};

class DeclarationReferenceExpression: public Expression
{
public:
    DeclarationReferenceExpression(): Expression(Expression::Kind::DeclarationReference) {}

    Declaration* declaration = nullptr;
};

class CallExpression: public Expression
{
public:
    CallExpression(): Expression(Expression::Kind::Call) {}

    DeclarationReferenceExpression* declarationReference = nullptr;
    std::vector<Expression*> arguments;
};

class ParenExpression: public Expression
{
public:
    ParenExpression(): Expression(Expression::Kind::Paren) {}

    Expression* expression = nullptr;
};

class MemberExpression: public Expression
{
public:
    MemberExpression(): Expression(Expression::Kind::Member) {}

    Expression* expression = nullptr;
    FieldDeclaration* fieldDeclaration = nullptr;
};

class ArraySubscriptExpression: public Expression
{
public:
    ArraySubscriptExpression(): Expression(Expression::Kind::ArraySubscript) {}

    Expression* expression = nullptr;
    Expression* subscript = nullptr;
};

class UnaryOperatorExpression: public Expression
{
public:
    UnaryOperatorExpression(): Expression(Expression::Kind::UnaryOperator) {}

    OperatorDeclaration* operatorDeclaration = nullptr;

    Expression* expression = nullptr;
};

class BinaryOperatorExpression: public Expression
{
public:
    BinaryOperatorExpression(): Expression(Expression::Kind::BinaryOperator) {}

    OperatorDeclaration* operatorDeclaration = nullptr;

    Expression* leftExpression = nullptr;
    Expression* rightExpression = nullptr;
};

class TernaryOperatorExpression: public Expression
{
public:
    TernaryOperatorExpression(): Expression(Expression::Kind::TernaryOperator) {}

    OperatorDeclaration* operatorDeclaration = nullptr;

    Expression* condition;
    Expression* leftExpression = nullptr;
    Expression* rightExpression = nullptr;
};

class TemporaryObjectExpression: public Expression
{
public:
    TemporaryObjectExpression(): Expression(Expression::Kind::TemporaryObject) {}

    ConstructorDeclaration* constructorDeclaration = nullptr;
    std::vector<Expression*> parameters;
};

class InitializerListExpression: public Expression
{
public:
    InitializerListExpression(): Expression(Expression::Kind::InitializerList) {}

    std::vector<Expression*> expressions;
};

class CastExpression: public Expression
{
public:
    enum class Kind
    {
        Implicit,
        CStyle,
        Functional,
        Const,
        Dynamic,
        Reinterpet,
        Static
    };

    CastExpression(Kind initCastKind) noexcept: Expression(Expression::Kind::Cast), castKind(initCastKind) {}

    inline Kind getCastKind() const noexcept { return castKind; }

    Expression* expression;

protected:
    const Kind castKind;
};

class SizeofExpression: public Expression
{
public:
    SizeofExpression(): Expression(Expression::Kind::Sizeof) {}
    
    Expression* expression;
    TypeDeclaration* type;
};

#endif // EXPRESSIONS_HPP
