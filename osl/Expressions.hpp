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

    private:
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

    private:
        const Kind literalKind;
    };

    class BooleanLiteralExpression final: public LiteralExpression
    {
    public:
        BooleanLiteralExpression(bool initValue) noexcept:
            LiteralExpression(LiteralExpression::Kind::Boolean),
            value(initValue) {}
        bool value;
    };

    class IntegerLiteralExpression final: public LiteralExpression
    {
    public:
        IntegerLiteralExpression(int64_t initValue) noexcept:
            LiteralExpression(LiteralExpression::Kind::Integer),
            value(initValue) {}

        int64_t value;
    };

    class FloatingPointLiteralExpression final: public LiteralExpression
    {
    public:
        FloatingPointLiteralExpression(double initValue) noexcept:
            LiteralExpression(LiteralExpression::Kind::FloatingPoint),
            value(initValue) {}

        double value;
    };

    class StringLiteralExpression final: public LiteralExpression
    {
    public:
        StringLiteralExpression(const std::string& initValue):
            LiteralExpression(LiteralExpression::Kind::String),
            value(initValue) {}

        std::string value;
    };

    class DeclarationReferenceExpression final: public Expression
    {
    public:
        DeclarationReferenceExpression(const Declaration* initDeclaration):
            Expression(Expression::Kind::DeclarationReference),
            declaration(initDeclaration) {}

        const Declaration* declaration = nullptr;
    };

    class CallExpression final: public Expression
    {
    public:
        CallExpression(): Expression(Expression::Kind::Call) {}

        const DeclarationReferenceExpression* declarationReference = nullptr;
        std::vector<const Expression*> arguments;
    };

    class ParenExpression final: public Expression
    {
    public:
        ParenExpression(const Expression* initExpression) noexcept:
            Expression(Expression::Kind::Paren),
            expression(initExpression) {}

        const Expression* expression = nullptr;
    };

    class MemberExpression final: public Expression
    {
    public:
        MemberExpression(const Expression* initExpression,
                         const FieldDeclaration* initFieldDeclaration) noexcept:
            Expression(Expression::Kind::Member),
            expression(initExpression),
            fieldDeclaration(initFieldDeclaration) {}

        const Expression* expression = nullptr;
        const FieldDeclaration* fieldDeclaration = nullptr;
    };

    class ArraySubscriptExpression final: public Expression
    {
    public:
        ArraySubscriptExpression(const Expression* initExpression,
                                 const Expression* initSubscript) noexcept:
            Expression(Expression::Kind::ArraySubscript),
            expression(initExpression),
            subscript(initSubscript) {}

        const Expression* expression = nullptr;
        const Expression* subscript = nullptr;
    };

    class UnaryOperatorExpression final: public Expression
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

        UnaryOperatorExpression(Kind initOperatorKind,
                                const Expression* initExpression) noexcept:
            Expression(Expression::Kind::UnaryOperator),
            expression(initExpression),
            operatorKind(initOperatorKind)
        {}

        inline Kind getOperatorKind() const noexcept { return operatorKind; }

        const Expression* expression = nullptr;

    private:
        Kind operatorKind;
    };

    class BinaryOperatorExpression final: public Expression
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

        BinaryOperatorExpression(Kind initOperatorKind,
                                 const Expression* initLeftExpression,
                                 const Expression* initRightExpression) noexcept:
            Expression(Expression::Kind::BinaryOperator),
            leftExpression(initLeftExpression),
            rightExpression(initRightExpression),
            operatorKind(initOperatorKind)
        {}

        inline Kind getOperatorKind() const noexcept { return operatorKind; }

        const Expression* leftExpression = nullptr;
        const Expression* rightExpression = nullptr;

    private:
        Kind operatorKind;
    };

    class TernaryOperatorExpression final: public Expression
    {
    public:
        TernaryOperatorExpression(const Expression* initCondition,
                                  const Expression* initLeftExpression,
                                  const Expression* initRightExpression) noexcept:
            Expression(Expression::Kind::TernaryOperator),
            condition(initCondition),
            leftExpression(initLeftExpression),
            rightExpression(initRightExpression) {}

        const Expression* condition;
        const Expression* leftExpression = nullptr;
        const Expression* rightExpression = nullptr;
    };

    class TemporaryObjectExpression final: public Expression
    {
    public:
        TemporaryObjectExpression(): Expression(Expression::Kind::TemporaryObject) {}

        const ConstructorDeclaration* constructorDeclaration = nullptr;
        std::vector<const Expression*> parameters;
    };

    class InitializerListExpression final: public Expression
    {
    public:
        InitializerListExpression(): Expression(Expression::Kind::InitializerList) {}

        std::vector<const Expression*> expressions;
    };

    class CastExpression final: public Expression
    {
    public:
        enum class Kind
        {
            CStyle,
            Functional,
            Static
        };

        CastExpression(Kind initCastKind,
                       const Expression* initExpression) noexcept:
            Expression(Expression::Kind::Cast),
            expression(initExpression),
            castKind(initCastKind) {}

        inline Kind getCastKind() const noexcept { return castKind; }

        const Expression* expression;

    private:
        const Kind castKind;
    };

    using TypeOrExpression = Variant<Type*, Expression*>;

    class SizeofExpression final: public Expression
    {
    public:
        SizeofExpression(const Expression* initExpression) noexcept:
            Expression(Expression::Kind::Sizeof),
            expression(initExpression) {}

        SizeofExpression(const Type* initType) noexcept:
            Expression(Expression::Kind::Sizeof),
            type(initType) {}

        const Expression* expression;
        const Type* type;
    };

    class VectorInitializeExpression final: public Expression
    {
    public:
        VectorInitializeExpression() noexcept: Expression(Expression::Kind::VectorInitialize) {}

        std::vector<const Expression*> parameters;
    };

    class VectorElementExpression final: public Expression
    {
    public:
        VectorElementExpression() noexcept: Expression(Expression::Kind::VectorElement) {}

        uint8_t count = 0;
        uint8_t positions[4]{};
    };

    class MatrixInitializeExpression final: public Expression
    {
    public:
        MatrixInitializeExpression() noexcept: Expression(Expression::Kind::MatrixInitialize) {}

        std::vector<const Expression*> parameters;
    };
}

#endif // EXPRESSIONS_HPP
