//
//  OSL
//

#ifndef EXPRESSIONS_HPP
#define EXPRESSIONS_HPP

#include "Construct.hpp"
#include "Types.hpp"

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

        enum class Category: std::uint8_t
        {
            Lvalue,
            Rvalue
        };

        Expression(Kind initExpressionKind,
                   const QualifiedType& initQualifiedType,
                   Category initCategory) noexcept:
            Construct{Construct::Kind::Expression},
            expressionKind{initExpressionKind},
            qualifiedType{initQualifiedType},
            category{initCategory} {}

        const Kind expressionKind;
        const QualifiedType qualifiedType;
        const Category category = Category::Rvalue;
    };

    using ExpressionRef = std::reference_wrapper<const Expression>;

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
                          const Type& type) noexcept:
            Expression{
                Expression::Kind::Literal,
                QualifiedType{type, Type::Qualifiers::Const},
                Category::Rvalue
            },
            literalKind{initLiteralKind} {}

        const Kind literalKind;
    };

    class BooleanLiteralExpression final: public LiteralExpression
    {
    public:
        BooleanLiteralExpression(const Type& type,
                                 bool initValue) noexcept:
            LiteralExpression{LiteralExpression::Kind::Boolean, type},
            value{initValue} {}

        const bool value;
    };

    class IntegerLiteralExpression final: public LiteralExpression
    {
    public:
        IntegerLiteralExpression(const Type& type,
                                 std::int64_t initValue) noexcept:
            LiteralExpression{LiteralExpression::Kind::Integer, type},
            value{initValue} {}

        const std::int64_t value;
    };

    class FloatingPointLiteralExpression final: public LiteralExpression
    {
    public:
        FloatingPointLiteralExpression(const Type& type,
                                       double initValue) noexcept:
            LiteralExpression{LiteralExpression::Kind::FloatingPoint, type},
            value{initValue} {}

        const double value;
    };

    class StringLiteralExpression final: public LiteralExpression
    {
    public:
        StringLiteralExpression(const Type& type,
                                const std::string& initValue):
            LiteralExpression{LiteralExpression::Kind::String, type},
            value{initValue} {}

        const std::string value;
    };

    class DeclarationReferenceExpression final: public Expression
    {
    public:
        DeclarationReferenceExpression(const QualifiedType& qualifiedType,
                                       const Declaration& initDeclaration,
                                       Category category):
            Expression{
                Expression::Kind::DeclarationReference,
                qualifiedType,
                category
            },
            declaration{initDeclaration} {}

        const Declaration& declaration;
    };

    class CallExpression final: public Expression
    {
    public:
        CallExpression(const QualifiedType& qualifiedType,
                       Category category,
                       const DeclarationReferenceExpression& initDeclarationReference,
                       std::vector<ExpressionRef> initArguments):
            Expression{
                Expression::Kind::Call,
                qualifiedType,
                category
            },
            declarationReference{initDeclarationReference},
            arguments{std::move(initArguments)} {}

        const DeclarationReferenceExpression& declarationReference;
        const std::vector<ExpressionRef> arguments;
    };

    class ParenExpression final: public Expression
    {
    public:
        explicit ParenExpression(const Expression& initExpression) noexcept:
            Expression{
                Expression::Kind::Paren,
                initExpression.qualifiedType,
                initExpression.category
            },
            expression{initExpression} {}

        const Expression& expression;
    };

    class MemberExpression final: public Expression
    {
    public:
        MemberExpression(const Expression& initExpression,
                         const FieldDeclaration& initFieldDeclaration) noexcept:
            Expression{
                Expression::Kind::Member,
                initFieldDeclaration.qualifiedType,
                initExpression.category
            },
            expression{initExpression},
            fieldDeclaration{initFieldDeclaration} {}

        const Expression& expression;
        const FieldDeclaration& fieldDeclaration;
    };

    class ArraySubscriptExpression final: public Expression
    {
    public:
        ArraySubscriptExpression(const QualifiedType& elementType,
                                 const Expression& initExpression,
                                 const Expression& initSubscript) noexcept:
            Expression{
                Expression::Kind::ArraySubscript,
                elementType,
                initExpression.category
            },
            expression{initExpression},
            subscript{initSubscript} {}

        const Expression& expression;
        const Expression& subscript;
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
                                const Type& type,
                                Category category,
                                const Expression& initExpression) noexcept:
            Expression{
                Expression::Kind::UnaryOperator,
                QualifiedType{type},
                category
            },
            operatorKind{initOperatorKind},
            expression{initExpression}
        {}

        const Kind operatorKind;
        const Expression& expression;
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
                                 const Type& type,
                                 Category category,
                                 const Expression& initLeftExpression,
                                 const Expression& initRightExpression) noexcept:
            Expression{
                Expression::Kind::BinaryOperator,
                QualifiedType{type},
                category
            },
            operatorKind{initOperatorKind},
            leftExpression{initLeftExpression},
            rightExpression{initRightExpression}
        {}

        const Kind operatorKind;
        const Expression& leftExpression;
        const Expression& rightExpression;
    };

    class TernaryOperatorExpression final: public Expression
    {
    public:
        TernaryOperatorExpression(const Expression& initCondition,
                                  const Expression& initLeftExpression,
                                  const Expression& initRightExpression) noexcept:
            Expression{
                Expression::Kind::TernaryOperator,
                initLeftExpression.qualifiedType,
                (initLeftExpression.category == Expression::Category::Lvalue &&
                 initRightExpression.category == Expression::Category::Lvalue) ?
                Expression::Category::Lvalue : Expression::Category::Rvalue
            },
            condition{initCondition},
            leftExpression{initLeftExpression},
            rightExpression{initRightExpression} {}

        const Expression& condition;
        const Expression& leftExpression;
        const Expression& rightExpression;
    };

    class TemporaryObjectExpression final: public Expression
    {
    public:
        TemporaryObjectExpression(const Type& type,
                                  const ConstructorDeclaration& initConstructorDeclaration,
                                  std::vector<ExpressionRef> initParameters):
            Expression{
                Expression::Kind::TemporaryObject,
                QualifiedType{type, Type::Qualifiers::Const},
                Category::Rvalue
            },
            constructorDeclaration{initConstructorDeclaration},
            parameters{std::move(initParameters)} {}

        const ConstructorDeclaration& constructorDeclaration;
        const std::vector<ExpressionRef> parameters;
    };

    class InitializerListExpression final: public Expression
    {
    public:
        InitializerListExpression(const Type& type,
                                  std::vector<ExpressionRef> initExpressions):
            Expression{
                Expression::Kind::InitializerList,
                QualifiedType{type, Type::Qualifiers::Const},
                Category::Rvalue
            },
            expressions{std::move(initExpressions)} {}

        const std::vector<ExpressionRef> expressions;
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
                       const Type& type,
                       const Expression& initExpression) noexcept:
            Expression{
                Expression::Kind::Cast,
                QualifiedType{type, Type::Qualifiers::Const},
                Category::Rvalue
            },
            castKind{initCastKind},
            expression{initExpression} {}

        const Kind castKind;
        const Expression& expression;
    };

    class VectorInitializeExpression final: public Expression
    {
    public:
        VectorInitializeExpression(const VectorType& vectorType,
                                   std::vector<ExpressionRef> initParameters) noexcept:
            Expression{
                Expression::Kind::VectorInitialize,
                QualifiedType{vectorType, Type::Qualifiers::Const},
                Category::Rvalue
            },
            parameters{std::move(initParameters)} {}

        const std::vector<ExpressionRef> parameters;
    };

    class VectorElementExpression final: public Expression
    {
    public:
        VectorElementExpression(const Type& resultType,
                                Type::Qualifiers qualifiers,
                                Category category,
                                std::vector<std::uint8_t> initPositions) noexcept:
            Expression{
                Expression::Kind::VectorElement,
                QualifiedType{resultType, qualifiers},
                category
            },
            positions{std::move(initPositions)} {}

        const std::vector<std::uint8_t> positions;
    };

    class MatrixInitializeExpression final: public Expression
    {
    public:
        MatrixInitializeExpression(const MatrixType& matrixType,
                                   std::vector<ExpressionRef> initParameters) noexcept:
            Expression{
                Expression::Kind::MatrixInitialize,
                QualifiedType{matrixType, Type::Qualifiers::Const},
                Category::Rvalue
            },
            parameters{std::move(initParameters)} {}

        const std::vector<ExpressionRef> parameters;
    };

    inline std::string toString(Expression::Kind kind)
    {
        switch (kind)
        {
            case Expression::Kind::Call: return "Call";
            case Expression::Kind::Literal: return "Literal";
            case Expression::Kind::DeclarationReference: return "DeclarationReference";
            case Expression::Kind::Paren: return "Paren";
            case Expression::Kind::Member: return "Member";
            case Expression::Kind::ArraySubscript: return "ArraySubscript";
            case Expression::Kind::UnaryOperator: return "UnaryOperator";
            case Expression::Kind::BinaryOperator: return "BinaryOperator";
            case Expression::Kind::TernaryOperator: return "TernaryOperator";
            case Expression::Kind::TemporaryObject: return "TemporaryObject";
            case Expression::Kind::InitializerList: return "InitializerList";
            case Expression::Kind::Cast: return "Cast";
            case Expression::Kind::VectorInitialize: return "VectorInitialize";
            case Expression::Kind::VectorElement: return "VectorElement";
            case Expression::Kind::MatrixInitialize: return "MatrixInitialize";
        }

        throw std::runtime_error{"Unknown expression kind"};
    }

    inline std::string toString(LiteralExpression::Kind kind)
    {
        switch (kind)
        {
            case LiteralExpression::Kind::Boolean: return "Boolean";
            case LiteralExpression::Kind::Integer: return "Integer";
            case LiteralExpression::Kind::FloatingPoint: return "FloatingPoint";
            case LiteralExpression::Kind::String: return "String";
        }

        throw std::runtime_error{"Unknown literal expression kind"};
    }

    inline std::string toString(CastExpression::Kind kind)
    {
        switch (kind)
        {
            case CastExpression::Kind::CStyle: return "CStyle";
            case CastExpression::Kind::Functional: return "Functional";
            case CastExpression::Kind::Static: return "Static";
        }

        throw std::runtime_error{"Unknown cast expression kind"};
    }

    inline std::string toString(UnaryOperatorExpression::Kind operatorKind)
    {
        switch (operatorKind)
        {
            case UnaryOperatorExpression::Kind::Negation: return "Negation";
            case UnaryOperatorExpression::Kind::Positive: return "Positive";
            case UnaryOperatorExpression::Kind::Negative: return "Negative";
            case UnaryOperatorExpression::Kind::PrefixIncrement: return "PrefixIncrement";
            case UnaryOperatorExpression::Kind::PrefixDecrement: return "PrefixDecrement";
            case UnaryOperatorExpression::Kind::PostfixIncrement: return "PostfixIncrement";
            case UnaryOperatorExpression::Kind::PostfixDecrement: return "PostfixDecrement";
        }

        throw std::runtime_error{"Unknown unary operator expression kind"};
    }

    inline std::string toString(BinaryOperatorExpression::Kind operatorKind)
    {
        switch (operatorKind)
        {
            case BinaryOperatorExpression::Kind::Addition: return "Addition";
            case BinaryOperatorExpression::Kind::Subtraction: return "Subtraction";
            case BinaryOperatorExpression::Kind::Multiplication: return "Multiplication";
            case BinaryOperatorExpression::Kind::Division: return "Division";
            case BinaryOperatorExpression::Kind::AdditionAssignment: return "AdditionAssignment";
            case BinaryOperatorExpression::Kind::SubtractAssignment: return "SubtractAssignment";
            case BinaryOperatorExpression::Kind::MultiplicationAssignment: return "MultiplicationAssignment";
            case BinaryOperatorExpression::Kind::DivisionAssignment: return "DivisionAssignment";
            case BinaryOperatorExpression::Kind::LessThan: return "LessThan";
            case BinaryOperatorExpression::Kind::LessThanEqual: return "LessThanEqual";
            case BinaryOperatorExpression::Kind::GreaterThan: return "GreaterThan";
            case BinaryOperatorExpression::Kind::GraterThanEqual: return "GraterThanEqual";
            case BinaryOperatorExpression::Kind::Equality: return "Equality";
            case BinaryOperatorExpression::Kind::Inequality: return "Inequality";
            case BinaryOperatorExpression::Kind::Assignment: return "Assignment";
            case BinaryOperatorExpression::Kind::Or: return "Or";
            case BinaryOperatorExpression::Kind::And: return "And";
            case BinaryOperatorExpression::Kind::Comma: return "Comma";
            case BinaryOperatorExpression::Kind::Subscript: return "Subscript";
        }

        throw std::runtime_error{"Unknown binary operator expression kind"};
    }

    inline std::string toString(Expression::Category category)
    {
        switch (category)
        {
            case Expression::Category::Lvalue: return "Lvalue";
            case Expression::Category::Rvalue: return "Rvalue";
        }

        throw std::runtime_error{"Unknown category"};
    }
}

#endif // EXPRESSIONS_HPP
