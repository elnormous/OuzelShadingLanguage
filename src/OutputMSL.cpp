//
//  OSL
//

#include <iostream>
#include "OutputMSL.hpp"

bool OutputMSL::output(const ASTContext& context, std::string& code)
{
    for (Declaration* declaration : context.declarations)
    {
        if (!printConstruct(declaration, Options(0), code))
        {
            return false;
        }

        if (declaration->getDeclarationKind() != Declaration::Kind::CALLABLE ||
            !static_cast<const CallableDeclaration*>(declaration)->body) // function doesn't have a body
        {
            code += ";";
        }

        code += "\n";
    }

    return true;
}

static std::pair<std::string, std::string> getPrintableTypeName(const QualifiedType& qualifiedType)
{
    std::pair<std::string, std::string> result;

    if (qualifiedType.isVolatile) result.first += "volatile ";
    if (qualifiedType.isConst) result.first += "const ";

    if (!qualifiedType.typeDeclaration)
    {
        result.first += "void";
    }
    else
    {
        TypeDeclaration* typeDeclaration = qualifiedType.typeDeclaration;
        while (typeDeclaration->getTypeKind() == TypeDeclaration::Kind::ARRAY)
        {
            ArrayTypeDeclaration* arrayTypeDeclaration = static_cast<ArrayTypeDeclaration*>(typeDeclaration);

            result.second = "[" + std::to_string(arrayTypeDeclaration->size) + "]" + result.second;

            typeDeclaration = arrayTypeDeclaration->elementType.typeDeclaration;
        }

        result.first = typeDeclaration->name;
    }

    return result;
}

bool OutputMSL::printDeclaration(const Declaration* declaration, Options options, std::string& code)
{
    switch (declaration->getDeclarationKind())
    {
        case Declaration::Kind::NONE:
        {
            break;
        }

        case Declaration::Kind::EMPTY:
        {
            code.append(options.indentation, ' ');
            break;
        }

        case Declaration::Kind::TYPE:
        {
            code.append(options.indentation, ' ');

            const TypeDeclaration* typeDeclaration = static_cast<const TypeDeclaration*>(declaration);

            if (typeDeclaration->getTypeKind() != TypeDeclaration::Kind::STRUCT) return false;

            const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(declaration);
            code += "struct " + structDeclaration->name;

            // if this is the definition
            if (structDeclaration->definition == structDeclaration)
            {
                code.append(options.indentation, ' ');
                code += "\n{\n";

                for (const Declaration* memberDeclaration : structDeclaration->memberDeclarations)
                {
                    if (!printConstruct(memberDeclaration, Options(options.indentation + 4), code))
                    {
                        return false;
                    }

                    code += ";\n";
                }

                code.append(options.indentation, ' ');
                code += "}";
            }

            break;
        }

        case Declaration::Kind::FIELD:
        {
            code.append(options.indentation, ' ');

            const FieldDeclaration* fieldDeclaration = static_cast<const FieldDeclaration*>(declaration);

            std::pair<std::string, std::string> printableTypeName = getPrintableTypeName(fieldDeclaration->qualifiedType);

            code += printableTypeName.first + " " + fieldDeclaration->name + printableTypeName.second;

            if (fieldDeclaration->semantic != Semantic::NONE)
            {
                code += ":";

                switch (fieldDeclaration->semantic)
                {
                    case Semantic::NONE: break;
                    case Semantic::BINORMAL:
                        code += "BINORMAL";
                        break;
                    case Semantic::BLEND_INDICES:
                        code += "BLEND_INDICES";
                        break;
                    case Semantic::BLEND_WEIGHT:
                        code += "BLEND_WEIGHT";
                        break;
                    case Semantic::COLOR:
                        code += "COLOR";
                        break;
                    case Semantic::NORMAL:
                        code += "NORMAL";
                        break;
                    case Semantic::POSITION:
                        code += "POSITION";
                        break;
                    case Semantic::POSITION_TRANSFORMED:
                        code += "POSITION_TRANSFORMED";
                        break;
                    case Semantic::POINT_SIZE:
                        code += "POINT_SIZE";
                        break;
                    case Semantic::TANGENT:
                        code += "TANGENT";
                        break;
                    case Semantic::TEXTURE_COORDINATES:
                        code += "TEXTURE_COORDINATES";
                        break;
                }
            }

            break;
        }

        case Declaration::Kind::CALLABLE:
        {
            code.append(options.indentation, ' ');

            const CallableDeclaration* callableDeclaration = static_cast<const CallableDeclaration*>(declaration);

            if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::FUNCTION)
            {
                const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(callableDeclaration);

                if (functionDeclaration->isStatic) code += "static ";
                if (functionDeclaration->isInline) code += "inline ";

                std::pair<std::string, std::string> printableTypeName = getPrintableTypeName(functionDeclaration->qualifiedType);

                code += printableTypeName.first + " " + functionDeclaration->name + "(";

                bool firstParameter = true;

                for (ParameterDeclaration* parameter : functionDeclaration->parameterDeclarations)
                {
                    if (!firstParameter) code += ", ";
                    firstParameter = false;

                    if (!printConstruct(parameter, Options(0), code))
                    {
                        return false;
                    }
                }

                code += ")";

                if (functionDeclaration->body)
                {
                    code += "\n";

                    if (!printConstruct(functionDeclaration->body, options, code))
                    {
                        return false;
                    }
                }
            }

            break;
        }

        case Declaration::Kind::VARIABLE:
        {
            code.append(options.indentation, ' ');

            const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(declaration);
            if (variableDeclaration->isStatic) code += "static ";

            std::pair<std::string, std::string> printableTypeName = getPrintableTypeName(variableDeclaration->qualifiedType);

            code += printableTypeName.first + " " + variableDeclaration->name + printableTypeName.second;

            if (variableDeclaration->initialization)
            {
                code += " = ";
                if (!printConstruct(variableDeclaration->initialization, Options(0), code))
                {
                    return false;
                }
            }

            break;
        }

        case Declaration::Kind::PARAMETER:
        {
            code.append(options.indentation, ' ');

            const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(declaration);

            std::pair<std::string, std::string> printableTypeName = getPrintableTypeName(parameterDeclaration->qualifiedType);

            code += printableTypeName.first + " " + parameterDeclaration->name + printableTypeName.second;
            break;
        }
    }

    return true;
}

bool OutputMSL::printStatement(const Statement* statement, Options options, std::string& code)
{
    switch (statement->getStatementKind())
    {
        case Statement::Kind::NONE:
        {
            break;
        }

        case Statement::Kind::EMPTY:
        {
            code.append(options.indentation, ' ');
            code += ";";
            break;
        }

        case Statement::Kind::EXPRESSION:
        {
            code.append(options.indentation, ' ');

            const ExpressionStatement* expressionStatement = static_cast<const ExpressionStatement*>(statement);
            if (!printConstruct(expressionStatement->expression, Options(0), code))
            {
                return false;
            }

            code += ";";
            break;
        }

        case Statement::Kind::DECLARATION:
        {
            code.append(options.indentation, ' ');

            const DeclarationStatement* declarationStatement = static_cast<const DeclarationStatement*>(statement);
            if (!printConstruct(declarationStatement->declaration, Options(0), code))
            {
                return false;
            }

            code += ";";
            break;
        }

        case Statement::Kind::COMPOUND:
        {
            const CompoundStatement* compoundStatement = static_cast<const CompoundStatement*>(statement);

            code.append(options.indentation, ' ');
            code += "{\n";

            for (Statement* statement : compoundStatement->statements)
            {
                if (!printConstruct(statement, Options(options.indentation + 4), code))
                {
                    return false;
                }

                code += "\n";
            }

            code.append(options.indentation, ' ');
            code += "}";
            break;
        }

        case Statement::Kind::IF:
        {
            code.append(options.indentation, ' ');

            const IfStatement* ifStatement = static_cast<const IfStatement*>(statement);
            code += "if (";

            if (!printConstruct(ifStatement->condition, Options(0), code))
            {
                return false;
            }

            code += ")\n";

            if (ifStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
            {
                if (!printConstruct(ifStatement->body, options, code))
                {
                    return false;
                }
            }
            else
            {
                if (!printConstruct(ifStatement->body, Options(options.indentation + 4), code))
                {
                    return false;
                }
            }

            if (ifStatement->elseBody)
            {
                code += "\n";
                code.append(options.indentation, ' ');
                code += "else\n";

                if (ifStatement->elseBody->getStatementKind() == Statement::Kind::COMPOUND)
                {
                    if (!printConstruct(ifStatement->elseBody, options, code))
                    {
                        return false;
                    }
                }
                else
                {
                    if (!printConstruct(ifStatement->elseBody, Options(options.indentation + 4), code))
                    {
                        return false;
                    }
                }
            }
            break;
        }

        case Statement::Kind::FOR:
        {
            code.append(options.indentation, ' ');

            const ForStatement* forStatement = static_cast<const ForStatement*>(statement);
            code += "for (";

            if (forStatement->initialization)
            {
                if (!printConstruct(forStatement->initialization, Options(0), code))
                {
                    return false;
                }
            }

            code += "; ";

            if (forStatement->condition)
            {
                if (!printConstruct(forStatement->condition, Options(0), code))
                {
                    return false;
                }
            }

            code += "; ";

            if (forStatement->increment)
            {
                if (!printConstruct(forStatement->increment, Options(0), code))
                {
                    return false;
                }
            }

            code += ")\n";

            if (forStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
            {
                if (!printConstruct(forStatement->body, options, code))
                {
                    return false;
                }
            }
            else
            {
                if (!printConstruct(forStatement->body, Options(options.indentation + 4), code))
                {
                    return false;
                }
            }
            break;
        }

        case Statement::Kind::SWITCH:
        {
            code.append(options.indentation, ' ');

            const SwitchStatement* switchStatement = static_cast<const SwitchStatement*>(statement);
            code += "switch (";

            if (!printConstruct(switchStatement->condition, Options(0), code))
            {
                return false;
            }

            code += ")\n";

            if (switchStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
            {
                if (!printConstruct(switchStatement->body, options, code))
                {
                    return false;
                }
            }
            else
            {
                if (!printConstruct(switchStatement->body, Options(options.indentation + 4), code))
                {
                    return false;
                }
            }

            break;
        }

        case Statement::Kind::CASE:
        {
            code.append(options.indentation, ' ');

            const CaseStatement* caseStatement = static_cast<const CaseStatement*>(statement);
            code += "case ";

            if (!printConstruct(caseStatement->condition, Options(0), code))
            {
                return false;
            }

            code += ":\n";

            if (caseStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
            {
                if (!printConstruct(caseStatement->body, options, code))
                {
                    return false;
                }
            }
            else
            {
                if (!printConstruct(caseStatement->body, Options(options.indentation + 4), code))
                {
                    return false;
                }
            }

            break;
        }

        case Statement::Kind::DEFAULT:
        {
            code.append(options.indentation, ' ');

            const DefaultStatement* defaultStatement = static_cast<const DefaultStatement*>(statement);
            code += "default:\n";

            if (defaultStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
            {
                if (!printConstruct(defaultStatement->body, options, code))
                {
                    return false;
                }
            }
            else
            {
                if (!printConstruct(defaultStatement->body, Options(options.indentation + 4), code))
                {
                    return false;
                }
            }

            break;
        }

        case Statement::Kind::WHILE:
        {
            code.append(options.indentation, ' ');

            const WhileStatement* whileStatement = static_cast<const WhileStatement*>(statement);
            code += "while (";

            if (!printConstruct(whileStatement->condition, Options(0), code))
            {
                return false;
            }

            code += ")\n";

            if (whileStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
            {
                if (!printConstruct(whileStatement->body, options, code))
                {
                    return false;
                }
            }
            else
            {
                if (!printConstruct(whileStatement->body, Options(options.indentation + 4), code))
                {
                    return false;
                }
            }
            break;
        }

        case Statement::Kind::DO:
        {
            code.append(options.indentation, ' ');

            const DoStatement* doStatement = static_cast<const DoStatement*>(statement);
            code += "do\n";

            if (doStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
            {
                if (!printConstruct(doStatement->body, options, code))
                {
                    return false;
                }
            }
            else
            {
                if (!printConstruct(doStatement->body, Options(options.indentation + 4), code))
                {
                    return false;
                }
            }

            code += "\n";

            code.append(options.indentation, ' ');
            code += "while (";

            if (!printConstruct(doStatement->condition, Options(0), code))
            {
                return false;
            }

            code += ");";

            break;
        }

        case Statement::Kind::BREAK:
        {
            code.append(options.indentation, ' ');
            code += "break;";
            break;
        }

        case Statement::Kind::CONTINUE:
        {
            code.append(options.indentation, ' ');
            code += "continue;";
            break;
        }

        case Statement::Kind::RETURN:
        {
            code.append(options.indentation, ' ');

            const ReturnStatement* returnStatement = static_cast<const ReturnStatement*>(statement);
            code += "return";

            if (returnStatement->result)
            {
                code += " ";

                if (!printConstruct(returnStatement->result, Options(0), code))
                {
                    return false;
                }
            }

            code += ";";
            break;
        }
    }

    return true;
}

bool OutputMSL::printExpression(const Expression* expression, Options options, std::string& code)
{
    switch (expression->getExpressionKind())
    {
        case Expression::Kind::NONE:
        {
            break;
        }

        case Expression::Kind::CALL:
        {
            code.append(options.indentation, ' ');

            const CallExpression* callExpression = static_cast<const CallExpression*>(expression);

            if (!printConstruct(callExpression->declarationReference, Options(0), code))
            {
                return false;
            }

            code += "(";

            bool firstParameter = true;

            for (Expression* parameter : callExpression->parameters)
            {
                if (!firstParameter) code += ", ";
                firstParameter = false;

                if (!printConstruct(parameter, Options(0), code))
                {
                    return false;
                }
            }

            code += ")";
            break;
        }

        case Expression::Kind::LITERAL:
        {
            code.append(options.indentation, ' ');

            const LiteralExpression* literalExpression = static_cast<const LiteralExpression*>(expression);

            switch (literalExpression->getLiteralKind())
            {
                case LiteralExpression::Kind::NONE: break;
                case LiteralExpression::Kind::BOOLEAN:
                {
                    const BooleanLiteralExpression* booleanLiteralExpression = static_cast<const BooleanLiteralExpression*>(literalExpression);
                    code += (booleanLiteralExpression->value ? "true" : "false");
                    break;
                }
                case LiteralExpression::Kind::INTEGER:
                {
                    const IntegerLiteralExpression* integerLiteralExpression = static_cast<const IntegerLiteralExpression*>(literalExpression);
                    code += std::to_string(integerLiteralExpression->value);
                    break;
                }
                case LiteralExpression::Kind::FLOATING_POINT:
                {
                    const FloatingPointLiteralExpression* floatingPointLiteralExpression = static_cast<const FloatingPointLiteralExpression*>(literalExpression);
                    code += std::to_string(floatingPointLiteralExpression->value);
                    break;
                }
                case LiteralExpression::Kind::STRING:
                {
                    const StringLiteralExpression* stringLiteralExpression = static_cast<const StringLiteralExpression*>(literalExpression);
                    code += stringLiteralExpression->value;
                    break;
                }
            }
            break;
        }

        case Expression::Kind::DECLARATION_REFERENCE:
        {
            code.append(options.indentation, ' ');

            const DeclarationReferenceExpression* declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(expression);
            Declaration* declaration = declarationReferenceExpression->declaration;

            switch (declaration->getDeclarationKind())
            {
                case Declaration::Kind::CALLABLE:
                {
                    const CallableDeclaration* callableDeclaration = static_cast<const CallableDeclaration*>(declaration);

                    if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::FUNCTION)
                    {
                        const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(callableDeclaration);
                        code += functionDeclaration->name;
                    }
                    break;
                }
                case Declaration::Kind::VARIABLE:
                {
                    const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(declaration);
                    code += variableDeclaration->name;
                    break;
                }
                case Declaration::Kind::PARAMETER:
                {
                    const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(declaration);
                    code += parameterDeclaration->name;
                    break;
                }
                default:
                    return false;
            }

            break;
        }

        case Expression::Kind::PAREN:
        {
            code.append(options.indentation, ' ');

            const ParenExpression* parenExpression = static_cast<const ParenExpression*>(expression);
            code += "(";

            if (!printConstruct(parenExpression->expression, Options(0), code))
            {
                return false;
            }

            code += ")";
            break;
        }

        case Expression::Kind::MEMBER:
        {
            code.append(options.indentation, ' ');

            const MemberExpression* memberExpression = static_cast<const MemberExpression*>(expression);

            if (!printConstruct(memberExpression->expression, Options(0), code))
            {
                return false;
            }

            code += ".";

            if (!memberExpression->fieldDeclaration)
            {
                std::cerr << "Field does not exist";
                return false;
            }

            code += memberExpression->fieldDeclaration->name;

            break;
        }

        case Expression::Kind::ARRAY_SUBSCRIPT:
        {
            code.append(options.indentation, ' ');

            const ArraySubscriptExpression* arraySubscriptExpression = static_cast<const ArraySubscriptExpression*>(expression);

            if (!printConstruct(arraySubscriptExpression->expression, Options(0), code))
            {
                return false;
            }

            code += "[";

            if (!printConstruct(arraySubscriptExpression->subscript, Options(0), code))
            {
                return false;
            }

            code += "]";

            break;
        }

        case Expression::Kind::UNARY:
        {
            code.append(options.indentation, ' ');

            const UnaryOperatorExpression* unaryOperatorExpression = static_cast<const UnaryOperatorExpression*>(expression);

            switch (unaryOperatorExpression->operatorKind)
            {
                case UnaryOperatorExpression::Kind::NEGATION: code += "!"; break;
                case UnaryOperatorExpression::Kind::POSITIVE: code += "+"; break;
                case UnaryOperatorExpression::Kind::NEGATIVE: code += "-"; break;
                default:
                    std::cerr << "Unknown operator";
                    return false;
            }

            if (!printConstruct(unaryOperatorExpression->expression, Options(0), code))
            {
                return false;
            }
            break;
        }

        case Expression::Kind::BINARY:
        {
            code.append(options.indentation, ' ');

            const BinaryOperatorExpression* binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(expression);
            if (!printConstruct(binaryOperatorExpression->leftExpression, Options(0), code))
            {
                return false;
            }

            switch (binaryOperatorExpression->operatorKind)
            {
                case BinaryOperatorExpression::Kind::ADDITION: code += " + "; break;
                case BinaryOperatorExpression::Kind::SUBTRACTION: code += " - "; break;
                case BinaryOperatorExpression::Kind::MULTIPLICATION: code += " * "; break;
                case BinaryOperatorExpression::Kind::DIVISION: code += " / "; break;
                case BinaryOperatorExpression::Kind::ADDITION_ASSIGNMENT: code += " += "; break;
                case BinaryOperatorExpression::Kind::SUBTRACTION_ASSIGNMENT: code += " -= "; break;
                case BinaryOperatorExpression::Kind::MULTIPLICATION_ASSIGNMENT: code += " *= "; break;
                case BinaryOperatorExpression::Kind::DIVISION_ASSIGNMENT: code += " /= "; break;
                case BinaryOperatorExpression::Kind::LESS_THAN: code += " < "; break;
                case BinaryOperatorExpression::Kind::LESS_THAN_EQUAL: code += " <= "; break;
                case BinaryOperatorExpression::Kind::GREATER_THAN: code += " > "; break;
                case BinaryOperatorExpression::Kind::GREATER_THAN_EQUAL: code += " >= "; break;
                case BinaryOperatorExpression::Kind::EQUALITY: code += " == "; break;
                case BinaryOperatorExpression::Kind::INEQUALITY: code += " != "; break;
                case BinaryOperatorExpression::Kind::ASSIGNMENT: code += " = "; break;
                case BinaryOperatorExpression::Kind::OR: code += " || "; break;
                case BinaryOperatorExpression::Kind::AND: code += " && "; break;
                case BinaryOperatorExpression::Kind::COMMA: code += ", "; break;
                default:
                    std::cerr << "Unknown operator";
                    return false;
            }

            if (!printConstruct(binaryOperatorExpression->rightExpression, Options(0), code))
            {
                return false;
            }
            break;
        }

        case Expression::Kind::TERNARY:
        {
            code.append(options.indentation, ' ');

            const TernaryOperatorExpression* ternaryOperatorExpression = static_cast<const TernaryOperatorExpression*>(expression);

            if (!printConstruct(ternaryOperatorExpression->condition, Options(0), code))
            {
                return false;
            }

            code += " ? ";

            if (!printConstruct(ternaryOperatorExpression->leftExpression, Options(0), code))
            {
                return false;
            }

            code += " : ";

            if (!printConstruct(ternaryOperatorExpression->rightExpression, Options(0), code))
            {
                return false;
            }
            break;
        }

        case Expression::Kind::TEMPORARY_OBJECT:
        {
            code.append(options.indentation, ' ');

            const TemporaryObjectExpression* temporaryObjectExpression = static_cast<const TemporaryObjectExpression*>(expression);

            const TypeDeclaration* typeDeclaration = static_cast<const TypeDeclaration*>(temporaryObjectExpression->constructorDeclaration->parent);

            if (typeDeclaration->getTypeKind() != TypeDeclaration::Kind::STRUCT) return false;

            const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(typeDeclaration);

            code += structDeclaration->name + "(";

            bool firstParameter = true;

            for (Expression* parameter : temporaryObjectExpression->parameters)
            {
                if (!firstParameter) code += ", ";
                firstParameter = false;

                if (!printConstruct(parameter, Options(0), code))
                {
                    return false;
                }
            }

            code += ")";

            break;
        }

        case Expression::Kind::INITIALIZER_LIST:
        {
            code.append(options.indentation, ' ');

            const InitializerListExpression* initializerListExpression = static_cast<const InitializerListExpression*>(expression);

            code += "{";

            bool firstExpression = true;

            for (Expression* expression : initializerListExpression->expressions)
            {
                if (!firstExpression) code += ", ";
                firstExpression = false;

                if (!printConstruct(expression, Options(0), code))
                {
                    return false;
                }
            }

            code += "}";

            break;
        }

        case Expression::Kind::CAST:
        {
            code.append(options.indentation, ' ');

            const CastExpression* castExpression = static_cast<const CastExpression*>(expression);

            if (castExpression->getCastKind() == CastExpression::Kind::EXPLICIT)
            {
                code += castExpression->qualifiedType.typeDeclaration->name + "(";

                if (!printConstruct(castExpression->expression, Options(0), code))
                {
                    return false;
                }

                code += ")";
            }
            else
            {
                if (!printConstruct(castExpression->expression, Options(0), code))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool OutputMSL::printConstruct(const Construct* construct, Options options, std::string& code)
{
    switch (construct->getKind())
    {
        case Construct::Kind::NONE:
        {
            break;
        }

        case Construct::Kind::DECLARATION:
        {
            const Declaration* declaration = static_cast<const Declaration*>(construct);
            printDeclaration(declaration, options, code);
            break;
        }

        case Construct::Kind::STATEMENT:
        {
            const Statement* statement = static_cast<const Statement*>(construct);
            printStatement(statement, options, code);
            break;
        }

        case Construct::Kind::EXPRESSION:
        {
            const Expression* expression = static_cast<const Expression*>(construct);
            printExpression(expression, options, code);
            break;
        }
    }

    return true;
}

