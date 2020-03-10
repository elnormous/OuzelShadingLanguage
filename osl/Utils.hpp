//
//  OSL
//

#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <stdexcept>
#include "Declarations.hpp"
#include "Expressions.hpp"
#include "Statements.hpp"
#include "Tokenizer.hpp"

namespace ouzel
{
    inline std::string toString(Construct::Kind kind)
    {
        switch (kind)
        {
            case Construct::Kind::Declaration: return "Declaration";
            case Construct::Kind::Statement: return "Statement";
            case Construct::Kind::Expression: return "Expression";
            case Construct::Kind::Attribute: return "Attribute";
        }

        throw std::runtime_error("Unknown construct kind");
    }

    inline std::string toString(Statement::Kind kind)
    {
        switch (kind)
        {
            case Statement::Kind::Empty: return "Empty";
            case Statement::Kind::Expression: return "Expression";
            case Statement::Kind::Declaration: return "Declaration";
            case Statement::Kind::Compound: return "Compound";
            case Statement::Kind::If: return "If";
            case Statement::Kind::For: return "For";
            case Statement::Kind::Switch: return "Switch";
            case Statement::Kind::Case: return "Case";
            case Statement::Kind::Default: return "Default";
            case Statement::Kind::While: return "While";
            case Statement::Kind::Do: return "Do";
            case Statement::Kind::Break: return "Break";
            case Statement::Kind::Continue: return "Continue";
            case Statement::Kind::Return: return "Return";
        }

        throw std::runtime_error("Unknown statement kind");
    }

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

        throw std::runtime_error("Unknown expression kind");
    }

    inline std::string toString(Declaration::Kind kind)
    {
        switch (kind)
        {
            case Declaration::Kind::Type: return "Type";
            case Declaration::Kind::Field: return "Field";
            case Declaration::Kind::Callable: return "Callable";
            case Declaration::Kind::Variable: return "Variable";
            case Declaration::Kind::Parameter: return "Parameter";
        }

        throw std::runtime_error("Unknown declaration kind");
    }

    inline std::string toString(Type::Kind kind)
    {
        switch (kind)
        {
            case Type::Kind::Void: return "Void";
            case Type::Kind::Array: return "Array";
            case Type::Kind::Scalar: return "Scalar";
            case Type::Kind::Struct: return "Struct";
            case Type::Kind::Vector: return "Vector";
            case Type::Kind::Matrix: return "Matrix";
        }

        throw std::runtime_error("Unknown type kind");
    }

    inline std::string toString(ScalarType::Kind kind)
    {
        switch (kind)
        {
            case ScalarType::Kind::Boolean: return "Boolean";
            case ScalarType::Kind::Integer: return "Integer";
            case ScalarType::Kind::FloatingPoint: return "FloatingPoint";
        }

        throw std::runtime_error("Unknown scalar type kind");
    }

    inline std::string toString(CallableDeclaration::Kind kind)
    {
        switch (kind)
        {
            case CallableDeclaration::Kind::Function: return "Function";
            case CallableDeclaration::Kind::Constructor: return "Constructor";
            case CallableDeclaration::Kind::Method: return "Method";
        }

        throw std::runtime_error("Unknown callable declaration kind");
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

        throw std::runtime_error("Unknown literal expression kind");
    }

    inline std::string toString(CastExpression::Kind kind)
    {
        switch (kind)
        {
            case CastExpression::Kind::CStyle: return "CStyle";
            case CastExpression::Kind::Functional: return "Functional";
            case CastExpression::Kind::Static: return "Static";
        }

        throw std::runtime_error("Unknown cast expression kind");
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

        throw std::runtime_error("Unknown unary operator expression kind");
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

        throw std::runtime_error("Unknown binary operator expression kind");
    }

    inline std::string toString(Attribute::Kind attributeKind)
    {
        switch (attributeKind)
        {
            case Attribute::Kind::Binormal: return "Binormal";
            case Attribute::Kind::BlendIndices: return "BlendIndices";
            case Attribute::Kind::BlendWeight: return "BlendWeight";
            case Attribute::Kind::Color: return "Color";
            case Attribute::Kind::Depth: return "Depth";
            case Attribute::Kind::Fog: return "Fog";
            case Attribute::Kind::Normal: return "Normal";
            case Attribute::Kind::Position: return "Position";
            case Attribute::Kind::PositionTransformed: return "PositionTransformed";
            case Attribute::Kind::PointSize: return "PointSize";
            case Attribute::Kind::Tangent: return "Tangent";
            case Attribute::Kind::TesselationFactor: return "TesselationFactor";
            case Attribute::Kind::TextureCoordinates: return "TextureCoordinates";
        }

        throw std::runtime_error("Unknown attribute kind");
    }

    inline std::string toString(InputModifier inputModifier)
    {
        switch (inputModifier)
        {
            case InputModifier::In: return "In";
            case InputModifier::Inout: return "Inout";
            case InputModifier::Out: return "Out";
        }

        throw std::runtime_error("Unknown input modifier");
    }

	inline std::string toString(Expression::Category category)
	{
		switch (category)
		{
			case Expression::Category::Lvalue: return "Lvalue";
			case Expression::Category::Rvalue: return "Rvalue";
		}

		throw std::runtime_error("Unknown category");
	}

    inline std::string getPrintableTypeName(const QualifiedType& qualifiedType)
    {
        std::string result;

        if ((qualifiedType.qualifiers & Type::Qualifiers::Volatile) == Type::Qualifiers::Volatile) result += "volatile ";
        if ((qualifiedType.qualifiers & Type::Qualifiers::Const) == Type::Qualifiers::Const) result += "const ";

        auto type = &qualifiedType.type;

        if (type->typeKind == Type::Kind::Array)
        {
            std::string arrayDimensions;
            while (type->typeKind == Type::Kind::Array)
            {
                auto arrayType = static_cast<const ArrayType*>(type);
                arrayDimensions += "[" + std::to_string(arrayType->size) + "]";

                type = &arrayType->elementType.type;
            }

            result += type->name + arrayDimensions;
        }
        else
            result += type->name;

        return result;
    }

    inline void dumpConstruct(const Construct& construct, const std::uint32_t level = 0);

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
                        std::cout << ", name: " << arrayType.name << ", element type: " << getPrintableTypeName(arrayType.elementType) << ", size" << arrayType.size;
                        break;
                    }

                    case Type::Kind::Struct:
                    {
                        auto& structType = static_cast<const StructType&>(type);
                        std::cout << ", name: " << structType.name;

                        std::cout << '\n';

                        for (auto memberDeclaration : structType.memberDeclarations)
                            dumpConstruct(*memberDeclaration, level + 1);

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

                std::cout << ", name: " << fieldDeclaration.name << ", type: " << getPrintableTypeName(fieldDeclaration.qualifiedType) << '\n';

                for (const auto attribute : fieldDeclaration.attributes)
                    dumpConstruct(*attribute, level + 1);

                break;
            }

            case Declaration::Kind::Callable:
            {
                auto& callableDeclaration = static_cast<const CallableDeclaration&>(declaration);

                std::cout << ", callable kind: " << toString(callableDeclaration.callableDeclarationKind) << ", name: " << callableDeclaration.name;

                if (callableDeclaration.callableDeclarationKind == CallableDeclaration::Kind::Function)
                {
                    auto& functionDeclaration = static_cast<const FunctionDeclaration&>(callableDeclaration);

                    std::cout << ", result type: " << getPrintableTypeName(functionDeclaration.resultType);
                    if (functionDeclaration.isBuiltin) std::cout << " builtin";
                }
                else if (callableDeclaration.callableDeclarationKind == CallableDeclaration::Kind::Method)
                {
                    auto& methodDeclaration = static_cast<const MethodDeclaration&>(callableDeclaration);

                    std::cout << ", result type: " << getPrintableTypeName(methodDeclaration.resultType);
                }

                if (callableDeclaration.previousDeclaration)
                    std::cout << ", previous declaration: " << callableDeclaration.previousDeclaration;

                if (callableDeclaration.definition)
                    std::cout << ", definition: " << callableDeclaration.definition;

                std::cout << '\n';

                for (auto parameter : callableDeclaration.parameterDeclarations)
                    dumpConstruct(*parameter, level + 1);

                for (auto attribute : callableDeclaration.attributes)
                    dumpConstruct(*attribute, level + 1);

                if (callableDeclaration.body)
                    dumpConstruct(*callableDeclaration.body, level + 1);

                break;
            }

            case Declaration::Kind::Variable:
            {
                auto& variableDeclaration = static_cast<const VariableDeclaration&>(declaration);
                std::cout << ", name: " << variableDeclaration.name << ", type: " << getPrintableTypeName(variableDeclaration.qualifiedType) << '\n';

                for (const auto attribute : variableDeclaration.attributes)
                    dumpConstruct(*attribute, level + 1);

                if (variableDeclaration.initialization)
                    dumpConstruct(*variableDeclaration.initialization, level + 1);

                break;
            }

            case Declaration::Kind::Parameter:
            {
                auto& parameterDeclaration = static_cast<const ParameterDeclaration&>(declaration);
                std::cout << ", name: " << parameterDeclaration.name <<
                    ", type: " << getPrintableTypeName(parameterDeclaration.qualifiedType) <<
                    ", input modifier: " << toString(parameterDeclaration.inputModifier) << '\n';

                for (const auto attribute : parameterDeclaration.attributes)
                    dumpConstruct(*attribute, level + 1);

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

                for (const auto subSstatement : compoundStatement.statements)
                    dumpConstruct(*subSstatement, level + 1);

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

                for (const auto argument : callExpression.arguments)
                    dumpConstruct(*argument, level + 1);

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

                for (const auto parameter : temporaryObjectExpression.parameters)
                    dumpConstruct(*parameter, level + 1);

                break;
            }

            case Expression::Kind::InitializerList:
            {
                auto& initializerListExpression = static_cast<const InitializerListExpression&>(expression);

                std::cout << '\n';

                for (const auto subExpression : initializerListExpression.expressions)
                    dumpConstruct(*subExpression, level + 1);

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

                for (const auto parameter : vectorInitializeExpression.parameters)
                    dumpConstruct(*parameter, level + 1);

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

                for (const auto parameter : matrixInitializeExpression.parameters)
                    dumpConstruct(*parameter, level + 1);

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

    inline std::string toString(Token::Type type)
    {
        switch (type)
        {
            case Token::Type::None: return "None";
            case Token::Type::IntLiteral: return "IntLiteral";
            case Token::Type::FloatLiteral: return "FloatLiteral";
            case Token::Type::DoubleLiteral: return "DoubleLiteral";
            case Token::Type::CharLiteral: return "CharLiteral";
            case Token::Type::StringLiteral: return "StringLiteral";
            case Token::Type::And: return "And";
            case Token::Type::Asm: return "Asm";
            case Token::Type::Auto: return "Auto";
            case Token::Type::Bool: return "Bool";
            case Token::Type::Break: return "Break";
            case Token::Type::Case: return "Case";
            case Token::Type::Catch: return "Catch";
            case Token::Type::Char: return "Char";
            case Token::Type::Class: return "Class";
            case Token::Type::Const: return "Const";
            case Token::Type::ConstCast: return "ConstCast";
            case Token::Type::Continue: return "Continue";
            case Token::Type::Default: return "Default";
            case Token::Type::Delete: return "Delete";
            case Token::Type::Do: return "Do";
            case Token::Type::Double: return "Double";
            case Token::Type::DynamicCast: return "DynamicCast";
            case Token::Type::Else: return "Else";
            case Token::Type::Enum: return "Enum";
            case Token::Type::Explicit: return "Explicit";
            case Token::Type::Export: return "Export";
            case Token::Type::Extern: return "Extern";
            case Token::Type::False: return "False";
            case Token::Type::Float: return "Float";
            case Token::Type::For: return "For";
            case Token::Type::Fragment: return "Fragment";
            case Token::Type::Friend: return "Friend";
            case Token::Type::Function: return "Function";
            case Token::Type::Goto: return "Goto";
            case Token::Type::If: return "If";
            case Token::Type::In: return "In";
            case Token::Type::Inline: return "Inline";
            case Token::Type::Inout: return "Inout";
            case Token::Type::Int: return "Int";
            case Token::Type::Long: return "Long";
            case Token::Type::Mutable: return "Mutable";
            case Token::Type::Namespace: return "Namespace";
            case Token::Type::New: return "New";
            case Token::Type::Noexcept: return "Noexcept";
            case Token::Type::Not: return "Not";
            case Token::Type::NotEq: return "NotEq";
            case Token::Type::Nullptr: return "Nullptr";
            case Token::Type::Operator: return "Operator";
            case Token::Type::Or: return "Or";
            case Token::Type::Out: return "Out";
            case Token::Type::Private: return "Private";
            case Token::Type::Protected: return "Protected";
            case Token::Type::Public: return "Public";
            case Token::Type::Register: return "Register";
            case Token::Type::ReinterpretCast: return "ReinterpretCast";
            case Token::Type::Return: return "Return";
            case Token::Type::Short: return "Short";
            case Token::Type::Signed: return "Signed";
            case Token::Type::Sizeof: return "Sizeof";
            case Token::Type::Static: return "Static";
            case Token::Type::StaticCast: return "StaticCast";
            case Token::Type::Struct: return "Struct";
            case Token::Type::Switch: return "Switch";
            case Token::Type::Template: return "Template";
            case Token::Type::This: return "This";
            case Token::Type::Throw: return "Throw";
            case Token::Type::True: return "True";
            case Token::Type::Try: return "Try";
            case Token::Type::Typedef: return "Typedef";
            case Token::Type::Typeid: return "Typeid";
            case Token::Type::Typename: return "Typename";
            case Token::Type::Union: return "Union";
            case Token::Type::Unsigned: return "Unsigned";
            case Token::Type::Using: return "Using";
            case Token::Type::Var: return "Var";
            case Token::Type::Varying: return "Varying";
            case Token::Type::Vertex: return "Vertex";
            case Token::Type::Virtual: return "Virtual";
            case Token::Type::Void: return "Void";
            case Token::Type::Volatile: return "Volatile";
            case Token::Type::WcharT: return "WcharT";
            case Token::Type::While: return "While";
            case Token::Type::LeftParenthesis: return "LeftParenthesis";
            case Token::Type::RightParenthesis: return "RightParenthesis";
            case Token::Type::LeftBrace: return "LeftBrace";
            case Token::Type::RightBrace: return "RightBrace";
            case Token::Type::LeftBracket: return "LeftBracket";
            case Token::Type::RightBracket: return "RightBracket";
            case Token::Type::Comma: return "Comma";
            case Token::Type::Semicolon: return "Semicolon";
            case Token::Type::Colon: return "Colon";
            case Token::Type::Plus: return "Plus";
            case Token::Type::Minus: return "Minus";
            case Token::Type::Multiply: return "Multiply";
            case Token::Type::Divide: return "Divide";
            case Token::Type::Modulo: return "Modulo";
            case Token::Type::Increment: return "Increment";
            case Token::Type::Decrement: return "Decrement";
            case Token::Type::Assignment: return "Assignment";
            case Token::Type::PlusAssignment: return "PlusAssignment";
            case Token::Type::MinusAssignment: return "MinusAssignment";
            case Token::Type::MultiplyAssignment: return "MultiplyAssignment";
            case Token::Type::DivideAssignment: return "DivideAssignment";
            case Token::Type::ModuloAssignment: return "ModuloAssignment";
            case Token::Type::BitwiseAndAssignment: return "BitwiseAndAssignment";
            case Token::Type::BitwiseOrAssignment: return "BitwiseOrAssignment";
            case Token::Type::BitwiseNotAssignment: return "BitwiseNotAssignment";
            case Token::Type::BitwiseXorAssignment: return "BitwiseXorAssignment";
            case Token::Type::ShiftRightAssignment: return "ShiftRightAssignment";
            case Token::Type::ShiftLeftAssignment: return "ShiftLeftAssignment";
            case Token::Type::BitwiseAnd: return "BitwiseAnd";
            case Token::Type::BitwiseOr: return "BitwiseOr";
            case Token::Type::BitwiseNot: return "BitwiseNot";
            case Token::Type::BitwiseXor: return "BitwiseXor";
            case Token::Type::ShiftRight: return "ShiftRight";
            case Token::Type::ShiftLeft: return "ShiftLeft";
            case Token::Type::Equal: return "Equal";
            case Token::Type::LessThan: return "LessThan";
            case Token::Type::GreaterThan: return "GreaterThan";
            case Token::Type::LessThanEqual: return "LessThanEqual";
            case Token::Type::GreaterThanEqual: return "GreaterThanEqual";
            case Token::Type::Conditional: return "Conditional";
            case Token::Type::Dot: return "Dot";
            case Token::Type::Arrow: return "Arrow";
            case Token::Type::Ellipsis: return "Ellipsis";
            case Token::Type::Identifier: return "Identifier";
        }

        throw std::runtime_error("Unknown token type");
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
