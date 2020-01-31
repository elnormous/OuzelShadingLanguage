//
//  OSL
//

#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include "Declarations.hpp"
#include "Expressions.hpp"
#include "Statements.hpp"

namespace
{
    std::string toString(Construct::Kind kind)
    {
        switch (kind)
        {
            case Construct::Kind::Declaration: return "Declaration";
            case Construct::Kind::Statement: return "Statement";
            case Construct::Kind::Expression: return "Expression";
            default: return "Unknown";
        }
    }

    std::string toString(Statement::Kind kind)
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
            default: return "Unknown";
        }
    }

    std::string toString(Expression::Kind kind)
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
            case Expression::Kind::Sizeof: return "Sizeof";
            case Expression::Kind::VectorElement: return "VectorElement";
            case Expression::Kind::MatrixElement: return "MatrixElement";
            default: return "Unknown";
        }
    }

    std::string toString(Declaration::Kind kind)
    {
        switch (kind)
        {
            case Declaration::Kind::Empty: return "Empty";
            case Declaration::Kind::Type: return "Type";
            case Declaration::Kind::Field: return "Field";
            case Declaration::Kind::Callable: return "Callable";
            case Declaration::Kind::Variable: return "Variable";
            case Declaration::Kind::Parameter: return "Parameter";
            default: return "Unknown";
        }
    }

    std::string toString(Type::Kind kind)
    {
        switch (kind)
        {
            case Type::Kind::Array: return "Array";
            case Type::Kind::Scalar: return "Scalar";
            case Type::Kind::Struct: return "Struct";
            case Type::Kind::Vector: return "Vector";
                //case TypeDeclaration::Kind::TypeDefinition: return "TypeDefinition";
            default: return "Unknown";
        }
    }

    std::string toString(ScalarType::Kind kind)
    {
        switch (kind)
        {
            case ScalarType::Kind::Boolean: return "Boolean";
            case ScalarType::Kind::Integer: return "Integer";
            case ScalarType::Kind::FloatingPoint: return "FloatingPoint";
            default: return "Unknown";
        }
    }

    std::string toString(CallableDeclaration::Kind kind)
    {
        switch (kind)
        {
            case CallableDeclaration::Kind::Function: return "Function";
            case CallableDeclaration::Kind::Constructor: return "Constructor";
            case CallableDeclaration::Kind::Method: return "Method";
            default: return "Unknown";
        }
    }

    std::string toString(LiteralExpression::Kind kind)
    {
        switch (kind)
        {
            case LiteralExpression::Kind::Boolean: return "Boolean";
            case LiteralExpression::Kind::Integer: return "Integer";
            case LiteralExpression::Kind::FloatingPoint: return "FloatingPoint";
            case LiteralExpression::Kind::String: return "String";
            default: return "Unknown";
        }
    }

    std::string toString(CastExpression::Kind kind)
    {
        switch (kind)
        {
            case CastExpression::Kind::Implicit: return "Implicit";
            case CastExpression::Kind::CStyle: return "CStyle";
            case CastExpression::Kind::Functional: return "Functional";
            case CastExpression::Kind::Dynamic: return "Dynamic";
            case CastExpression::Kind::Reinterpet: return "Reinterpet";
            case CastExpression::Kind::Static: return "Static";
            default: return "Unknown";
        }
    }

    std::string toString(UnaryOperatorExpression::Kind operatorKind)
    {
        switch (operatorKind)
        {
            case UnaryOperatorExpression::Kind::Negation: return "Negation";
            case UnaryOperatorExpression::Kind::Positive: return "Positive";
            case UnaryOperatorExpression::Kind::Negative: return "Negative";
            default: return "Unknown";
        }
    }

    std::string toString(BinaryOperatorExpression::Kind operatorKind)
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
            default: return "Unknown";
        }
    }

    std::string toString(OverloadedOperator op)
    {
        switch (op)
        {
            case OverloadedOperator::Negation: return "Negation";
            case OverloadedOperator::Positive: return "Positive";
            case OverloadedOperator::Negative: return "Negative";
            case OverloadedOperator::Addition: return "Addition";
            case OverloadedOperator::Subtraction: return "Subtraction";
            case OverloadedOperator::Multiplication: return "Multiplication";
            case OverloadedOperator::Division: return "Division";
            case OverloadedOperator::AdditionAssignment: return "AdditionAssignment";
            case OverloadedOperator::SubtractAssignment: return "SubtractAssignment";
            case OverloadedOperator::MultiplicationAssignment: return "MultiplicationAssignment";
            case OverloadedOperator::DivisionAssignment: return "DivisionAssignment";
            case OverloadedOperator::LessThan: return "LessThan";
            case OverloadedOperator::LessThanEqual: return "LessThanEqual";
            case OverloadedOperator::GreaterThan: return "GreaterThan";
            case OverloadedOperator::GraterThanEqual: return "GraterThanEqual";
            case OverloadedOperator::Equality: return "Equality";
            case OverloadedOperator::Inequality: return "Inequality";
            case OverloadedOperator::Assignment: return "Assignment";
            case OverloadedOperator::Or: return "Or";
            case OverloadedOperator::And: return "And";
            case OverloadedOperator::Comma: return "Comma";
            case OverloadedOperator::Subscript: return "Subscript";
            default: return "Unknown";
        }
    }

    std::string getPrintableTypeName(const QualifiedType& qualifiedType)
    {
        std::string result;

        if ((qualifiedType.qualifiers & Qualifiers::Volatile) == Qualifiers::Volatile) result += "volatile ";
        if ((qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const) result += "const ";

        if (!qualifiedType.type)
        {
            result += "void";
        }
        else
        {
            auto type = qualifiedType.type;

            if (type->getTypeKind() == Type::Kind::Array)
            {
                std::string arrayDimensions;
                while (type->getTypeKind() == Type::Kind::Array)
                {
                    const ArrayType* arrayType = static_cast<const ArrayType*>(type);
                    arrayDimensions += "[" + std::to_string(arrayType->size) + "]";

                    type = arrayType->elementType.type;
                }

                result += (type ? type->name : "void") + arrayDimensions;
            }
            else
                result += type->name;
        }

        return result;
    }

    void dumpConstruct(const Construct* construct, const uint32_t level = 0);

    void dumpDeclaration(const Declaration* declaration, const uint32_t level = 0)
    {
        std::cout << " " << toString(declaration->getDeclarationKind());

        switch (declaration->getDeclarationKind())
        {
            case Declaration::Kind::Empty:
            {
                std::cout << '\n';
                break;
            }

            case Declaration::Kind::Type:
            {
                auto typeDeclaration = static_cast<const TypeDeclaration*>(declaration);
                auto type = typeDeclaration->type;

                std::cout << " " << toString(type->getTypeKind());

                switch (type->getTypeKind())
                {
                    case Type::Kind::Array: // array types can not be declared in code
                    {
                        break;
                    }

                    case Type::Kind::Struct:
                    {
                        const StructType* structType = static_cast<const StructType*>(type);
                        std::cout << ", name: " << structType->name;

                        if (structType->declaration)
                            std::cout << ", first declaration: " << structType->declaration;

                        if (structType->definition)
                            std::cout << ", definition: " << structType->definition;

                        std::cout << '\n';

                        for (const Declaration* memberDeclaration : structType->memberDeclarations)
                            dumpConstruct(memberDeclaration, level + 1);

                        break;
                    }

                    case Type::Kind::Scalar:
                    {
                        const ScalarType* scalarType = static_cast<const ScalarType*>(type);
                        std::cout << ", name: " << scalarType->name << ", scalar type kind: " << toString(scalarType->getScalarTypeKind());
                        break;
                    }

                    case Type::Kind::Vector:
                    {
                        // TODO: implement
                        break;
                    }
                }
                break;
            }

            case Declaration::Kind::Field:
            {
                auto fieldDeclaration = static_cast<const FieldDeclaration*>(declaration);

                std::cout << ", name: " << fieldDeclaration->name << ", type: " << getPrintableTypeName(fieldDeclaration->qualifiedType);

                if (fieldDeclaration->semantic != Semantic::None)
                    std::cout << ", semantic: " << toString(fieldDeclaration->semantic);

                std::cout << '\n';
                break;
            }

            case Declaration::Kind::Callable:
            {
                auto callableDeclaration = static_cast<const CallableDeclaration*>(declaration);

                std::cout << ", callable kind: " << toString(callableDeclaration->getCallableDeclarationKind()) << ", name: " << callableDeclaration->name << ", result type: " << getPrintableTypeName(callableDeclaration->qualifiedType);

                if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::Function)
                {
                    const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(callableDeclaration);

                    if (functionDeclaration->isStatic) std::cout << " static";
                    if (functionDeclaration->isInline) std::cout << " inline";

                    if (functionDeclaration->overloadedOperator != OverloadedOperator::None)
                        std::cout << ", operator: " << toString(functionDeclaration->overloadedOperator);

                    if (functionDeclaration->isProgram)
                        std::cout << ", program: " << toString(functionDeclaration->program);
                }

                if (callableDeclaration->previousDeclaration)
                    std::cout << ", previous declaration: " << callableDeclaration->previousDeclaration;

                if (callableDeclaration->definition)
                    std::cout << ", definition: " << callableDeclaration->definition;

                std::cout << '\n';

                for (ParameterDeclaration* parameter : callableDeclaration->parameterDeclarations)
                    dumpConstruct(parameter, level + 1);

                if (callableDeclaration->body)
                    dumpConstruct(callableDeclaration->body, level + 1);

                break;
            }

            case Declaration::Kind::Variable:
            {
                auto variableDeclaration = static_cast<const VariableDeclaration*>(declaration);
                std::cout << ", name: " << variableDeclaration->name << ", type: " << getPrintableTypeName(variableDeclaration->qualifiedType) << '\n';

                if (variableDeclaration->initialization)
                {
                    dumpConstruct(variableDeclaration->initialization, level + 1);
                }

                break;
            }

            case Declaration::Kind::Parameter:
            {
                auto parameterDeclaration = static_cast<const ParameterDeclaration*>(declaration);
                std::cout << ", name: " << parameterDeclaration->name << ", type: " << getPrintableTypeName(parameterDeclaration->qualifiedType) << '\n';
                break;
            }

            default:
                break;
        }
    }

    void dumpStatement(const Statement* statement, const uint32_t level = 0)
    {
        std::cout << " " << toString(statement->getStatementKind());

        switch (statement->getStatementKind())
        {
            case Statement::Kind::Empty:
            {
                std::cout << '\n';
                break;
            }

            case Statement::Kind::Expression:
            {
                auto expressionStatement = static_cast<const ExpressionStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(expressionStatement->expression, level + 1);
                break;
            }

            case Statement::Kind::Declaration:
            {
                auto declarationStatement = static_cast<const DeclarationStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(declarationStatement->declaration, level + 1);
                break;
            }

            case Statement::Kind::Compound:
            {
                auto compoundStatement = static_cast<const CompoundStatement*>(statement);

                std::cout << '\n';

                for (const auto subSstatement : compoundStatement->statements)
                    dumpConstruct(subSstatement, level + 1);

                break;
            }

            case Statement::Kind::If:
            {
                auto ifStatement = static_cast<const IfStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(ifStatement->condition, level + 1);
                dumpConstruct(ifStatement->body, level + 1);
                if (ifStatement->elseBody) dumpConstruct(ifStatement->elseBody, level + 1);
                break;
            }

            case Statement::Kind::For:
            {
                auto forStatement = static_cast<const ForStatement*>(statement);

                std::cout << '\n';

                if (forStatement->initialization) dumpConstruct(forStatement->initialization, level + 1);
                if (forStatement->condition) dumpConstruct(forStatement->condition, level + 1);
                if (forStatement->increment) dumpConstruct(forStatement->increment, level + 1);
                dumpConstruct(forStatement->body, level + 1);
                break;
            }

            case Statement::Kind::Switch:
            {
                auto switchStatement = static_cast<const SwitchStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(switchStatement->condition, level + 1);
                dumpConstruct(switchStatement->body, level + 1);
                break;
            }

            case Statement::Kind::Case:
            {
                auto caseStatement = static_cast<const CaseStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(caseStatement->condition, level + 1);
                dumpConstruct(caseStatement->body, level + 1);
                break;
            }

            case Statement::Kind::Default:
            {
                auto defaultStatement = static_cast<const DefaultStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(defaultStatement->body, level + 1);
                break;
            }

            case Statement::Kind::While:
            {
                auto whileStatement = static_cast<const WhileStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(whileStatement->condition, level + 1);
                dumpConstruct(whileStatement->body, level + 1);
                break;
            }

            case Statement::Kind::Do:
            {
                auto doStatement = static_cast<const DoStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(doStatement->body, level + 1);
                dumpConstruct(doStatement->condition, level + 1);
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
                auto returnStatement = static_cast<const ReturnStatement*>(statement);

                std::cout << '\n';

                if (returnStatement->result)
                    dumpConstruct(returnStatement->result, level + 1);
                break;
            }

            default:
                break;
        }
    }

    std::string toString(Expression::Category category)
    {
        switch (category)
        {
            case Expression::Category::Lvalue: return "Lvalue";
            case Expression::Category::Rvalue: return "Rvalue";
            default: return "Unknown";
        }
    }

    void dumpExpression(const Expression* expression, const uint32_t level = 0)
    {
        std::cout << " " << toString(expression->getExpressionKind()) << ", category: " << toString(expression->category);

        switch (expression->getExpressionKind())
        {
            case Expression::Kind::Call:
            {
                auto callExpression = static_cast<const CallExpression*>(expression);

                std::cout << '\n';

                dumpConstruct(callExpression->declarationReference, level + 1);

                for (const auto argument : callExpression->arguments)
                    dumpConstruct(argument, level + 1);

                break;
            }

            case Expression::Kind::Literal:
            {
                auto literalExpression = static_cast<const LiteralExpression*>(expression);

                std::cout << ", literal kind: " << toString(literalExpression->getLiteralKind()) << ", value: ";

                switch (literalExpression->getLiteralKind())
                {
                    case LiteralExpression::Kind::Boolean:
                    {
                        auto booleanLiteralExpression = static_cast<const BooleanLiteralExpression*>(literalExpression);
                        std::cout << (booleanLiteralExpression->value ? "true" : "false");
                        break;
                    }
                    case LiteralExpression::Kind::Integer:
                    {
                        auto integerLiteralExpression = static_cast<const IntegerLiteralExpression*>(literalExpression);
                        std::cout << integerLiteralExpression->value;
                        break;
                    }
                    case LiteralExpression::Kind::FloatingPoint:
                    {
                        auto floatingPointLiteralExpression = static_cast<const FloatingPointLiteralExpression*>(literalExpression);
                        std::cout << floatingPointLiteralExpression->value;
                        break;
                    }
                    case LiteralExpression::Kind::String:
                    {
                        auto stringLiteralExpression = static_cast<const StringLiteralExpression*>(literalExpression);
                        std::cout << stringLiteralExpression->value;
                        break;
                    }
                }

                std::cout << '\n';
                break;
            }

            case Expression::Kind::DeclarationReference:
            {
                auto declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(expression);

                std::cout << ", name: " << declarationReferenceExpression->declaration->name << ", declaration: " << declarationReferenceExpression->declaration;

                std::cout << '\n';
                break;
            }

            case Expression::Kind::Paren:
            {
                auto parenExpression = static_cast<const ParenExpression*>(expression);

                std::cout << '\n';

                dumpConstruct(parenExpression->expression, level + 1);
                break;
            }

            case Expression::Kind::Member:
            {
                auto memberExpression = static_cast<const MemberExpression*>(expression);

                std::cout << ", field: " << memberExpression->fieldDeclaration->name << '\n';

                dumpConstruct(memberExpression->expression, level + 1);
                break;
            }

            case Expression::Kind::ArraySubscript:
            {
                auto arraySubscriptExpression = static_cast<const ArraySubscriptExpression*>(expression);

                std::cout << '\n';

                dumpConstruct(arraySubscriptExpression->expression, level + 1);
                dumpConstruct(arraySubscriptExpression->subscript, level + 1);
                break;
            }

            case Expression::Kind::UnaryOperator:
            {
                auto unaryOperatorExpression = static_cast<const UnaryOperatorExpression*>(expression);

                std::cout <<", operator: " << toString(unaryOperatorExpression->getOperatorKind()) << '\n';

                dumpConstruct(unaryOperatorExpression->expression, level + 1);
                break;
            }

            case Expression::Kind::BinaryOperator:
            {
                auto binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(expression);

                std::cout << ", operator: " << toString(binaryOperatorExpression->getOperatorKind()) << '\n';

                dumpConstruct(binaryOperatorExpression->leftExpression, level + 1);
                dumpConstruct(binaryOperatorExpression->rightExpression, level + 1);
                break;
            }

            case Expression::Kind::TernaryOperator:
            {
                auto ternaryOperatorExpression = static_cast<const TernaryOperatorExpression*>(expression);

                std::cout << '\n';

                dumpConstruct(ternaryOperatorExpression->condition, level + 1);
                dumpConstruct(ternaryOperatorExpression->leftExpression, level + 1);
                dumpConstruct(ternaryOperatorExpression->rightExpression, level + 1);
                break;
            }

            case Expression::Kind::OperatorCall:
            {
                auto operatorCallExpression = static_cast<const OperatorCallExpression*>(expression);

                std::cout << '\n';

                dumpConstruct(operatorCallExpression->declarationReference, level + 1);

                for (const auto argument : operatorCallExpression->arguments)
                    dumpConstruct(argument, level + 1);

                break;
            }

            case Expression::Kind::TemporaryObject:
            {
                auto temporaryObjectExpression = static_cast<const TemporaryObjectExpression*>(expression);

                auto typeDeclaration = static_cast<const TypeDeclaration*>(temporaryObjectExpression->constructorDeclaration->parent);

                std::cout << " " << typeDeclaration->name << '\n';

                for (const auto parameter : temporaryObjectExpression->parameters)
                    dumpConstruct(parameter, level + 1);

                break;
            }

            case Expression::Kind::InitializerList:
            {
                auto initializerListExpression = static_cast<const InitializerListExpression*>(expression);

                std::cout << '\n';

                for (const auto subExpression : initializerListExpression->expressions)
                    dumpConstruct(subExpression, level + 1);

                break;
            }

            case Expression::Kind::Cast:
            {
                auto castExpression = static_cast<const CastExpression*>(expression);

                std::cout << ", cast kind: " << toString(castExpression->getCastKind()) <<
                    ", type: " << castExpression->qualifiedType.type->name << '\n';

                dumpConstruct(castExpression->expression, level + 1);

                break;
            }
            case Expression::Kind::Sizeof:
            {
                auto sizeofExpression = static_cast<const SizeofExpression*>(expression);

                if (sizeofExpression->expression)
                {
                    std::cout << '\n';
                    dumpConstruct(sizeofExpression->expression, level + 1);
                }
                else if (sizeofExpression->type)
                    std::cout << ", type: " << sizeofExpression->type->name << '\n';
                break;
            }
            case Expression::Kind::VectorElement:
            {
                auto sizeofExpression = static_cast<const VectorElementExpression*>(expression);
                break;
            }
            case Expression::Kind::MatrixElement:
            {
                auto sizeofExpression = static_cast<const MatrixElementExpression*>(expression);
                break;
            }
        }
    }

    void dumpConstruct(const Construct* construct, const uint32_t level)
    {
        for (uint32_t i = 0; i < level; ++i)
            std::cout << "  ";

        std::cout << construct;

        if (construct->parent) std::cout << ", parent: " << construct->parent;
        std::cout << " " << toString(construct->getKind());

        switch (construct->getKind())
        {
            case Construct::Kind::Declaration:
            {
                auto declaration = static_cast<const Declaration*>(construct);
                dumpDeclaration(declaration, level);
                break;
            }

            case Construct::Kind::Statement:
            {
                auto statement = static_cast<const Statement*>(construct);
                dumpStatement(statement, level);
                break;
            }

            case Construct::Kind::Expression:
            {
                auto expression = static_cast<const Expression*>(construct);
                dumpExpression(expression, level);
                break;
            }
        }
    }
}

#endif // UTILS_HPP
