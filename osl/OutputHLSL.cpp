//
//  OSL
//

#include "OutputHLSL.hpp"

namespace ouzel
{
    namespace
    {
        struct BuiltinFunction
        {
            BuiltinFunction(const std::string& initName): name(initName) {}
            std::string name;
        };

        std::map<std::string, BuiltinFunction> builtinFunctions;
    }

    OutputHLSL::OutputHLSL(Program initProgram, const std::string& initMainFunction):
        Output(initProgram, initMainFunction)
    {
    }

    std::string OutputHLSL::output(const ASTContext& context, bool whitespaces)
    {
        std::string result;

        for (auto declaration : context.getDeclarations())
        {
            printConstruct(declaration, Options(0, whitespaces), result);

            if (declaration->getDeclarationKind() != Declaration::Kind::Callable ||
                !static_cast<const CallableDeclaration*>(declaration)->body) // function doesn't have a body
                result += ";";

            if (whitespaces) result += "\n";
        }

        return result;
    }

    namespace
    {
        std::pair<std::string, std::string> getPrintableTypeName(const QualifiedType& qualifiedType)
        {
            std::pair<std::string, std::string> result;

            if ((qualifiedType.qualifiers & Qualifiers::Volatile) == Qualifiers::Volatile) result.first += "volatile ";
            if ((qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const) result.first += "const ";

            if (!qualifiedType.type)
            {
                result.first += "void";
            }
            else
            {
                auto type = qualifiedType.type;
                while (type->getTypeKind() == Type::Kind::Array)
                {
                    auto arrayType = static_cast<const ArrayType*>(type);

                    result.second = "[" + std::to_string(arrayType->size) + "]" + result.second;

                    type = arrayType->elementType.type;
                }

                result.first = type->name;
            }

            return result;
        }
    }

    void OutputHLSL::printDeclaration(const Declaration* declaration, Options options, std::string& code)
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

                auto typeDeclaration = static_cast<const TypeDeclaration*>(declaration);
                auto type = typeDeclaration->type;

                if (type->getTypeKind() != Type::Kind::Struct)
                    throw std::runtime_error("Type declaration must be a struct");

                auto structType = static_cast<const StructType*>(type);
                code += "struct " + type->name;

                // if this is the definition
                if (typeDeclaration->definition == typeDeclaration)
                {
                    if (options.whitespaces) code.append(options.indentation, ' ');
                    if (options.whitespaces) code += "\n";
                    code += "{";
                    if (options.whitespaces) code += "\n";

                    for (auto memberDeclaration : structType->memberDeclarations)
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

                auto fieldDeclaration = static_cast<const FieldDeclaration*>(declaration);

                std::pair<std::string, std::string> printableTypeName = getPrintableTypeName(fieldDeclaration->qualifiedType);

                code += printableTypeName.first + " " + fieldDeclaration->name + printableTypeName.second;

                // TODO: print semantics

                break;
            }

            case Declaration::Kind::Callable:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto callableDeclaration = static_cast<const CallableDeclaration*>(declaration);

                if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::Function)
                {
                    auto functionDeclaration = static_cast<const FunctionDeclaration*>(callableDeclaration);

                    if (functionDeclaration->isInline) code += "inline ";

                    std::pair<std::string, std::string> printableTypeName = getPrintableTypeName(functionDeclaration->qualifiedType);

                    code += printableTypeName.first + " " + functionDeclaration->name + "(";

                    bool firstParameter = true;

                    for (auto parameter : functionDeclaration->parameterDeclarations)
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

                auto variableDeclaration = static_cast<const VariableDeclaration*>(declaration);

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

                auto parameterDeclaration = static_cast<const ParameterDeclaration*>(declaration);

                std::pair<std::string, std::string> printableTypeName = getPrintableTypeName(parameterDeclaration->qualifiedType);

                code += printableTypeName.first + " " + parameterDeclaration->name + printableTypeName.second;
                break;
            }
        }
    }

    void OutputHLSL::printStatement(const Statement* statement, Options options, std::string& code)
    {
        switch (statement->getStatementKind())
        {
            case Statement::Kind::Empty:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');
                code += ";";
                break;
            }

            case Statement::Kind::Expression:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto expressionStatement = static_cast<const ExpressionStatement*>(statement);
                printConstruct(expressionStatement->expression, Options(0, options.whitespaces), code);

                code += ";";
                break;
            }

            case Statement::Kind::Declaration:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto declarationStatement = static_cast<const DeclarationStatement*>(statement);
                printConstruct(declarationStatement->declaration, Options(0, options.whitespaces), code);

                code += ";";
                break;
            }

            case Statement::Kind::Compound:
            {
                auto compoundStatement = static_cast<const CompoundStatement*>(statement);

                if (options.whitespaces) code.append(options.indentation, ' ');
                code += "{";
                if (options.whitespaces) code += "\n";

                for (auto subStatement : compoundStatement->statements)
                {
                    printConstruct(subStatement, Options(options.indentation + 4, options.whitespaces), code);
                    if (options.whitespaces) code += "\n";
                }

                if (options.whitespaces) code.append(options.indentation, ' ');
                code += "}";
                break;
            }

            case Statement::Kind::If:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto ifStatement = static_cast<const IfStatement*>(statement);
                code += "if";
                if (options.whitespaces) code += " ";
                code += "(";

                printConstruct(ifStatement->condition, Options(0, options.whitespaces), code);

                code += ")";
                if (options.whitespaces) code += "\n";

                if (ifStatement->body->getStatementKind() == Statement::Kind::Compound)
                    printConstruct(ifStatement->body, options, code);
                else
                    printConstruct(ifStatement->body, Options(options.indentation + 4, options.whitespaces), code);

                if (ifStatement->elseBody)
                {
                    if (options.whitespaces) code += "\n";
                    if (options.whitespaces) code.append(options.indentation, ' ');
                    code += "else";
                    if (options.whitespaces) code += "\n";

                    if (ifStatement->elseBody->getStatementKind() == Statement::Kind::Compound)
                        printConstruct(ifStatement->elseBody, options, code);
                    else
                        printConstruct(ifStatement->elseBody, Options(options.indentation + 4, options.whitespaces), code);
                }
                break;
            }

            case Statement::Kind::For:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto forStatement = static_cast<const ForStatement*>(statement);
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

                if (forStatement->body->getStatementKind() == Statement::Kind::Compound)
                    printConstruct(forStatement->body, options, code);
                else
                    printConstruct(forStatement->body, Options(options.indentation + 4, options.whitespaces), code);
                break;
            }

            case Statement::Kind::Switch:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto switchStatement = static_cast<const SwitchStatement*>(statement);
                code += "switch";
                if (options.whitespaces) code += " ";
                code += "(";

                printConstruct(switchStatement->condition, Options(0, options.whitespaces), code);

                code += ")";
                if (options.whitespaces) code += "\n";

                if (switchStatement->body->getStatementKind() == Statement::Kind::Compound)
                    printConstruct(switchStatement->body, options, code);
                else
                    printConstruct(switchStatement->body, Options(options.indentation + 4, options.whitespaces), code);

                break;
            }

            case Statement::Kind::Case:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto caseStatement = static_cast<const CaseStatement*>(statement);
                code += "case ";

                printConstruct(caseStatement->condition, Options(0, options.whitespaces), code);

                code += ":";
                if (options.whitespaces) code += "\n";

                if (caseStatement->body->getStatementKind() == Statement::Kind::Compound)
                    printConstruct(caseStatement->body, options, code);
                else
                    printConstruct(caseStatement->body, Options(options.indentation + 4, options.whitespaces), code);

                break;
            }

            case Statement::Kind::Default:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto defaultStatement = static_cast<const DefaultStatement*>(statement);
                code += "default:";
                if (options.whitespaces) code += "\n";

                if (defaultStatement->body->getStatementKind() == Statement::Kind::Compound)
                    printConstruct(defaultStatement->body, options, code);
                else
                    printConstruct(defaultStatement->body, Options(options.indentation + 4, options.whitespaces), code);

                break;
            }

            case Statement::Kind::While:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto whileStatement = static_cast<const WhileStatement*>(statement);
                code += "while";
                if (options.whitespaces) code += " ";
                code += "(";

                printConstruct(whileStatement->condition, Options(0, options.whitespaces), code);

                code += ")";
                if (options.whitespaces) code += "\n";

                if (whileStatement->body->getStatementKind() == Statement::Kind::Compound)
                    printConstruct(whileStatement->body, options, code);
                else
                    printConstruct(whileStatement->body, Options(options.indentation + 4, options.whitespaces), code);
                break;
            }

            case Statement::Kind::Do:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto doStatement = static_cast<const DoStatement*>(statement);
                code += "do";
                if (options.whitespaces) code += "\n";

                if (doStatement->body->getStatementKind() == Statement::Kind::Compound)
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

            case Statement::Kind::Break:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');
                code += "break;";
                break;
            }

            case Statement::Kind::Continue:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');
                code += "continue;";
                break;
            }

            case Statement::Kind::Return:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto returnStatement = static_cast<const ReturnStatement*>(statement);
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

    void OutputHLSL::printExpression(const Expression* expression, Options options, std::string& code)
    {
        switch (expression->getExpressionKind())
        {
            case Expression::Kind::Call:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto callExpression = static_cast<const CallExpression*>(expression);

                printConstruct(callExpression->declarationReference, Options(0, options.whitespaces), code);

                code += "(";

                bool firstParameter = true;

                for (auto argument : callExpression->arguments)
                {
                    if (!firstParameter)
                    {
                        code += ",";
                        if (options.whitespaces) code += " ";
                        firstParameter = false;
                    }

                    printConstruct(argument, Options(0, options.whitespaces), code);
                }

                code += ")";
                break;
            }

            case Expression::Kind::Literal:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto literalExpression = static_cast<const LiteralExpression*>(expression);

                switch (literalExpression->getLiteralKind())
                {
                    case LiteralExpression::Kind::Boolean:
                    {
                        auto booleanLiteralExpression = static_cast<const BooleanLiteralExpression*>(literalExpression);
                        code += (booleanLiteralExpression->value ? "true" : "false");
                        break;
                    }
                    case LiteralExpression::Kind::Integer:
                    {
                        auto integerLiteralExpression = static_cast<const IntegerLiteralExpression*>(literalExpression);
                        code += std::to_string(integerLiteralExpression->value);
                        break;
                    }
                    case LiteralExpression::Kind::FloatingPoint:
                    {
                        auto floatingPointLiteralExpression = static_cast<const FloatingPointLiteralExpression*>(literalExpression);
                        code += std::to_string(floatingPointLiteralExpression->value);
                        break;
                    }
                    case LiteralExpression::Kind::String:
                    {
                        auto stringLiteralExpression = static_cast<const StringLiteralExpression*>(literalExpression);
                        code += stringLiteralExpression->value;
                        break;
                    }
                }
                break;
            }

            case Expression::Kind::DeclarationReference:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(expression);
                auto declaration = declarationReferenceExpression->declaration;

                switch (declaration->getDeclarationKind())
                {
                    case Declaration::Kind::Callable:
                    {
                        auto callableDeclaration = static_cast<const CallableDeclaration*>(declaration);

                        if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::Function)
                        {
                            auto functionDeclaration = static_cast<const FunctionDeclaration*>(callableDeclaration);
                            code += functionDeclaration->name;
                        }
                        break;
                    }
                    case Declaration::Kind::Variable:
                    {
                        auto variableDeclaration = static_cast<const VariableDeclaration*>(declaration);
                        code += variableDeclaration->name;
                        break;
                    }
                    case Declaration::Kind::Parameter:
                    {
                        auto parameterDeclaration = static_cast<const ParameterDeclaration*>(declaration);
                        code += parameterDeclaration->name;
                        break;
                    }
                    default:
                        throw std::runtime_error("Unknown declaration type");
                }

                break;
            }

            case Expression::Kind::Paren:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto parenExpression = static_cast<const ParenExpression*>(expression);
                code += "(";

                printConstruct(parenExpression->expression, Options(0, options.whitespaces), code);

                code += ")";
                break;
            }

            case Expression::Kind::Member:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto memberExpression = static_cast<const MemberExpression*>(expression);

                printConstruct(memberExpression->expression, Options(0, options.whitespaces), code);

                code += ".";

                if (!memberExpression->fieldDeclaration)
                    throw std::runtime_error("Field does not exist");

                code += memberExpression->fieldDeclaration->name;

                break;
            }

            case Expression::Kind::ArraySubscript:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto arraySubscriptExpression = static_cast<const ArraySubscriptExpression*>(expression);

                printConstruct(arraySubscriptExpression->expression, Options(0, options.whitespaces), code);

                code += "[";

                printConstruct(arraySubscriptExpression->subscript, Options(0, options.whitespaces), code);

                code += "]";

                break;
            }

            case Expression::Kind::UnaryOperator:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto unaryOperatorExpression = static_cast<const UnaryOperatorExpression*>(expression);

                switch (unaryOperatorExpression->getOperatorKind())
                {
                    case UnaryOperatorExpression::Kind::Negation: code += "!"; break;
                    case UnaryOperatorExpression::Kind::Positive: code += "+"; break;
                    case UnaryOperatorExpression::Kind::Negative: code += "-"; break;
                    default:
                        throw std::runtime_error("Unknown operator");
                }

                printConstruct(unaryOperatorExpression->expression, Options(0, options.whitespaces), code);
                break;
            }

            case Expression::Kind::BinaryOperator:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(expression);
                printConstruct(binaryOperatorExpression->leftExpression, Options(0, options.whitespaces), code);

                if (options.whitespaces &&
                    binaryOperatorExpression->getOperatorKind() != BinaryOperatorExpression::Kind::Comma) code += " ";

                switch (binaryOperatorExpression->getOperatorKind())
                {
                    case BinaryOperatorExpression::Kind::Addition: code += "+"; break;
                    case BinaryOperatorExpression::Kind::Subtraction: code += "-"; break;
                    case BinaryOperatorExpression::Kind::Multiplication: code += "*"; break;
                    case BinaryOperatorExpression::Kind::Division: code += "/"; break;
                    case BinaryOperatorExpression::Kind::AdditionAssignment: code += "+="; break;
                    case BinaryOperatorExpression::Kind::SubtractAssignment: code += "-="; break;
                    case BinaryOperatorExpression::Kind::MultiplicationAssignment: code += "*="; break;
                    case BinaryOperatorExpression::Kind::DivisionAssignment: code += "/="; break;
                    case BinaryOperatorExpression::Kind::LessThan: code += "<"; break;
                    case BinaryOperatorExpression::Kind::LessThanEqual: code += "<="; break;
                    case BinaryOperatorExpression::Kind::GreaterThan: code += ">"; break;
                    case BinaryOperatorExpression::Kind::GraterThanEqual: code += ">="; break;
                    case BinaryOperatorExpression::Kind::Equality: code += "=="; break;
                    case BinaryOperatorExpression::Kind::Inequality: code += "!="; break;
                    case BinaryOperatorExpression::Kind::Assignment: code += "="; break;
                    case BinaryOperatorExpression::Kind::Or: code += "||"; break;
                    case BinaryOperatorExpression::Kind::And: code += "&&"; break;
                    case BinaryOperatorExpression::Kind::Comma: code += ","; break;
                    default:
                        throw std::runtime_error("Unknown operator");
                }

                if (options.whitespaces) code += " ";

                printConstruct(binaryOperatorExpression->rightExpression, Options(0, options.whitespaces), code);
                break;
            }

            case Expression::Kind::TernaryOperator:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto ternaryOperatorExpression = static_cast<const TernaryOperatorExpression*>(expression);

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

            case Expression::Kind::TemporaryObject:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto temporaryObjectExpression = static_cast<const TemporaryObjectExpression*>(expression);

                auto typeDeclaration = static_cast<const TypeDeclaration*>(temporaryObjectExpression->constructorDeclaration->parent);
                auto type = typeDeclaration->type;

                if (type->getTypeKind() != Type::Kind::Struct)
                    throw std::runtime_error("Temporary object must be a struct");

                auto structType = static_cast<const StructType*>(type);

                code += structType->name + "(";

                bool firstParameter = true;

                for (auto parameter : temporaryObjectExpression->parameters)
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

            case Expression::Kind::InitializerList:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto initializerListExpression = static_cast<const InitializerListExpression*>(expression);

                code += "{";

                bool firstExpression = true;

                for (auto subExpression : initializerListExpression->expressions)
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

            case Expression::Kind::Cast:
            {
                if (options.whitespaces) code.append(options.indentation, ' ');

                auto castExpression = static_cast<const CastExpression*>(expression);

                code += castExpression->qualifiedType.type->name + "(";
                printConstruct(castExpression->expression, Options(0, options.whitespaces), code);
                code += ")";

                break;
            }
            case Expression::Kind::Sizeof:
            {
                // TODO: implement
                break;
            }
        }
    }

    void OutputHLSL::printConstruct(const Construct* construct, Options options, std::string& code)
    {
        switch (construct->getKind())
        {
            case Construct::Kind::Declaration:
            {
                auto declaration = static_cast<const Declaration*>(construct);
                printDeclaration(declaration, options, code);
                break;
            }

            case Construct::Kind::Statement:
            {
                auto statement = static_cast<const Statement*>(construct);
                printStatement(statement, options, code);
                break;
            }

            case Construct::Kind::Expression:
            {
                auto expression = static_cast<const Expression*>(construct);
                printExpression(expression, options, code);
                break;
            }
        }
    }
}
