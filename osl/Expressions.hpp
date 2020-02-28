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
            VectorInitialize,
            VectorElement,
            MatrixInitialize
        };

        enum class Category: uint8_t
        {
            Lvalue,
            Rvalue
        };

        Expression(Kind initExpressionKind,
                   const QualifiedType& initQualifiedType,
                   Category initCategory) noexcept:
            Construct(Construct::Kind::Expression),
            qualifiedType(initQualifiedType),
            category(initCategory),
            expressionKind(initExpressionKind) {}

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

        LiteralExpression(Kind initLiteralKind,
                          const Type* type) noexcept:
            Expression(Expression::Kind::Literal,
                       QualifiedType{type, Qualifiers::Const},
                       Category::Rvalue),
            literalKind(initLiteralKind) {}

        inline Kind getLiteralKind() const noexcept { return literalKind; }

    private:
        const Kind literalKind;
    };

    class BooleanLiteralExpression final: public LiteralExpression
    {
    public:
        BooleanLiteralExpression(const Type* type,
                                 bool initValue) noexcept:
            LiteralExpression(LiteralExpression::Kind::Boolean, type),
            value(initValue) {}
        bool value;
    };

    class IntegerLiteralExpression final: public LiteralExpression
    {
    public:
        IntegerLiteralExpression(const Type* type,
                                 int64_t initValue) noexcept:
            LiteralExpression(LiteralExpression::Kind::Integer, type),
            value(initValue) {}

        int64_t value;
    };

    class FloatingPointLiteralExpression final: public LiteralExpression
    {
    public:
        FloatingPointLiteralExpression(const Type* type,
                                       double initValue) noexcept:
            LiteralExpression(LiteralExpression::Kind::FloatingPoint, type),
            value(initValue) {}

        double value;
    };

    class StringLiteralExpression final: public LiteralExpression
    {
    public:
        StringLiteralExpression(const Type* type,
                                const std::string& initValue):
            LiteralExpression(LiteralExpression::Kind::String, type),
            value(initValue) {}

        std::string value;
    };

    class DeclarationReferenceExpression final: public Expression
    {
    public:
        DeclarationReferenceExpression(const Declaration* initDeclaration,
                                       Category category):
            Expression(Expression::Kind::DeclarationReference,
                       initDeclaration->qualifiedType,
                       category),
            declaration(initDeclaration) {}

        const Declaration* declaration = nullptr;
    };

    class CallExpression final: public Expression
    {
    public:
        CallExpression():
            Expression(Expression::Kind::Call,
                       QualifiedType{nullptr}, // TODO
                       Category::Rvalue) {} // TODO

        const DeclarationReferenceExpression* declarationReference = nullptr;
        std::vector<const Expression*> arguments;
    };

    class ParenExpression final: public Expression
    {
    public:
        ParenExpression(const Expression* initExpression) noexcept:
            Expression(Expression::Kind::Paren,
                       initExpression->qualifiedType,
                       initExpression->category),
            expression(initExpression) {}

        const Expression* expression = nullptr;
    };

    class MemberExpression final: public Expression
    {
    public:
        MemberExpression(const Expression* initExpression,
                         const FieldDeclaration* initFieldDeclaration) noexcept:
            Expression(Expression::Kind::Member,
                       initFieldDeclaration->qualifiedType,
                       initExpression->category),
            expression(initExpression),
            fieldDeclaration(initFieldDeclaration) {}

        const Expression* expression = nullptr;
        const FieldDeclaration* fieldDeclaration = nullptr;
    };

    class ArraySubscriptExpression final: public Expression
    {
    public:
        ArraySubscriptExpression(const QualifiedType& elementType,
                                 const Expression* initExpression,
                                 const Expression* initSubscript) noexcept:
            Expression(Expression::Kind::ArraySubscript,
                       elementType,
                       initExpression->category),
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
                                const Type* type,
                                Category category,
                                const Expression* initExpression) noexcept:
            Expression(Expression::Kind::UnaryOperator,
                       QualifiedType{type},
                       category),
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
                                 const Type* type,
                                 Category category,
                                 const Expression* initLeftExpression,
                                 const Expression* initRightExpression) noexcept:
            Expression(Expression::Kind::BinaryOperator,
                       QualifiedType{type},
                       category),
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
            Expression(Expression::Kind::TernaryOperator,
                       initLeftExpression->qualifiedType,
                       (initLeftExpression->category == Expression::Category::Lvalue &&
                        initRightExpression->category == Expression::Category::Lvalue) ?
                       Expression::Category::Lvalue : Expression::Category::Rvalue),
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
        TemporaryObjectExpression(const Type* type):
            Expression(Expression::Kind::TemporaryObject,
                       QualifiedType{type, Qualifiers::Const},
                       Category::Rvalue) {}

        const ConstructorDeclaration* constructorDeclaration = nullptr;
        std::vector<const Expression*> parameters;
    };

    class InitializerListExpression final: public Expression
    {
    public:
        InitializerListExpression(const Type* type,
                                  std::vector<const Expression*> initExpressions):
            Expression(Expression::Kind::InitializerList,
                       QualifiedType{type, Qualifiers::Const},
                       Category::Rvalue),
            expressions(std::move(initExpressions)) {}

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
                       const Type* type,
                       const Expression* initExpression) noexcept:
            Expression(Expression::Kind::Cast,
                       QualifiedType{type, Qualifiers::Const},
                       Category::Rvalue),
            expression(initExpression),
            castKind(initCastKind) {}

        inline Kind getCastKind() const noexcept { return castKind; }

        const Expression* expression;

    private:
        const Kind castKind;
    };

    class VectorInitializeExpression final: public Expression
    {
    public:
        VectorInitializeExpression(const VectorType* vectorType,
                                   std::vector<const Expression*> initParameters) noexcept:
            Expression(Expression::Kind::VectorInitialize,
                       QualifiedType{vectorType, Qualifiers::Const},
                       Category::Rvalue),
            parameters(std::move(initParameters)) {}

        std::vector<const Expression*> parameters;
    };

    class VectorElementExpression final: public Expression
    {
    public:
        VectorElementExpression(const Type* resultType,
                                Qualifiers qualifiers,
                                Category category,
                                std::vector<uint8_t> initPositions) noexcept:
            Expression(Expression::Kind::VectorElement,
                       QualifiedType{resultType, qualifiers},
                       category),
            positions(std::move(initPositions)) {}

        std::vector<uint8_t> positions;
    };

    class MatrixInitializeExpression final: public Expression
    {
    public:
        MatrixInitializeExpression(const MatrixType* matrixType,
                                   std::vector<const Expression*> initParameters) noexcept:
            Expression(Expression::Kind::MatrixInitialize,
                       QualifiedType{matrixType, Qualifiers::Const},
                       Category::Rvalue),
            parameters(std::move(initParameters)) {}

        std::vector<const Expression*> parameters;
    };
}

#endif // EXPRESSIONS_HPP
