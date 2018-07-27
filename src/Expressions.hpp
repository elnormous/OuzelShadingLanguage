//
//  OSL
//

#pragma once

#include "Construct.hpp"
#include "QualifiedType.hpp"

class Expression: public Construct
{
public:
    enum class Kind
    {
        NONE,
        CALL,
        LITERAL,
        DECLARATION_REFERENCE,
        PAREN,
        MEMBER,
        ARRAY_SUBSCRIPT,
        UNARY_OPERATOR,
        BINARY_OPERATOR,
        TERNARY_OPERATOR,
        TEMPORARY_OBJECT,
        INITIALIZER_LIST,
        CAST
    };

    Expression(Kind initExpressionKind): Construct(Construct::Kind::EXPRESSION), expressionKind(initExpressionKind) {}

    inline Kind getExpressionKind() const { return expressionKind; }

    QualifiedType qualifiedType;
    bool isLValue = false;

protected:
    Kind expressionKind = Kind::NONE;
};

class LiteralExpression: public Expression
{
public:
    enum class Kind
    {
        NONE,
        BOOLEAN,
        INTEGER,
        FLOATING_POINT,
        STRING
    };

    LiteralExpression(Kind initLiteralKind): Expression(Expression::Kind::LITERAL), literalKind(initLiteralKind) {}

    inline Kind getLiteralKind() const { return literalKind; }

    TypeDeclaration* typeDeclaration = nullptr;

protected:
    Kind literalKind = Kind::NONE;
};

class BooleanLiteralExpression: public LiteralExpression
{
public:
    BooleanLiteralExpression(): LiteralExpression(LiteralExpression::Kind::BOOLEAN) {}
    bool value;
};

class IntegerLiteralExpression: public LiteralExpression
{
public:
    IntegerLiteralExpression(): LiteralExpression(LiteralExpression::Kind::INTEGER) {}

    int64_t value;
};

class FloatingPointLiteralExpression: public LiteralExpression
{
public:
    FloatingPointLiteralExpression(): LiteralExpression(LiteralExpression::Kind::FLOATING_POINT) {}

    double value;
};

class StringLiteralExpression: public LiteralExpression
{
public:
    StringLiteralExpression(): LiteralExpression(LiteralExpression::Kind::STRING) {}

    std::string value;
};

class DeclarationReferenceExpression: public Expression
{
public:
    DeclarationReferenceExpression(): Expression(Expression::Kind::DECLARATION_REFERENCE) {}

    Declaration* declaration = nullptr;
};

class CallExpression: public Expression
{
public:
    CallExpression(): Expression(Expression::Kind::CALL) {}

    DeclarationReferenceExpression* declarationReference = nullptr;
    std::vector<Expression*> parameters;
};

class ParenExpression: public Expression
{
public:
    ParenExpression(): Expression(Expression::Kind::PAREN) {}

    Expression* expression = nullptr;
};

class MemberExpression: public Expression
{
public:
    MemberExpression(): Expression(Expression::Kind::MEMBER) {}

    Expression* expression = nullptr;
    FieldDeclaration* fieldDeclaration = nullptr;
};

class ArraySubscriptExpression: public Expression
{
public:
    ArraySubscriptExpression(): Expression(Expression::Kind::ARRAY_SUBSCRIPT) {}

    Expression* expression = nullptr;
    Expression* subscript = nullptr;
};

class UnaryOperatorExpression: public Expression
{
public:
    enum class Kind
    {
        NONE,
        NEGATION, // !
        POSITIVE, // +
        NEGATIVE // -
    };

    UnaryOperatorExpression(): Expression(Expression::Kind::UNARY_OPERATOR) {}

    OperatorDeclaration* operatorDeclaration = nullptr;

    Expression* expression = nullptr;

    Kind unaryOperatorKind = Kind::NONE;
};

class BinaryOperatorExpression: public Expression
{
public:
    enum class Kind
    {
        NONE,
        ADDITION, // +
        SUBTRACTION, // -
        MULTIPLICATION, // *
        DIVISION, // /
        ADDITION_ASSIGNMENT, // +=
        SUBTRACTION_ASSIGNMENT, // -=
        MULTIPLICATION_ASSIGNMENT, // *=
        DIVISION_ASSIGNMENT, // /=
        LESS_THAN, // <
        LESS_THAN_EQUAL, // <=
        GREATER_THAN, // >
        GREATER_THAN_EQUAL, // >=
        EQUALITY, // ==
        INEQUALITY, // !=
        ASSIGNMENT, // =
        OR, // ||
        AND, // &&
        COMMA // ,
    };

    BinaryOperatorExpression(): Expression(Expression::Kind::BINARY_OPERATOR) {}

    OperatorDeclaration* operatorDeclaration = nullptr;

    Expression* leftExpression = nullptr;
    Expression* rightExpression = nullptr;

    Kind binaryOperatorKind = Kind::NONE;
};

class TernaryOperatorExpression: public Expression
{
public:
    TernaryOperatorExpression(): Expression(Expression::Kind::TERNARY_OPERATOR) {}

    OperatorDeclaration* operatorDeclaration = nullptr;

    Expression* condition;
    Expression* leftExpression = nullptr;
    Expression* rightExpression = nullptr;
};

class TemporaryObjectExpression: public Expression
{
public:
    TemporaryObjectExpression(): Expression(Expression::Kind::TEMPORARY_OBJECT) {}

    ConstructorDeclaration* constructorDeclaration = nullptr;
    std::vector<Expression*> parameters;
};

class InitializerListExpression: public Expression
{
public:
    InitializerListExpression(): Expression(Expression::Kind::INITIALIZER_LIST) {}

    std::vector<Expression*> expressions;
};

class CastExpression: public Expression
{
public:
    enum class Kind
    {
        NONE,
        IMPLICIT,
        EXPLICIT
    };

    CastExpression(Kind initCastKind): Expression(Expression::Kind::CAST), castKind(initCastKind) {}

    inline Kind getCastKind() const { return castKind; }

    Expression* expression;

protected:
    Kind castKind;
};
