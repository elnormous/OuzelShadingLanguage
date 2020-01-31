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
        OperatorCall,
        TemporaryObject,
        InitializerList,
        Cast,
        Sizeof,
        VectorElement,
        MatrixElement
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
    BooleanLiteralExpression() noexcept: LiteralExpression(LiteralExpression::Kind::Boolean) {}
    bool value;
};

class IntegerLiteralExpression: public LiteralExpression
{
public:
    IntegerLiteralExpression() noexcept: LiteralExpression(LiteralExpression::Kind::Integer) {}

    int64_t value;
};

class FloatingPointLiteralExpression: public LiteralExpression
{
public:
    FloatingPointLiteralExpression() noexcept: LiteralExpression(LiteralExpression::Kind::FloatingPoint) {}

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
    ParenExpression() noexcept: Expression(Expression::Kind::Paren) {}

    Expression* expression = nullptr;
};

class MemberExpression: public Expression
{
public:
    MemberExpression() noexcept: Expression(Expression::Kind::Member) {}

    Expression* expression = nullptr;
    FieldDeclaration* fieldDeclaration = nullptr;
};

class ArraySubscriptExpression: public Expression
{
public:
    ArraySubscriptExpression() noexcept: Expression(Expression::Kind::ArraySubscript) {}

    Expression* expression = nullptr;
    Expression* subscript = nullptr;
};

class UnaryOperatorExpression: public Expression
{
public:
    enum class Kind
    {
        Negation, // !
        Positive, // +
        Negative // -
    };

    UnaryOperatorExpression(Kind initOperatorKind) noexcept:
        Expression(Expression::Kind::UnaryOperator),
        operatorKind(initOperatorKind)
    {}

    inline Kind getOperatorKind() const noexcept { return operatorKind; }

    Expression* expression = nullptr;

private:
    Kind operatorKind;
};

class BinaryOperatorExpression: public Expression
{
public:
    enum class Kind
    {
        PrefixIncrement, // ++
        PrefixDecrement, // --
        PostfixIncrement, // ++
        PostfixDecrement, // --
        Addition, // +
        Subtraction, // -
        Multiplication, // *
        Division, // /
        AdditionAssignment, // +=
        SubtractAssignment, // -=
        MultiplicationAssignment, // *=
        DivisionAssignment, // /=
        LessThan, // <
        LessThanEqual, // <=
        GreaterThan, // >
        GraterThanEqual, // >=
        Equality, // ==
        Inequality, // !=
        Assignment, // =
        Or, // ||
        And, // &&
        Comma, // ,
        Subscript // []
    };

    BinaryOperatorExpression(Kind initOperatorKind) noexcept: Expression(Expression::Kind::BinaryOperator),
        operatorKind(initOperatorKind)
    {}

    inline Kind getOperatorKind() const noexcept { return operatorKind; }

    Expression* leftExpression = nullptr;
    Expression* rightExpression = nullptr;

private:
    Kind operatorKind;
};

class TernaryOperatorExpression: public Expression
{
public:
    TernaryOperatorExpression() noexcept: Expression(Expression::Kind::TernaryOperator) {}

    Expression* condition;
    Expression* leftExpression = nullptr;
    Expression* rightExpression = nullptr;
};

class OperatorCallExpression: public Expression
{
public:
    OperatorCallExpression() noexcept: Expression(Expression::Kind::OperatorCall) {}

    DeclarationReferenceExpression* declarationReference = nullptr;
    std::vector<Expression*> arguments;
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
    SizeofExpression() noexcept: Expression(Expression::Kind::Sizeof) {}
    
    Expression* expression;
    Type* type;
};

class VectorElementExpression: public Expression
{
public:
    VectorElementExpression() noexcept: Expression(Expression::Kind::VectorElement) {}

    uint8_t count = 0;
    uint8_t positions[4]{};
};

class MatrixElementExpression: public Expression
{
public:
    MatrixElementExpression() noexcept: Expression(Expression::Kind::MatrixElement) {}

    uint8_t count = 0;
    std::pair<uint8_t, uint8_t> positions[4]{};
};

#endif // EXPRESSIONS_HPP
