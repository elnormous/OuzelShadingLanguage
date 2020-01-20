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

    std::string toString(TypeDeclaration::Kind kind)
    {
        switch (kind)
        {
            case TypeDeclaration::Kind::Array: return "Array";
            case TypeDeclaration::Kind::Scalar: return "Scalar";
            case TypeDeclaration::Kind::Struct: return "Struct";
                //case TypeDeclaration::Kind::TypeDefinition: return "TypeDefinition";
            default: return "Unknown";
        }
    }

    std::string toString(ScalarTypeDeclaration::Kind kind)
    {
        switch (kind)
        {
            case ScalarTypeDeclaration::Kind::Boolean: return "Boolean";
            case ScalarTypeDeclaration::Kind::Integer: return "Integer";
            case ScalarTypeDeclaration::Kind::FloatingPoint: return "FloatingPoint";
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
            case CallableDeclaration::Kind::Operator: return "Operator";
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

    std::string toString(Operator op)
    {
        switch (op)
        {
            case Operator::Negation: return "Negation";
            case Operator::Positive: return "Positive";
            case Operator::Negative: return "Negative";
            case Operator::Addition: return "Addition";
            case Operator::Subtraction: return "Subtraction";
            case Operator::Multiplication: return "Multiplication";
            case Operator::Division: return "Division";
            case Operator::AdditionAssignment: return "AdditionAssignment";
            case Operator::SubtractAssignment: return "SubtractAssignment";
            case Operator::MultiplicationAssignment: return "MultiplicationAssignment";
            case Operator::DivisionAssignment: return "DivisionAssignment";
            case Operator::LessThan: return "LessThan";
            case Operator::LessThanEqual: return "LessThanEqual";
            case Operator::GreaterThan: return "GreaterThan";
            case Operator::GraterThanEqual: return "GraterThanEqual";
            case Operator::Equality: return "Equality";
            case Operator::Inequality: return "Inequality";
            case Operator::Assignment: return "Assignment";
            case Operator::Or: return "Or";
            case Operator::And: return "And";
            case Operator::Comma: return "Comma";
            case Operator::Conditional: return "Conditional";
            default: return "Unknown";
        }
    }

    std::string getPrintableTypeName(const QualifiedType& qualifiedType)
    {
        std::string result;

        if ((qualifiedType.qualifiers & Qualifiers::Volatile) == Qualifiers::Volatile) result += "volatile ";
        if ((qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const) result += "const ";

        if (!qualifiedType.typeDeclaration)
        {
            result += "void";
        }
        else
        {
            const TypeDeclaration* typeDeclaration = qualifiedType.typeDeclaration;

            if (typeDeclaration->getTypeKind() == TypeDeclaration::Kind::Array)
            {
                const TypeDeclaration* currentTypeDeclaration = typeDeclaration;

                std::string arrayDimensions;
                while (currentTypeDeclaration->getTypeKind() == TypeDeclaration::Kind::Array)
                {
                    const ArrayTypeDeclaration* arrayTypeDeclaration = static_cast<const ArrayTypeDeclaration*>(typeDeclaration);
                    arrayDimensions += "[" + std::to_string(arrayTypeDeclaration->size) + "]";

                    currentTypeDeclaration = arrayTypeDeclaration->elementType.typeDeclaration;
                }

                result += (currentTypeDeclaration ? currentTypeDeclaration->name : "void") + arrayDimensions;
            }
            else
                result += typeDeclaration->name;
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
                const TypeDeclaration* typeDeclaration = static_cast<const TypeDeclaration*>(declaration);

                std::cout << " " << toString(typeDeclaration->getTypeKind());

                switch (typeDeclaration->getTypeKind())
                {
                    case TypeDeclaration::Kind::Array: // array types can not be declared in code
                    {
                        break;
                    }

                    case TypeDeclaration::Kind::Struct:
                    {
                        const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(typeDeclaration);
                        std::cout << ", name: " << structDeclaration->name;

                        if (structDeclaration->previousDeclaration)
                            std::cout << ", previous declaration: " << structDeclaration->previousDeclaration;

                        if (structDeclaration->definition)
                            std::cout << ", definition: " << structDeclaration->definition;

                        std::cout << '\n';

                        for (const Declaration* memberDeclaration : structDeclaration->memberDeclarations)
                            dumpConstruct(memberDeclaration, level + 1);

                        break;
                    }

                    case TypeDeclaration::Kind::Scalar:
                    {
                        const ScalarTypeDeclaration* scalarTypeDeclaration = static_cast<const ScalarTypeDeclaration*>(typeDeclaration);
                        std::cout << ", name: " << scalarTypeDeclaration->name << ", scalar type kind: " << toString(scalarTypeDeclaration->getScalarTypeKind());
                        break;
                    }
                }
                break;
            }

            case Declaration::Kind::Field:
            {
                const FieldDeclaration* fieldDeclaration = static_cast<const FieldDeclaration*>(declaration);

                std::cout << ", name: " << fieldDeclaration->name << ", type: " << getPrintableTypeName(fieldDeclaration->qualifiedType);

                if (fieldDeclaration->semantic != Semantic::None)
                    std::cout << ", semantic: " << toString(fieldDeclaration->semantic);

                std::cout << '\n';
                break;
            }

            case Declaration::Kind::Callable:
            {
                const CallableDeclaration* callableDeclaration = static_cast<const CallableDeclaration*>(declaration);

                std::cout << ", callable kind: " << toString(callableDeclaration->getCallableDeclarationKind()) << ", name: " << callableDeclaration->name << ", result type: " << getPrintableTypeName(callableDeclaration->qualifiedType);

                if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::Function)
                {
                    const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(callableDeclaration);

                    if (functionDeclaration->isStatic) std::cout << " static";
                    if (functionDeclaration->isInline) std::cout << " inline";

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
                const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(declaration);
                std::cout << ", name: " << variableDeclaration->name << ", type: " << getPrintableTypeName(variableDeclaration->qualifiedType) << '\n';

                if (variableDeclaration->initialization)
                {
                    dumpConstruct(variableDeclaration->initialization, level + 1);
                }

                break;
            }

            case Declaration::Kind::Parameter:
            {
                const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(declaration);
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
                const ExpressionStatement* expressionStatement = static_cast<const ExpressionStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(expressionStatement->expression, level + 1);
                break;
            }

            case Statement::Kind::Declaration:
            {
                const DeclarationStatement* declarationStatement = static_cast<const DeclarationStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(declarationStatement->declaration, level + 1);
                break;
            }

            case Statement::Kind::Compound:
            {
                const CompoundStatement* compoundStatement = static_cast<const CompoundStatement*>(statement);

                std::cout << '\n';

                for (Statement* subSstatement : compoundStatement->statements)
                    dumpConstruct(subSstatement, level + 1);

                break;
            }

            case Statement::Kind::If:
            {
                const IfStatement* ifStatement = static_cast<const IfStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(ifStatement->condition, level + 1);
                dumpConstruct(ifStatement->body, level + 1);
                if (ifStatement->elseBody) dumpConstruct(ifStatement->elseBody, level + 1);
                break;
            }

            case Statement::Kind::For:
            {
                const ForStatement* forStatement = static_cast<const ForStatement*>(statement);

                std::cout << '\n';

                if (forStatement->initialization) dumpConstruct(forStatement->initialization, level + 1);
                if (forStatement->condition) dumpConstruct(forStatement->condition, level + 1);
                if (forStatement->increment) dumpConstruct(forStatement->increment, level + 1);
                dumpConstruct(forStatement->body, level + 1);
                break;
            }

            case Statement::Kind::Switch:
            {
                const SwitchStatement* switchStatement = static_cast<const SwitchStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(switchStatement->condition, level + 1);
                dumpConstruct(switchStatement->body, level + 1);
                break;
            }

            case Statement::Kind::Case:
            {
                const CaseStatement* caseStatement = static_cast<const CaseStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(caseStatement->condition, level + 1);
                dumpConstruct(caseStatement->body, level + 1);
                break;
            }

            case Statement::Kind::Default:
            {
                const DefaultStatement* defaultStatement = static_cast<const DefaultStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(defaultStatement->body, level + 1);
                break;
            }

            case Statement::Kind::While:
            {
                const WhileStatement* whileStatement = static_cast<const WhileStatement*>(statement);

                std::cout << '\n';

                dumpConstruct(whileStatement->condition, level + 1);
                dumpConstruct(whileStatement->body, level + 1);
                break;
            }

            case Statement::Kind::Do:
            {
                const DoStatement* doStatement = static_cast<const DoStatement*>(statement);

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
                const ReturnStatement* returnStatement = static_cast<const ReturnStatement*>(statement);

                std::cout << '\n';

                if (returnStatement->result)
                {
                    dumpConstruct(returnStatement->result, level + 1);
                }
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
                const CallExpression* callExpression = static_cast<const CallExpression*>(expression);

                std::cout << '\n';

                dumpConstruct(callExpression->declarationReference, level + 1);

                for (Expression* argument : callExpression->arguments)
                    dumpConstruct(argument, level + 1);

                break;
            }

            case Expression::Kind::Literal:
            {
                const LiteralExpression* literalExpression = static_cast<const LiteralExpression*>(expression);

                std::cout << ", literal kind: " << toString(literalExpression->getLiteralKind()) << ", value: ";

                switch (literalExpression->getLiteralKind())
                {
                    case LiteralExpression::Kind::Boolean:
                    {
                        const BooleanLiteralExpression* booleanLiteralExpression = static_cast<const BooleanLiteralExpression*>(literalExpression);
                        std::cout << (booleanLiteralExpression->value ? "true" : "false");
                        break;
                    }
                    case LiteralExpression::Kind::Integer:
                    {
                        const IntegerLiteralExpression* integerLiteralExpression = static_cast<const IntegerLiteralExpression*>(literalExpression);
                        std::cout << integerLiteralExpression->value;
                        break;
                    }
                    case LiteralExpression::Kind::FloatingPoint:
                    {
                        const FloatingPointLiteralExpression* floatingPointLiteralExpression = static_cast<const FloatingPointLiteralExpression*>(literalExpression);
                        std::cout << floatingPointLiteralExpression->value;
                        break;
                    }
                    case LiteralExpression::Kind::String:
                    {
                        const StringLiteralExpression* stringLiteralExpression = static_cast<const StringLiteralExpression*>(literalExpression);
                        std::cout << stringLiteralExpression->value;
                        break;
                    }
                }

                std::cout << '\n';
                break;
            }

            case Expression::Kind::DeclarationReference:
            {
                const DeclarationReferenceExpression* declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(expression);

                std::cout << ", name: " << declarationReferenceExpression->declaration->name << ", declaration: " << declarationReferenceExpression->declaration;

                std::cout << '\n';
                break;
            }

            case Expression::Kind::Paren:
            {
                const ParenExpression* parenExpression = static_cast<const ParenExpression*>(expression);

                std::cout << '\n';

                dumpConstruct(parenExpression->expression, level + 1);
                break;
            }

            case Expression::Kind::Member:
            {
                const MemberExpression* memberExpression = static_cast<const MemberExpression*>(expression);

                std::cout << ", field: " << memberExpression->fieldDeclaration->name << '\n';

                dumpConstruct(memberExpression->expression, level + 1);
                break;
            }

            case Expression::Kind::ArraySubscript:
            {
                const ArraySubscriptExpression* arraySubscriptExpression = static_cast<const ArraySubscriptExpression*>(expression);

                std::cout << '\n';

                dumpConstruct(arraySubscriptExpression->expression, level + 1);
                dumpConstruct(arraySubscriptExpression->subscript, level + 1);
                break;
            }

            case Expression::Kind::UnaryOperator:
            {
                const UnaryOperatorExpression* unaryOperatorExpression = static_cast<const UnaryOperatorExpression*>(expression);

                std::cout <<", operator: " << toString(unaryOperatorExpression->operatorDeclaration->op) << '\n';

                dumpConstruct(unaryOperatorExpression->expression, level + 1);
                break;
            }

            case Expression::Kind::BinaryOperator:
            {
                const BinaryOperatorExpression* binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(expression);

                std::cout << ", operator: " << toString(binaryOperatorExpression->operatorDeclaration->op) << '\n';

                dumpConstruct(binaryOperatorExpression->leftExpression, level + 1);
                dumpConstruct(binaryOperatorExpression->rightExpression, level + 1);
                break;
            }

            case Expression::Kind::TernaryOperator:
            {
                const TernaryOperatorExpression* ternaryOperatorExpression = static_cast<const TernaryOperatorExpression*>(expression);

                std::cout << '\n';

                dumpConstruct(ternaryOperatorExpression->condition, level + 1);
                dumpConstruct(ternaryOperatorExpression->leftExpression, level + 1);
                dumpConstruct(ternaryOperatorExpression->rightExpression, level + 1);
                break;
            }

            case Expression::Kind::TemporaryObject:
            {
                const TemporaryObjectExpression* temporaryObjectExpression = static_cast<const TemporaryObjectExpression*>(expression);

                const TypeDeclaration* typeDeclaration = static_cast<const TypeDeclaration*>(temporaryObjectExpression->constructorDeclaration->parent);

                std::cout << " " << typeDeclaration->name << '\n';

                for (Expression* parameter : temporaryObjectExpression->parameters)
                    dumpConstruct(parameter, level + 1);

                break;
            }

            case Expression::Kind::InitializerList:
            {
                const InitializerListExpression* initializerListExpression = static_cast<const InitializerListExpression*>(expression);

                std::cout << '\n';

                for (Expression* subExpression : initializerListExpression->expressions)
                    dumpConstruct(subExpression, level + 1);

                break;
            }

            case Expression::Kind::Cast:
            {
                const CastExpression* castExpression = static_cast<const CastExpression*>(expression);

                std::cout << ", cast kind: " << toString(castExpression->getCastKind()) <<
                ", type: " << castExpression->qualifiedType.typeDeclaration->name << '\n';

                dumpConstruct(castExpression->expression, level + 1);

                break;
            }
            case Expression::Kind::Sizeof:
            {
                const SizeofExpression* sizeofExpression = static_cast<const SizeofExpression*>(expression);

                std::cout << '\n';

                if (sizeofExpression->expression)
                    dumpConstruct(sizeofExpression->expression, level + 1);
                else if (sizeofExpression->type)
                    dumpConstruct(sizeofExpression->type, level + 1);
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
                const Declaration* declaration = static_cast<const Declaration*>(construct);
                dumpDeclaration(declaration, level);
                break;
            }

            case Construct::Kind::Statement:
            {
                const Statement* statement = static_cast<const Statement*>(construct);
                dumpStatement(statement, level);
                break;
            }

            case Construct::Kind::Expression:
            {
                const Expression* expression = static_cast<const Expression*>(construct);
                dumpExpression(expression, level);
                break;
            }
        }
    }
}

#endif // UTILS_HPP
