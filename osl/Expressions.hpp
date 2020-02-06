//
//  OSL
//

#ifndef EXPRESSIONS_HPP
#define EXPRESSIONS_HPP

#include "Construct.hpp"
#include "QualifiedType.hpp"

namespace ouzel
{
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
            VectorInitialize,
            VectorElement,
            MatrixInitialize
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

        const Declaration* declaration = nullptr;
    };

    class CallExpression: public Expression
    {
    public:
        CallExpression(): Expression(Expression::Kind::Call) {}

        const DeclarationReferenceExpression* declarationReference = nullptr;
        std::vector<const Expression*> arguments;
    };

    class ParenExpression: public Expression
    {
    public:
        ParenExpression() noexcept: Expression(Expression::Kind::Paren) {}

        const Expression* expression = nullptr;
    };

    class MemberExpression: public Expression
    {
    public:
        MemberExpression() noexcept: Expression(Expression::Kind::Member) {}

        const Expression* expression = nullptr;
        const FieldDeclaration* fieldDeclaration = nullptr;
    };

    class ArraySubscriptExpression: public Expression
    {
    public:
        ArraySubscriptExpression() noexcept: Expression(Expression::Kind::ArraySubscript) {}

        const Expression* expression = nullptr;
        const Expression* subscript = nullptr;
    };

    class UnaryOperatorExpression: public Expression
    {
    public:
        enum class Kind
        {
            Negation, // !
            Positive, // +
            Negative, // -
            PrefixIncrement, // ++
            PrefixDecrement, // --
            PostfixIncrement, // ++
            PostfixDecrement, // --
        };

        UnaryOperatorExpression(Kind initOperatorKind) noexcept:
            Expression(Expression::Kind::UnaryOperator),
            operatorKind(initOperatorKind)
        {}

        inline Kind getOperatorKind() const noexcept { return operatorKind; }

        const Expression* expression = nullptr;

    private:
        Kind operatorKind;
    };

    class BinaryOperatorExpression: public Expression
    {
    public:
        enum class Kind
        {
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

        const Expression* leftExpression = nullptr;
        const Expression* rightExpression = nullptr;

    private:
        Kind operatorKind;
    };

    class TernaryOperatorExpression: public Expression
    {
    public:
        TernaryOperatorExpression() noexcept: Expression(Expression::Kind::TernaryOperator) {}

        const Expression* condition;
        const Expression* leftExpression = nullptr;
        const Expression* rightExpression = nullptr;
    };

    class OperatorCallExpression: public Expression
    {
    public:
        OperatorCallExpression() noexcept: Expression(Expression::Kind::OperatorCall) {}

        const DeclarationReferenceExpression* declarationReference = nullptr;
        std::vector<const Expression*> arguments;
    };

    class TemporaryObjectExpression: public Expression
    {
    public:
        TemporaryObjectExpression(): Expression(Expression::Kind::TemporaryObject) {}

        const ConstructorDeclaration* constructorDeclaration = nullptr;
        std::vector<const Expression*> parameters;
    };

    class InitializerListExpression: public Expression
    {
    public:
        InitializerListExpression(): Expression(Expression::Kind::InitializerList) {}

        std::vector<const Expression*> expressions;
    };

    class CastExpression: public Expression
    {
    public:
        enum class Kind
        {
            CStyle,
            Functional,
            Static
        };

        CastExpression(Kind initCastKind) noexcept: Expression(Expression::Kind::Cast), castKind(initCastKind) {}

        inline Kind getCastKind() const noexcept { return castKind; }

        const Expression* expression;

    protected:
        const Kind castKind;
    };

    class SizeofExpression: public Expression
    {
    public:
        SizeofExpression() noexcept: Expression(Expression::Kind::Sizeof) {}

        const Expression* expression;
        const Type* type;
    };

    class VectorInitializeExpression: public Expression
    {
    public:
        VectorInitializeExpression() noexcept: Expression(Expression::Kind::VectorInitialize) {}

        std::vector<const Expression*> parameters;
    };

    class VectorElementExpression: public Expression
    {
    public:
        VectorElementExpression() noexcept: Expression(Expression::Kind::VectorElement) {}

        uint8_t count = 0;
        uint8_t positions[4]{};
    };

    class MatrixInitializeExpression: public Expression
    {
    public:
        MatrixInitializeExpression() noexcept: Expression(Expression::Kind::MatrixInitialize) {}

        std::vector<const Expression*> parameters;
    };
}

#endif // EXPRESSIONS_HPP
