//
//  OSL
//

#include "OutputMSL.hpp"
#include "Semantic.hpp"

struct BuiltinFunction
{
    BuiltinFunction(const std::string& initName): name(initName) {}
    std::string name;
};

static std::map<std::string, BuiltinFunction> builtinFunctions;

OutputMSL::OutputMSL(Program initProgram,
                     const std::map<Semantic, uint32_t>& initSemantics):
    program(initProgram), semantics(initSemantics)
{
}

std::string OutputMSL::output(const ASTContext& context, bool whitespaces)
{
    std::string result;

    for (Declaration* declaration : context.getDeclarations())
    {
        printConstruct(declaration, Options(0, whitespaces), result);

        if (declaration->getDeclarationKind() != Declaration::Kind::Callable ||
            !static_cast<const CallableDeclaration*>(declaration)->body) // function doesn't have a body
            result += ";";

        if (whitespaces) result += "\n";
    }

    return result;
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

void OutputMSL::printDeclaration(const Declaration* declaration, Options options, std::string& code)
{
    switch (declaration->getDeclarationKind())
    {
        case Declaration::Kind::Empty:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');
            break;
        }

        case Declaration::Kind::Type:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const TypeDeclaration* typeDeclaration = static_cast<const TypeDeclaration*>(declaration);

            if (typeDeclaration->getTypeKind() != TypeDeclaration::Kind::STRUCT)
                throw std::runtime_error("Type declaration must be a struct");

            const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(declaration);
            code += "struct " + structDeclaration->name;

            // if this is the definition
            if (structDeclaration->definition == structDeclaration)
            {
                if (options.whitespaces) code.append(options.indentation, ' ');
                if (options.whitespaces) code += "\n";
                code += "{";
                if (options.whitespaces) code += "\n";

                for (const Declaration* memberDeclaration : structDeclaration->memberDeclarations)
                {
                    printConstruct(memberDeclaration, Options(options.indentation + 4, options.whitespaces), code);

                    code += ";";
                    if (options.whitespaces) code += "\n";
                }

                if (options.whitespaces) code.append(options.indentation, ' ');
                code += "}";
            }

            break;
        }

        case Declaration::Kind::Field:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const FieldDeclaration* fieldDeclaration = static_cast<const FieldDeclaration*>(declaration);

            std::pair<std::string, std::string> printableTypeName = getPrintableTypeName(fieldDeclaration->qualifiedType);

            code += printableTypeName.first + " " + fieldDeclaration->name + printableTypeName.second;

            if (fieldDeclaration->semantic != Semantic::NONE)
            {
                auto attribute = semantics.find(fieldDeclaration->semantic);

                if (attribute == semantics.end())
                    throw std::runtime_error("Invalid semantic " + toString(fieldDeclaration->semantic));

                code += " [[attribute(" + std::to_string(attribute->second) + ")]]";
            }

            break;
        }

        case Declaration::Kind::Callable:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

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
                    if (!firstParameter)
                    {
                        code += ",";
                        if (options.whitespaces) code += " ";
                        firstParameter = false;
                    }

                    printConstruct(parameter, Options(0, options.whitespaces), code);
                }

                code += ")";

                if (functionDeclaration->body)
                {
                    if (options.whitespaces) code += "\n";
                    printConstruct(functionDeclaration->body, options, code);
                }
            }

            break;
        }

        case Declaration::Kind::Variable:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(declaration);
            if (variableDeclaration->storageClass == VariableDeclaration::StorageClass::STATIC) code += "static ";

            std::pair<std::string, std::string> printableTypeName = getPrintableTypeName(variableDeclaration->qualifiedType);

            code += printableTypeName.first + " " + variableDeclaration->name + printableTypeName.second;

            if (variableDeclaration->initialization)
            {
                if (options.whitespaces) code += " ";
                code += "=";
                if (options.whitespaces) code += " ";
                printConstruct(variableDeclaration->initialization, Options(0, options.whitespaces), code);
            }

            break;
        }

        case Declaration::Kind::Parameter:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(declaration);

            std::pair<std::string, std::string> printableTypeName = getPrintableTypeName(parameterDeclaration->qualifiedType);

            code += printableTypeName.first + " " + parameterDeclaration->name + printableTypeName.second;
            break;
        }
    }
}

void OutputMSL::printStatement(const Statement* statement, Options options, std::string& code)
{
    switch (statement->getStatementKind())
    {
        case Statement::Kind::NONE:
        {
            break;
        }

        case Statement::Kind::EMPTY:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');
            code += ";";
            break;
        }

        case Statement::Kind::EXPRESSION:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const ExpressionStatement* expressionStatement = static_cast<const ExpressionStatement*>(statement);
            printConstruct(expressionStatement->expression, Options(0, options.whitespaces), code);

            code += ";";
            break;
        }

        case Statement::Kind::DECLARATION:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const DeclarationStatement* declarationStatement = static_cast<const DeclarationStatement*>(statement);
            printConstruct(declarationStatement->declaration, Options(0, options.whitespaces), code);

            code += ";";
            break;
        }

        case Statement::Kind::COMPOUND:
        {
            const CompoundStatement* compoundStatement = static_cast<const CompoundStatement*>(statement);

            if (options.whitespaces) code.append(options.indentation, ' ');
            code += "{";
            if (options.whitespaces) code += "\n";

            for (Statement* subStatement : compoundStatement->statements)
            {
                printConstruct(subStatement, Options(options.indentation + 4, options.whitespaces), code);
                if (options.whitespaces) code += "\n";
            }

            if (options.whitespaces) code.append(options.indentation, ' ');
            code += "}";
            break;
        }

        case Statement::Kind::IF:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const IfStatement* ifStatement = static_cast<const IfStatement*>(statement);
            code += "if";
            if (options.whitespaces) code += " ";
            code += "(";

            printConstruct(ifStatement->condition, Options(0, options.whitespaces), code);

            code += ")";
            if (options.whitespaces) code += "\n";

            if (ifStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
                printConstruct(ifStatement->body, options, code);
            else
                printConstruct(ifStatement->body, Options(options.indentation + 4, options.whitespaces), code);

            if (ifStatement->elseBody)
            {
                if (options.whitespaces) code += "\n";
                if (options.whitespaces) code.append(options.indentation, ' ');
                code += "else";
                if (options.whitespaces) code += "\n";

                if (ifStatement->elseBody->getStatementKind() == Statement::Kind::COMPOUND)
                    printConstruct(ifStatement->elseBody, options, code);
                else
                    printConstruct(ifStatement->elseBody, Options(options.indentation + 4, options.whitespaces), code);
            }
            break;
        }

        case Statement::Kind::FOR:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const ForStatement* forStatement = static_cast<const ForStatement*>(statement);
            code += "for";
            if (options.whitespaces) code += " ";
            code += "(";

            if (forStatement->initialization)
                printConstruct(forStatement->initialization, Options(0, options.whitespaces), code);

            code += ";";
            if (options.whitespaces) code += " ";

            if (forStatement->condition)
                printConstruct(forStatement->condition, Options(0, options.whitespaces), code);

            code += ";";
            if (options.whitespaces) code += " ";

            if (forStatement->increment)
                printConstruct(forStatement->increment, Options(0, options.whitespaces), code);

            code += ")";
            if (options.whitespaces) code += "\n";

            if (forStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
                printConstruct(forStatement->body, options, code);
            else
                printConstruct(forStatement->body, Options(options.indentation + 4, options.whitespaces), code);
            break;
        }

        case Statement::Kind::SWITCH:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const SwitchStatement* switchStatement = static_cast<const SwitchStatement*>(statement);
            code += "switch";
            if (options.whitespaces) code += " ";
            code += "(";

            printConstruct(switchStatement->condition, Options(0, options.whitespaces), code);

            code += ")";
            if (options.whitespaces) code += "\n";

            if (switchStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
                printConstruct(switchStatement->body, options, code);
            else
                printConstruct(switchStatement->body, Options(options.indentation + 4, options.whitespaces), code);

            break;
        }

        case Statement::Kind::CASE:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const CaseStatement* caseStatement = static_cast<const CaseStatement*>(statement);
            code += "case ";

            printConstruct(caseStatement->condition, Options(0, options.whitespaces), code);

            code += ":";
            if (options.whitespaces) code += "\n";

            if (caseStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
                printConstruct(caseStatement->body, options, code);
            else
                printConstruct(caseStatement->body, Options(options.indentation + 4, options.whitespaces), code);

            break;
        }

        case Statement::Kind::DEFAULT:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const DefaultStatement* defaultStatement = static_cast<const DefaultStatement*>(statement);
            code += "default:";
            if (options.whitespaces) code += "\n";

            if (defaultStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
                printConstruct(defaultStatement->body, options, code);
            else
                printConstruct(defaultStatement->body, Options(options.indentation + 4, options.whitespaces), code);

            break;
        }

        case Statement::Kind::WHILE:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const WhileStatement* whileStatement = static_cast<const WhileStatement*>(statement);
            code += "while";
            if (options.whitespaces) code += " ";
            code += "(";

            printConstruct(whileStatement->condition, Options(0, options.whitespaces), code);

            code += ")";
            if (options.whitespaces) code += "\n";

            if (whileStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
                printConstruct(whileStatement->body, options, code);
            else
                printConstruct(whileStatement->body, Options(options.indentation + 4, options.whitespaces), code);
            break;
        }

        case Statement::Kind::DO:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const DoStatement* doStatement = static_cast<const DoStatement*>(statement);
            code += "do";
            if (options.whitespaces) code += "\n";

            if (doStatement->body->getStatementKind() == Statement::Kind::COMPOUND)
                printConstruct(doStatement->body, options, code);
            else
            {
                if (!options.whitespaces) code += " ";
                printConstruct(doStatement->body, Options(options.indentation + 4, options.whitespaces), code);
            }

            if (options.whitespaces) code += "\n";

            if (options.whitespaces) code.append(options.indentation, ' ');
            code += "while";
            if (options.whitespaces) code += " ";
            code += "(";

            printConstruct(doStatement->condition, Options(0, options.whitespaces), code);

            code += ");";

            break;
        }

        case Statement::Kind::BREAK:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');
            code += "break;";
            break;
        }

        case Statement::Kind::CONTINUE:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');
            code += "continue;";
            break;
        }

        case Statement::Kind::RETURN:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const ReturnStatement* returnStatement = static_cast<const ReturnStatement*>(statement);
            code += "return";

            if (returnStatement->result)
            {
                code += " ";
                printConstruct(returnStatement->result, Options(0, options.whitespaces), code);
            }

            code += ";";
            break;
        }
    }
}

void OutputMSL::printExpression(const Expression* expression, Options options, std::string& code)
{
    switch (expression->getExpressionKind())
    {
        case Expression::Kind::NONE:
        {
            break;
        }

        case Expression::Kind::CALL:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const CallExpression* callExpression = static_cast<const CallExpression*>(expression);

            printConstruct(callExpression->declarationReference, Options(0, options.whitespaces), code);

            code += "(";

            bool firstParameter = true;

            for (Expression* parameter : callExpression->parameters)
            {
                if (!firstParameter)
                {
                    code += ",";
                    if (options.whitespaces) code += " ";
                    firstParameter = false;
                }

                printConstruct(parameter, Options(0, options.whitespaces), code);
            }

            code += ")";
            break;
        }

        case Expression::Kind::LITERAL:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

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
            if (options.whitespaces) code.append(options.indentation, ' ');

            const DeclarationReferenceExpression* declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(expression);
            Declaration* declaration = declarationReferenceExpression->declaration;

            switch (declaration->getDeclarationKind())
            {
                case Declaration::Kind::Callable:
                {
                    const CallableDeclaration* callableDeclaration = static_cast<const CallableDeclaration*>(declaration);

                    if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::FUNCTION)
                    {
                        const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(callableDeclaration);
                        code += functionDeclaration->name;
                    }
                    break;
                }
                case Declaration::Kind::Variable:
                {
                    const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(declaration);
                    code += variableDeclaration->name;
                    break;
                }
                case Declaration::Kind::Parameter:
                {
                    const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(declaration);
                    code += parameterDeclaration->name;
                    break;
                }
                default:
                    throw std::runtime_error("Unknown declaration type");
            }

            break;
        }

        case Expression::Kind::PAREN:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const ParenExpression* parenExpression = static_cast<const ParenExpression*>(expression);
            code += "(";

            printConstruct(parenExpression->expression, Options(0, options.whitespaces), code);

            code += ")";
            break;
        }

        case Expression::Kind::MEMBER:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const MemberExpression* memberExpression = static_cast<const MemberExpression*>(expression);

            printConstruct(memberExpression->expression, Options(0, options.whitespaces), code);

            code += ".";

            if (!memberExpression->fieldDeclaration)
                throw std::runtime_error("Field does not exist");

            code += memberExpression->fieldDeclaration->name;

            break;
        }

        case Expression::Kind::ARRAY_SUBSCRIPT:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const ArraySubscriptExpression* arraySubscriptExpression = static_cast<const ArraySubscriptExpression*>(expression);

            printConstruct(arraySubscriptExpression->expression, Options(0, options.whitespaces), code);

            code += "[";

            printConstruct(arraySubscriptExpression->subscript, Options(0, options.whitespaces), code);

            code += "]";

            break;
        }

        case Expression::Kind::UNARY_OPERATOR:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const UnaryOperatorExpression* unaryOperatorExpression = static_cast<const UnaryOperatorExpression*>(expression);

            switch (unaryOperatorExpression->operatorDeclaration->op)
            {
                case Operator::NEGATION: code += "!"; break;
                case Operator::POSITIVE: code += "+"; break;
                case Operator::NEGATIVE: code += "-"; break;
                default:
                    throw std::runtime_error("Unknown operator");
            }

            printConstruct(unaryOperatorExpression->expression, Options(0, options.whitespaces), code);
            break;
        }

        case Expression::Kind::BINARY_OPERATOR:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const BinaryOperatorExpression* binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(expression);
            printConstruct(binaryOperatorExpression->leftExpression, Options(0, options.whitespaces), code);

            if (options.whitespaces &&
                binaryOperatorExpression->operatorDeclaration->op != Operator::COMMA) code += " ";

            switch (binaryOperatorExpression->operatorDeclaration->op)
            {
                case Operator::ADDITION: code += "+"; break;
                case Operator::SUBTRACTION: code += "-"; break;
                case Operator::MULTIPLICATION: code += "*"; break;
                case Operator::DIVISION: code += " / "; break;
                case Operator::ADDITION_ASSIGNMENT: code += "+="; break;
                case Operator::SUBTRACTION_ASSIGNMENT: code += "-="; break;
                case Operator::MULTIPLICATION_ASSIGNMENT: code += "*="; break;
                case Operator::DIVISION_ASSIGNMENT: code += "/="; break;
                case Operator::LESS_THAN: code += "<"; break;
                case Operator::LESS_THAN_EQUAL: code += "<="; break;
                case Operator::GREATER_THAN: code += ">"; break;
                case Operator::GREATER_THAN_EQUAL: code += ">="; break;
                case Operator::EQUALITY: code += "=="; break;
                case Operator::INEQUALITY: code += "!="; break;
                case Operator::ASSIGNMENT: code += "="; break;
                case Operator::OR: code += "||"; break;
                case Operator::AND: code += "&&"; break;
                case Operator::COMMA: code += ","; break;
                default:
                    throw std::runtime_error("Unknown operator");
            }

            if (options.whitespaces) code += " ";

            printConstruct(binaryOperatorExpression->rightExpression, Options(0, options.whitespaces), code);
            break;
        }

        case Expression::Kind::TERNARY_OPERATOR:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const TernaryOperatorExpression* ternaryOperatorExpression = static_cast<const TernaryOperatorExpression*>(expression);

            printConstruct(ternaryOperatorExpression->condition, Options(0, options.whitespaces), code);

            if (options.whitespaces) code += " ";
            code += "?";
            if (options.whitespaces) code += " ";

            printConstruct(ternaryOperatorExpression->leftExpression, Options(0, options.whitespaces), code);

            if (options.whitespaces) code += " ";
            code += ":";
            if (options.whitespaces) code += " ";

            printConstruct(ternaryOperatorExpression->rightExpression, Options(0, options.whitespaces), code);
            break;
        }

        case Expression::Kind::TEMPORARY_OBJECT:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const TemporaryObjectExpression* temporaryObjectExpression = static_cast<const TemporaryObjectExpression*>(expression);

            const TypeDeclaration* typeDeclaration = static_cast<const TypeDeclaration*>(temporaryObjectExpression->constructorDeclaration->parent);

            if (typeDeclaration->getTypeKind() != TypeDeclaration::Kind::STRUCT)
                throw std::runtime_error("Temporary object must be a struct");

            const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(typeDeclaration);

            code += structDeclaration->name + "(";

            bool firstParameter = true;

            for (Expression* parameter : temporaryObjectExpression->parameters)
            {
                if (!firstParameter)
                {
                    code += ",";
                    if (options.whitespaces) code += "\n";
                    firstParameter = false;
                }

                printConstruct(parameter, Options(0, options.whitespaces), code);
            }

            code += ")";

            break;
        }

        case Expression::Kind::INITIALIZER_LIST:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const InitializerListExpression* initializerListExpression = static_cast<const InitializerListExpression*>(expression);

            code += "{";

            bool firstExpression = true;

            for (Expression* subExpression : initializerListExpression->expressions)
            {
                if (!firstExpression)
                {
                    code += ",";
                    if (options.whitespaces) code += " ";
                    firstExpression = false;
                }

                printConstruct(subExpression, Options(0, options.whitespaces), code);
            }

            code += "}";

            break;
        }

        case Expression::Kind::CAST:
        {
            if (options.whitespaces) code.append(options.indentation, ' ');

            const CastExpression* castExpression = static_cast<const CastExpression*>(expression);

            if (castExpression->getCastKind() != CastExpression::Kind::IMPLICIT)
            {
                code += castExpression->qualifiedType.typeDeclaration->name + "(";

                printConstruct(castExpression->expression, Options(0, options.whitespaces), code);

                code += ")";
            }
            else
            {
                printConstruct(castExpression->expression, Options(0, options.whitespaces), code);
            }
            break;
        }
        case Expression::Kind::SIZEOF:
        {
            // TODO: implement
            break;
        }
    }
}

void OutputMSL::printConstruct(const Construct* construct, Options options, std::string& code)
{
    switch (construct->getKind())
    {
        case Construct::Kind::Declaration:
        {
            const Declaration* declaration = static_cast<const Declaration*>(construct);
            printDeclaration(declaration, options, code);
            break;
        }

        case Construct::Kind::Statement:
        {
            const Statement* statement = static_cast<const Statement*>(construct);
            printStatement(statement, options, code);
            break;
        }

        case Construct::Kind::Expression:
        {
            const Expression* expression = static_cast<const Expression*>(construct);
            printExpression(expression, options, code);
            break;
        }
    }
}

