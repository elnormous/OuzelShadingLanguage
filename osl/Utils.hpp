//
//  OSL
//

#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <stdexcept>
#include <string>
#include "Declarations.hpp"
#include "Expressions.hpp"
#include "Statements.hpp"
#include "Tokenizer.hpp"

namespace ouzel
{
    void dumpConstruct(const Construct& construct, const std::uint32_t level = 0);

    inline void dumpDeclaration(const Declaration& declaration, const std::uint32_t level = 0)
    {
        std::cout << " " << toString(declaration.declarationKind);

        switch (declaration.declarationKind)
        {
            case Declaration::Kind::Type:
            {
                auto& typeDeclaration = static_cast<const TypeDeclaration&>(declaration);
                auto& type = typeDeclaration.type;

                std::cout << " " << toString(type.typeKind);

                switch (type.typeKind)
                {
                    case Type::Kind::Void: // void types can not be declared in code
                    {
                        std::cout << ", name: " << type.name;
                        break;
                    }

                    case Type::Kind::Array: // array types can not be declared in code
                    {
                        auto& arrayType = static_cast<const ArrayType&>(type);
                        std::cout << ", name: " << arrayType.name << ", element type: " << getPrintableName(arrayType.elementType) << ", size" << arrayType.size;
                        break;
                    }

                    case Type::Kind::Struct:
                    {
                        auto& structType = static_cast<const StructType&>(type);
                        std::cout << ", name: " << structType.name;

                        std::cout << '\n';

                        for (auto& memberDeclaration : structType.memberDeclarations)
                            dumpConstruct(memberDeclaration, level + 1);

                        break;
                    }

                    case Type::Kind::Scalar: // scalar types can not be declared in code
                    {
                        auto& scalarType = static_cast<const ScalarType&>(type);
                        std::cout << ", name: " << scalarType.name << ", scalar type kind: " << toString(scalarType.scalarTypeKind);
                        break;
                    }

                    case Type::Kind::Vector: // vector types can not be declared in code
                    {
                        auto& vectorType = static_cast<const VectorType&>(type);
                        std::cout << ", name: " << vectorType.name << ", component type: " << vectorType.componentType.name << ", components: " << vectorType.componentCount;
                        break;
                    }

                    case Type::Kind::Matrix: // matrix types can not be declared in code
                    {
                        auto& matrixType = static_cast<const MatrixType&>(type);
                        std::cout << ", name: " << matrixType.name << ", row type: " << matrixType.rowType.name << ", rows: " << matrixType.rowCount;
                        break;
                    }
                }
                break;
            }

            case Declaration::Kind::Field:
            {
                auto& fieldDeclaration = static_cast<const FieldDeclaration&>(declaration);

                std::cout << ", name: " << fieldDeclaration.name << ", type: " << getPrintableName(fieldDeclaration.qualifiedType) << '\n';

                for (auto& attribute : fieldDeclaration.attributes)
                    dumpConstruct(attribute, level + 1);

                break;
            }

            case Declaration::Kind::Callable:
            {
                auto& callableDeclaration = static_cast<const CallableDeclaration&>(declaration);

                std::cout << ", callable kind: " << toString(callableDeclaration.callableDeclarationKind) << ", name: " << callableDeclaration.name;

                if (callableDeclaration.callableDeclarationKind == CallableDeclaration::Kind::Function)
                {
                    auto& functionDeclaration = static_cast<const FunctionDeclaration&>(callableDeclaration);

                    std::cout << ", result type: " << getPrintableName(functionDeclaration.resultType);
                    if (functionDeclaration.isBuiltin) std::cout << " builtin";
                }
                else if (callableDeclaration.callableDeclarationKind == CallableDeclaration::Kind::Method)
                {
                    auto& methodDeclaration = static_cast<const MethodDeclaration&>(callableDeclaration);

                    std::cout << ", result type: " << getPrintableName(methodDeclaration.resultType);
                }

                if (callableDeclaration.previousDeclaration)
                    std::cout << ", previous declaration: " << callableDeclaration.previousDeclaration;

                if (callableDeclaration.definition)
                    std::cout << ", definition: " << callableDeclaration.definition;

                std::cout << '\n';

                for (auto parameter : callableDeclaration.parameterDeclarations)
                    dumpConstruct(*parameter, level + 1);

                for (auto& attribute : callableDeclaration.attributes)
                    dumpConstruct(attribute, level + 1);

                if (callableDeclaration.body)
                    dumpConstruct(*callableDeclaration.body, level + 1);

                break;
            }

            case Declaration::Kind::Variable:
            {
                auto& variableDeclaration = static_cast<const VariableDeclaration&>(declaration);
                std::cout << ", name: " << variableDeclaration.name << ", type: " << getPrintableName(variableDeclaration.qualifiedType) << '\n';

                for (auto& attribute : variableDeclaration.attributes)
                    dumpConstruct(attribute, level + 1);

                if (variableDeclaration.initialization)
                    dumpConstruct(*variableDeclaration.initialization, level + 1);

                break;
            }

            case Declaration::Kind::Parameter:
            {
                auto& parameterDeclaration = static_cast<const ParameterDeclaration&>(declaration);
                std::cout << ", name: " << parameterDeclaration.name <<
                    ", type: " << getPrintableName(parameterDeclaration.qualifiedType) <<
                    ", input modifier: " << toString(parameterDeclaration.inputModifier) << '\n';

                for (auto& attribute : parameterDeclaration.attributes)
                    dumpConstruct(attribute, level + 1);

                break;
            }

            default:
                break;
        }
    }

    inline void dumpStatement(const Statement& statement, const std::uint32_t level = 0)
    {
        std::cout << " " << toString(statement.statementKind);

        switch (statement.statementKind)
        {
            case Statement::Kind::Empty:
            {
                std::cout << '\n';
                break;
            }

            case Statement::Kind::Expression:
            {
                auto& expressionStatement = static_cast<const ExpressionStatement&>(statement);

                std::cout << '\n';

                dumpConstruct(expressionStatement.expression, level + 1);
                break;
            }

            case Statement::Kind::Declaration:
            {
                auto& declarationStatement = static_cast<const DeclarationStatement&>(statement);

                std::cout << '\n';

                dumpConstruct(declarationStatement.declaration, level + 1);
                break;
            }

            case Statement::Kind::Compound:
            {
                auto& compoundStatement = static_cast<const CompoundStatement&>(statement);

                std::cout << '\n';

                for (auto& subSstatement : compoundStatement.statements)
                    dumpConstruct(subSstatement, level + 1);

                break;
            }

            case Statement::Kind::If:
            {
                auto& ifStatement = static_cast<const IfStatement&>(statement);

                std::cout << '\n';

                dumpConstruct(ifStatement.condition, level + 1);
                dumpConstruct(ifStatement.body, level + 1);
                if (ifStatement.elseBody) dumpConstruct(*ifStatement.elseBody, level + 1);
                break;
            }

            case Statement::Kind::For:
            {
                auto& forStatement = static_cast<const ForStatement&>(statement);

                std::cout << '\n';

                if (forStatement.initialization)
                    dumpConstruct(*forStatement.initialization, level + 1);
                if (forStatement.condition)
                    dumpConstruct(*forStatement.condition, level + 1);
                if (forStatement.increment)
                    dumpConstruct(*forStatement.increment, level + 1);
                dumpConstruct(forStatement.body, level + 1);
                break;
            }

            case Statement::Kind::Switch:
            {
                auto& switchStatement = static_cast<const SwitchStatement&>(statement);

                std::cout << '\n';

                dumpConstruct(switchStatement.condition, level + 1);
                dumpConstruct(switchStatement.body, level + 1);
                break;
            }

            case Statement::Kind::Case:
            {
                auto& caseStatement = static_cast<const CaseStatement&>(statement);

                std::cout << '\n';

                dumpConstruct(caseStatement.condition, level + 1);
                dumpConstruct(caseStatement.body, level + 1);
                break;
            }

            case Statement::Kind::Default:
            {
                auto& defaultStatement = static_cast<const DefaultStatement&>(statement);

                std::cout << '\n';

                dumpConstruct(defaultStatement.body, level + 1);
                break;
            }

            case Statement::Kind::While:
            {
                auto& whileStatement = static_cast<const WhileStatement&>(statement);

                std::cout << '\n';

                dumpConstruct(whileStatement.condition, level + 1);
                dumpConstruct(whileStatement.body, level + 1);
                break;
            }

            case Statement::Kind::Do:
            {
                auto& doStatement = static_cast<const DoStatement&>(statement);

                std::cout << '\n';

                dumpConstruct(doStatement.body, level + 1);
                dumpConstruct(doStatement.condition, level + 1);
                break;
            }

            case Statement::Kind::Break:
            {
                std::cout << '\n';
                break;
            }

            case Statement::Kind::Continue:
            {
                std::cout << '\n';
                break;
            }

            case Statement::Kind::Return:
            {
                auto& returnStatement = static_cast<const ReturnStatement&>(statement);

                std::cout << '\n';

                if (returnStatement.result)
                    dumpConstruct(*returnStatement.result, level + 1);
                break;
            }

            default:
                break;
        }
    }

    inline void dumpExpression(const Expression& expression, const std::uint32_t level = 0)
    {
        std::cout << " " << toString(expression.expressionKind) << ", category: " << toString(expression.category);

        switch (expression.expressionKind)
        {
            case Expression::Kind::Call:
            {
                auto& callExpression = static_cast<const CallExpression&>(expression);

                std::cout << '\n';

                dumpConstruct(callExpression.declarationReference, level + 1);

                for (auto& argument : callExpression.arguments)
                    dumpConstruct(argument, level + 1);

                break;
            }

            case Expression::Kind::Literal:
            {
                auto& literalExpression = static_cast<const LiteralExpression&>(expression);

                std::cout << ", literal kind: " << toString(literalExpression.literalKind) << ", value: ";

                switch (literalExpression.literalKind)
                {
                    case LiteralExpression::Kind::Boolean:
                    {
                        auto& booleanLiteralExpression = static_cast<const BooleanLiteralExpression&>(literalExpression);
                        std::cout << (booleanLiteralExpression.value ? "true" : "false");
                        break;
                    }
                    case LiteralExpression::Kind::Integer:
                    {
                        auto& integerLiteralExpression = static_cast<const IntegerLiteralExpression&>(literalExpression);
                        std::cout << integerLiteralExpression.value;
                        break;
                    }
                    case LiteralExpression::Kind::FloatingPoint:
                    {
                        auto& floatingPointLiteralExpression = static_cast<const FloatingPointLiteralExpression&>(literalExpression);
                        std::cout << floatingPointLiteralExpression.value;
                        break;
                    }
                    case LiteralExpression::Kind::String:
                    {
                        auto& stringLiteralExpression = static_cast<const StringLiteralExpression&>(literalExpression);
                        std::cout << stringLiteralExpression.value;
                        break;
                    }
                }

                std::cout << '\n';
                break;
            }

            case Expression::Kind::DeclarationReference:
            {
                auto& declarationReferenceExpression = static_cast<const DeclarationReferenceExpression&>(expression);

                std::cout << ", name: " << declarationReferenceExpression.declaration.name << ", declaration: " << &declarationReferenceExpression.declaration;

                std::cout << '\n';
                break;
            }

            case Expression::Kind::Paren:
            {
                auto& parenExpression = static_cast<const ParenExpression&>(expression);

                std::cout << '\n';

                dumpConstruct(parenExpression.expression, level + 1);
                break;
            }

            case Expression::Kind::Member:
            {
                auto& memberExpression = static_cast<const MemberExpression&>(expression);

                std::cout << ", field: " << memberExpression.fieldDeclaration.name << '\n';

                dumpConstruct(memberExpression.expression, level + 1);
                break;
            }

            case Expression::Kind::ArraySubscript:
            {
                auto& arraySubscriptExpression = static_cast<const ArraySubscriptExpression&>(expression);

                std::cout << '\n';

                dumpConstruct(arraySubscriptExpression.expression, level + 1);
                dumpConstruct(arraySubscriptExpression.subscript, level + 1);
                break;
            }

            case Expression::Kind::UnaryOperator:
            {
                auto& unaryOperatorExpression = static_cast<const UnaryOperatorExpression&>(expression);

                std::cout <<", operator: " << toString(unaryOperatorExpression.operatorKind) << '\n';

                dumpConstruct(unaryOperatorExpression.expression, level + 1);
                break;
            }

            case Expression::Kind::BinaryOperator:
            {
                auto& binaryOperatorExpression = static_cast<const BinaryOperatorExpression&>(expression);

                std::cout << ", operator: " << toString(binaryOperatorExpression.operatorKind) << '\n';

                dumpConstruct(binaryOperatorExpression.leftExpression, level + 1);
                dumpConstruct(binaryOperatorExpression.rightExpression, level + 1);
                break;
            }

            case Expression::Kind::TernaryOperator:
            {
                auto& ternaryOperatorExpression = static_cast<const TernaryOperatorExpression&>(expression);

                std::cout << '\n';

                dumpConstruct(ternaryOperatorExpression.condition, level + 1);
                dumpConstruct(ternaryOperatorExpression.leftExpression, level + 1);
                dumpConstruct(ternaryOperatorExpression.rightExpression, level + 1);
                break;
            }

            case Expression::Kind::TemporaryObject:
            {
                auto& temporaryObjectExpression = static_cast<const TemporaryObjectExpression&>(expression);

                std::cout << " " << temporaryObjectExpression.qualifiedType.type.name << '\n';

                for (auto& parameter : temporaryObjectExpression.parameters)
                    dumpConstruct(parameter, level + 1);

                break;
            }

            case Expression::Kind::InitializerList:
            {
                auto& initializerListExpression = static_cast<const InitializerListExpression&>(expression);

                std::cout << '\n';

                for (auto& subExpression : initializerListExpression.expressions)
                    dumpConstruct(subExpression, level + 1);

                break;
            }

            case Expression::Kind::Cast:
            {
                auto& castExpression = static_cast<const CastExpression&>(expression);

                std::cout << ", cast kind: " << toString(castExpression.castKind) <<
                    ", type: " << castExpression.qualifiedType.type.name << '\n';

                dumpConstruct(castExpression.expression, level + 1);

                break;
            }
            case Expression::Kind::VectorInitialize:
            {
                auto& vectorInitializeExpression = static_cast<const VectorInitializeExpression&>(expression);

                std::cout << '\n';

                for (auto& parameter : vectorInitializeExpression.parameters)
                    dumpConstruct(parameter, level + 1);

                break;
            }
            case Expression::Kind::VectorElement:
            {
                constexpr char components[4] = {'x', 'y', 'z', 'w'};

                std::cout << ", components: ";

                auto& vectorElementExpression = static_cast<const VectorElementExpression&>(expression);
                for (auto position : vectorElementExpression.positions)
                    std::cout << components[position];

                std::cout << '\n';
                break;
            }
            case Expression::Kind::MatrixInitialize:
            {
                auto& matrixInitializeExpression = static_cast<const MatrixInitializeExpression&>(expression);

                std::cout << '\n';

                for (auto& parameter : matrixInitializeExpression.parameters)
                    dumpConstruct(parameter, level + 1);

                break;
            }
        }
    }

    inline void dumpAttribute(const Attribute& attribute)
    {
        std::cout << " " << toString(attribute.attributeKind);

        switch (attribute.attributeKind)
        {
            case Attribute::Kind::Binormal:
            {
                auto& binormalAttribute = static_cast<const BinormalAttribute&>(attribute);
                std::cout << ", n: " << binormalAttribute.n;
                break;
            }
            case Attribute::Kind::BlendIndices:
            {
                auto& blendIndicesAttribute = static_cast<const BlendIndicesAttribute&>(attribute);
                std::cout << ", n: " << blendIndicesAttribute.n;
                break;
            }
            case Attribute::Kind::BlendWeight:
            {
                auto& blendWeightAttribute = static_cast<const BlendWeightAttribute&>(attribute);
                std::cout << ", n: " << blendWeightAttribute.n;
                break;
            }
            case Attribute::Kind::Color:
            {
                auto& colorAttribute = static_cast<const ColorAttribute&>(attribute);
                std::cout << ", n: " << colorAttribute.n;
                break;
            }
            case Attribute::Kind::Depth:
            {
                auto& depthAttribute = static_cast<const DepthAttribute&>(attribute);
                std::cout << ", n: " << depthAttribute.n;
                break;
            }
            case Attribute::Kind::Fog:
            {
                break;
            }
            case Attribute::Kind::Normal:
            {
                auto& normalAttribute = static_cast<const NormalAttribute&>(attribute);
                std::cout << ", n: " << normalAttribute.n;
                break;
            }
            case Attribute::Kind::Position:
            {
                auto& positionAttribute = static_cast<const PositionAttribute&>(attribute);
                std::cout << ", n: " << positionAttribute.n;
                break;
            }
            case Attribute::Kind::PositionTransformed:
            {
                break;
            }
            case Attribute::Kind::PointSize:
            {
                auto& pointSizeAttribute = static_cast<const PointSizeAttribute&>(attribute);
                std::cout << ", n: " << pointSizeAttribute.n;
                break;
            }
            case Attribute::Kind::Tangent:
            {
                auto& tangentAttribute = static_cast<const TangentAttribute&>(attribute);
                std::cout << ", n: " << tangentAttribute.n;
                break;
            }
            case Attribute::Kind::TesselationFactor:
            {
                auto& tesselationFactorAttribute = static_cast<const TesselationFactorAttribute&>(attribute);
                std::cout << ", n: " << tesselationFactorAttribute.n;
                break;
            }
            case Attribute::Kind::TextureCoordinates:
            {
                auto& textureCoordinatesAttribute = static_cast<const TextureCoordinatesAttribute&>(attribute);
                std::cout << ", n: " << textureCoordinatesAttribute.n;
                break;
            }
        };

        std::cout << '\n';
    }

    inline void dumpConstruct(const Construct& construct, const std::uint32_t level)
    {
        for (std::uint32_t i = 0; i < level; ++i)
            std::cout << "  ";

        std::cout << &construct;

        std::cout << " " << toString(construct.kind);

        switch (construct.kind)
        {
            case Construct::Kind::Declaration:
            {
                auto& declaration = static_cast<const Declaration&>(construct);
                dumpDeclaration(declaration, level);
                break;
            }

            case Construct::Kind::Statement:
            {
                auto& statement = static_cast<const Statement&>(construct);
                dumpStatement(statement, level);
                break;
            }

            case Construct::Kind::Expression:
            {
                auto& expression = static_cast<const Expression&>(construct);
                dumpExpression(expression, level);
                break;
            }
            case Construct::Kind::Attribute:
            {
                auto& attribute = static_cast<const Attribute&>(construct);
                dumpAttribute(attribute);
                break;
            }
        }
    }

    inline void dump(const std::vector<Token>& tokens)
    {
        for (const auto& token : tokens)
        {
            std::cout << "Token, type: " << toString(token.type) <<
                ", value: " << token.value <<
                ", line: " << token.line <<
                ", column: " << token.column << '\n';
        }
    }
}

#endif // UTILS_HPP
