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

    if (!printConstruct(context.getTranslationUnit(), Options(0), code))
    {
        std::cerr << "Failed to print code" << std::endl;
        return EXIT_FAILURE;
    }

    file << code;

    return true;
}

bool OutputHLSL::printConstruct(const Construct* node, Options options, std::string& code)
{
    switch (node->kind)
    {
        case Construct::Kind::NONE:
        {
            break;
        }

        case Construct::Kind::TYPE_SIMPLE:
        {
            break;
        }

        case Construct::Kind::TYPE_STRUCT:
        {
            break;
        }

        case Construct::Kind::FIELD:
        {
            const Field* field = static_cast<const Field*>(node);
            code.append(options.indentation, ' ');
            code += field->qualifiedType.type->name + " " + field->name;
            break;
        }

        case Construct::Kind::TRANSLATION_UNIT:
        {
            const TranslationUnit* translationUnit = static_cast<const TranslationUnit*>(node);

            for (Declaration* declaration : translationUnit->declarations)
            {
                if (!printConstruct(declaration, options, code))
                {
                    return false;
                }

                code += ";\n";
            }
            break;
        }

        case Construct::Kind::DECLARATION_EMPTY:
        {
            code.append(options.indentation, ' ');
            break;
        }

        case Construct::Kind::DECLARATION_STRUCT:
        {
            const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(node);
            code.append(options.indentation, ' ');
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
            const FieldDeclaration* fieldDeclaration = static_cast<const FieldDeclaration*>(node);
            code.append(options.indentation, ' ');
            code += fieldDeclaration->field->qualifiedType.type->name + " " + fieldDeclaration->field->name;
            break;
        }

        case Construct::Kind::DECLARATION_FUNCTION:
        {
            const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(node);

            code.append(options.indentation, ' ');
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
                code += ")"; // does not have a definition
            }

            break;
        }

        case Construct::Kind::DECLARATION_VARIABLE:
        {
            const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(node);
            code.append(options.indentation, ' ');
            code += variableDeclaration->qualifiedType.type->name + " " + variableDeclaration->name;
            break;
        }

        case Construct::Kind::DECLARATION_PARAMETER:
        {
            const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(node);
            code.append(options.indentation, ' ');
            code += parameterDeclaration->qualifiedType.type->name + " " + parameterDeclaration->name;
            break;
        }

        case Construct::Kind::EXPRESSION_CALL:
        {
            const CallExpression* callExpression = static_cast<const CallExpression*>(node);

            if (!printConstruct(callExpression->declarationReference, options, code))
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
            const Expression* expression = static_cast<const Expression*>(node);
            code.append(options.indentation, ' ');
            code += expression->value;
            break;
        }

        case Construct::Kind::EXPRESSION_DECLARATION_REFERENCE:
        {
            const DeclarationReferenceExpression* declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(node);
            Declaration* declaration = declarationReferenceExpression->declaration;

            if (!printConstruct(declaration, options, code))
            {
                return false;
            }
            break;
        }

        case Construct::Kind::EXPRESSION_PAREN:
        {
            const ParenExpression* parenExpression = static_cast<const ParenExpression*>(node);

            code.append(options.indentation, ' ');
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
            const MemberExpression* memberExpression = static_cast<const MemberExpression*>(node);

            if (!printConstruct(memberExpression->expression, options, code))
            {
                return false;
            }

            code += ".";

            if (!printConstruct(memberExpression->field, Options(0), code))
            {
                return false;
            }
            break;
        }

        case Construct::Kind::EXPRESSION_ARRAY_SUBSCRIPT:
        {
            const ArraySubscriptExpression* arraySubscriptExpression = static_cast<const ArraySubscriptExpression*>(node);

            if (!printConstruct(arraySubscriptExpression->declarationReference, options, code))
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
            const ExpressionStatement* expressionStatement = static_cast<const ExpressionStatement*>(node);
            if (!printConstruct(expressionStatement->expression, options, code))
            {
                return false;
            }
            code += ";\n";
            break;
        }

        case Construct::Kind::STATEMENT_DECLARATION:
        {
            const DeclarationStatement* declarationStatement = static_cast<const DeclarationStatement*>(node);
            if (!printConstruct(declarationStatement->declaration, options, code))
            {
                return false;
            }
            code += ";\n";
            break;
        }

        case Construct::Kind::STATEMENT_COMPOUND:
        {
            const CompoundStatement* compoundStatement = static_cast<const CompoundStatement*>(node);

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
            const IfStatement* ifStatement = static_cast<const IfStatement*>(node);
            
            code.append(options.indentation, ' ');
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
            const ForStatement* forStatement = static_cast<const ForStatement*>(node);

            code.append(options.indentation, ' ');
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
            const SwitchStatement* switchStatement = static_cast<const SwitchStatement*>(node);

            code.append(options.indentation, ' ');
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
            const CaseStatement* caseStatement = static_cast<const CaseStatement*>(node);

            code.append(options.indentation, ' ');
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
            const WhileStatement* whileStatement = static_cast<const WhileStatement*>(node);

            code.append(options.indentation, ' ');
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
            const DoStatement* doStatement = static_cast<const DoStatement*>(node);

            code.append(options.indentation, ' ');
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
            const ReturnStatement* returnStatement = static_cast<const ReturnStatement*>(node);

            code.append(options.indentation, ' ');
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
            const UnaryOperatorExpression* unaryOperatorExpression = static_cast<const UnaryOperatorExpression*>(node);

            code.append(options.indentation, ' ');
            code += unaryOperatorExpression->value;

            if (!printConstruct(unaryOperatorExpression->expression, Options(0), code))
            {
                return false;
            }
            break;
        }

        case Construct::Kind::OPERATOR_BINARY:
        {
            const BinaryOperatorExpression* binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(node);

            if (!printConstruct(binaryOperatorExpression->leftExpression, options, code))
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
            const TernaryOperatorExpression* ternaryOperatorExpression = static_cast<const TernaryOperatorExpression*>(node);

            if (!printConstruct(ternaryOperatorExpression->condition, options, code))
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
