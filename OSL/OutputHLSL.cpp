//
//  OSL
//

#include <fstream>
#include <iostream>
#include "OutputHLSL.hpp"

bool OutputHLSL::output(const ASTContext& context, const std::string& outputFile)
{
    std::string code;

    std::ofstream file(outputFile, std::ios::binary);

    if (!file)
    {
        std::cerr << "Failed to open file " << outputFile << std::endl;
        return EXIT_FAILURE;
    }

    for (Declaration* declaration : context.declarations)
    {
        if (!printConstruct(declaration, Options(0), code))
        {
            return false;
        }

        // TODO: remove
        code += "\n";
    }

    file << code;

    return true;
}

bool OutputHLSL::printDeclaration(const Declaration* declaration, Options options, std::string& code)
{
    switch (declaration->declarationKind)
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

        case Declaration::Kind::STRUCT:
        {
            code.append(options.indentation, ' ');

            const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(declaration);
            code += "struct " + structDeclaration->type->name;

            if (!structDeclaration->fieldDeclarations.empty())
            {
                code.append(options.indentation, ' ');
                code += "\n{\n";

                for (const FieldDeclaration* fieldDeclaration : structDeclaration->fieldDeclarations)
                {
                    if (!printConstruct(fieldDeclaration, Options(options.indentation + 4), code))
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
            code += fieldDeclaration->qualifiedType.type->name + " " + fieldDeclaration->name;
            break;
        }

        case Declaration::Kind::FUNCTION:
        {
            code.append(options.indentation, ' ');

            const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(declaration);

            code += functionDeclaration->qualifiedType.type->name + " " + functionDeclaration->name + "(";

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

            if (functionDeclaration->body)
            {
                code += ")\n";

                if (!printConstruct(functionDeclaration->body, options, code))
                {
                    return false;
                }
            }
            else
            {
                code += ");"; // does not have a definition
            }

            break;
        }

        case Declaration::Kind::VARIABLE:
        {
            code.append(options.indentation, ' ');

            const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(declaration);
            code += variableDeclaration->qualifiedType.type->name + " " + variableDeclaration->name;

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
            code += parameterDeclaration->qualifiedType.type->name + " " + parameterDeclaration->name;
            break;
        }
    }

    return true;
}

bool OutputHLSL::printStatement(const Statement* statement, Options options, std::string& code)
{
    switch (statement->statementKind)
    {
        case Statement::Kind::NONE:
        {
            break;
        }

        case Statement::Kind::EMPTY:
        {
            code.append(options.indentation, ' ');
            code += ";\n";
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
            code += ";\n";
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
            code += ";\n";
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
            }

            code.append(options.indentation, ' ');
            code += "}\n";
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

            if (ifStatement->body->statementKind == Statement::Kind::COMPOUND)
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
                code.append(options.indentation, ' ');
                code += "else\n";

                if (ifStatement->elseBody->statementKind == Statement::Kind::COMPOUND)
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

            if (forStatement->body->statementKind == Statement::Kind::COMPOUND)
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

            if (switchStatement->body->statementKind == Statement::Kind::COMPOUND)
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

            if (caseStatement->body->statementKind == Statement::Kind::COMPOUND)
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

                code += ";\n";
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

            if (whileStatement->body->statementKind == Statement::Kind::COMPOUND)
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

            if (doStatement->body->statementKind == Statement::Kind::COMPOUND)
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

                code += ";";
            }

            code.append(options.indentation, ' ');
            code += "while (";

            if (!printConstruct(doStatement->condition, Options(0), code))
            {
                return false;
            }

            code += ");\n";

            break;
        }

        case Statement::Kind::BREAK:
        {
            code.append(options.indentation, ' ');
            code += "break;\n";
            break;
        }

        case Statement::Kind::CONTINUE:
        {
            code.append(options.indentation, ' ');
            code += "continue;\n";
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
                
                code += ";\n";
            }
            
            break;
        }
    }

    return true;
}

bool OutputHLSL::printExpression(const Expression* expression, Options options, std::string& code)
{
    switch (expression->expressionKind)
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

            code += expression->value;
            break;
        }

        case Expression::Kind::DECLARATION_REFERENCE:
        {
            code.append(options.indentation, ' ');

            const DeclarationReferenceExpression* declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(expression);
            Declaration* declaration = declarationReferenceExpression->declaration;

            switch (declaration->declarationKind)
            {
                case Declaration::Kind::FUNCTION:
                {
                    const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(declaration);
                    code += functionDeclaration->name;
                    break;
                }
                case Declaration::Kind::VARIABLE:
                {
                    const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(declaration);
                    code += variableDeclaration->name;
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

            if (!printConstruct(arraySubscriptExpression->declarationReference, Options(0), code))
            {
                return false;
            }

            code += "[";

            if (!printConstruct(arraySubscriptExpression->expression, Options(0), code))
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
            code += unaryOperatorExpression->value;

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

            code += " " + binaryOperatorExpression->value + " ";

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
    }

    return true;
}

bool OutputHLSL::printConstruct(const Construct* construct, Options options, std::string& code)
{
    switch (construct->kind)
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
