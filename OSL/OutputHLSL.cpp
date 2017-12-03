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

bool OutputHLSL::printConstruct(const Construct* construct, Options options, std::string& code)
{
    switch (construct->kind)
    {
        case Construct::Kind::NONE:
        {
            break;
        }

        case Construct::Kind::DECLARATION_EMPTY:
        {
            code.append(options.indentation, ' ');
            break;
        }

        case Construct::Kind::DECLARATION_STRUCT:
        {
            code.append(options.indentation, ' ');

            const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(construct);
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

        case Construct::Kind::DECLARATION_FIELD:
        {
            code.append(options.indentation, ' ');

            const FieldDeclaration* fieldDeclaration = static_cast<const FieldDeclaration*>(construct);
            code += fieldDeclaration->field->qualifiedType.type->name + " " + fieldDeclaration->field->name;
            break;
        }

        case Construct::Kind::DECLARATION_FUNCTION:
        {
            code.append(options.indentation, ' ');

            const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(construct);

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

        case Construct::Kind::DECLARATION_VARIABLE:
        {
            code.append(options.indentation, ' ');

            const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(construct);
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

        case Construct::Kind::DECLARATION_PARAMETER:
        {
            code.append(options.indentation, ' ');

            const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(construct);
            code += parameterDeclaration->qualifiedType.type->name + " " + parameterDeclaration->name;
            break;
        }

        case Construct::Kind::EXPRESSION_CALL:
        {
            code.append(options.indentation, ' ');

            const CallExpression* callExpression = static_cast<const CallExpression*>(construct);

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

        case Construct::Kind::EXPRESSION_LITERAL:
        {
            code.append(options.indentation, ' ');

            const Expression* expression = static_cast<const Expression*>(construct);
            code += expression->value;
            break;
        }

        case Construct::Kind::EXPRESSION_DECLARATION_REFERENCE:
        {
            code.append(options.indentation, ' ');

            const DeclarationReferenceExpression* declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(construct);
            Declaration* declaration = declarationReferenceExpression->declaration;

            switch (declaration->kind)
            {
                case Construct::Kind::DECLARATION_FUNCTION:
                {
                    const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(declaration);
                    code += functionDeclaration->name;
                    break;
                }
                case Construct::Kind::DECLARATION_VARIABLE:
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

        case Construct::Kind::EXPRESSION_PAREN:
        {
            code.append(options.indentation, ' ');

            const ParenExpression* parenExpression = static_cast<const ParenExpression*>(construct);
            code += "(";

            if (!printConstruct(parenExpression->expression, Options(0), code))
            {
                return false;
            }

            code += ")";
            break;
        }

        case Construct::Kind::EXPRESSION_MEMBER:
        {
            code.append(options.indentation, ' ');

            const MemberExpression* memberExpression = static_cast<const MemberExpression*>(construct);

            if (!printConstruct(memberExpression->expression, Options(0), code))
            {
                return false;
            }

            code += ".";

            if (!memberExpression->field)
            {
                std::cerr << "Field does not exist";
                return false;
            }

            code += memberExpression->field->name;

            break;
        }

        case Construct::Kind::EXPRESSION_ARRAY_SUBSCRIPT:
        {
            code.append(options.indentation, ' ');

            const ArraySubscriptExpression* arraySubscriptExpression = static_cast<const ArraySubscriptExpression*>(construct);

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

        case Construct::Kind::STATEMENT_EMPTY:
        {
            code.append(options.indentation, ' ');
            code += ";\n";
            break;
        }

        case Construct::Kind::STATEMENT_EXPRESSION:
        {
            code.append(options.indentation, ' ');

            const ExpressionStatement* expressionStatement = static_cast<const ExpressionStatement*>(construct);
            if (!printConstruct(expressionStatement->expression, Options(0), code))
            {
                return false;
            }
            code += ";\n";
            break;
        }

        case Construct::Kind::STATEMENT_DECLARATION:
        {
            code.append(options.indentation, ' ');

            const DeclarationStatement* declarationStatement = static_cast<const DeclarationStatement*>(construct);
            if (!printConstruct(declarationStatement->declaration, Options(0), code))
            {
                return false;
            }
            code += ";\n";
            break;
        }

        case Construct::Kind::STATEMENT_COMPOUND:
        {
            const CompoundStatement* compoundStatement = static_cast<const CompoundStatement*>(construct);

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

        case Construct::Kind::STATEMENT_IF:
        {
            code.append(options.indentation, ' ');

            const IfStatement* ifStatement = static_cast<const IfStatement*>(construct);
            code += "if (";

            if (!printConstruct(ifStatement->condition, Options(0), code))
            {
                return false;
            }

            code += ")\n";

            if (ifStatement->body->kind == Construct::Kind::STATEMENT_COMPOUND)
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
                
                if (ifStatement->elseBody->kind == Construct::Kind::STATEMENT_COMPOUND)
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

        case Construct::Kind::STATEMENT_FOR:
        {
            code.append(options.indentation, ' ');

            const ForStatement* forStatement = static_cast<const ForStatement*>(construct);
            code += "for (";

            if (!printConstruct(forStatement->initialization, Options(0), code))
            {
                return false;
            }

            code += "; ";

            if (!printConstruct(forStatement->condition, Options(0), code))
            {
                return false;
            }

            code += "; ";

            if (!printConstruct(forStatement->increment, Options(0), code))
            {
                return false;
            }

            code += ")\n";

            if (forStatement->body->kind == Construct::Kind::STATEMENT_COMPOUND)
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

        case Construct::Kind::STATEMENT_SWITCH:
        {
            code.append(options.indentation, ' ');

            const SwitchStatement* switchStatement = static_cast<const SwitchStatement*>(construct);
            code += "switch (";

            if (!printConstruct(switchStatement->condition, Options(0), code))
            {
                return false;
            }

            code += ")\n";

            if (switchStatement->body->kind == Construct::Kind::STATEMENT_COMPOUND)
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

        case Construct::Kind::STATEMENT_CASE:
        {
            code.append(options.indentation, ' ');

            const CaseStatement* caseStatement = static_cast<const CaseStatement*>(construct);
            code += "case ";

            if (!printConstruct(caseStatement->condition, Options(0), code))
            {
                return false;
            }

            code += ":\n";

            if (caseStatement->body->kind == Construct::Kind::STATEMENT_COMPOUND)
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

        case Construct::Kind::STATEMENT_WHILE:
        {
            code.append(options.indentation, ' ');

            const WhileStatement* whileStatement = static_cast<const WhileStatement*>(construct);
            code += "while (";

            if (!printConstruct(whileStatement->condition, Options(0), code))
            {
                return false;
            }

            code += ")\n";

            if (whileStatement->body->kind == Construct::Kind::STATEMENT_COMPOUND)
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

        case Construct::Kind::STATEMENT_DO:
        {
            code.append(options.indentation, ' ');

            const DoStatement* doStatement = static_cast<const DoStatement*>(construct);
            code += "do\n";

            if (doStatement->body->kind == Construct::Kind::STATEMENT_COMPOUND)
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

        case Construct::Kind::STATEMENT_BREAK:
        {
            code.append(options.indentation, ' ');
            code += "break;\n";
            break;
        }

        case Construct::Kind::STATEMENT_CONTINUE:
        {
            code.append(options.indentation, ' ');
            code += "continue;\n";
            break;
        }

        case Construct::Kind::STATEMENT_RETURN:
        {
            code.append(options.indentation, ' ');

            const ReturnStatement* returnStatement = static_cast<const ReturnStatement*>(construct);
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

        case Construct::Kind::OPERATOR_UNARY:
        {
            code.append(options.indentation, ' ');

            const UnaryOperatorExpression* unaryOperatorExpression = static_cast<const UnaryOperatorExpression*>(construct);
            code += unaryOperatorExpression->value;

            if (!printConstruct(unaryOperatorExpression->expression, Options(0), code))
            {
                return false;
            }
            break;
        }

        case Construct::Kind::OPERATOR_BINARY:
        {
            code.append(options.indentation, ' ');

            const BinaryOperatorExpression* binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(construct);
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

        case Construct::Kind::OPERATOR_TERNARY:
        {
            code.append(options.indentation, ' ');

            const TernaryOperatorExpression* ternaryOperatorExpression = static_cast<const TernaryOperatorExpression*>(construct);

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
