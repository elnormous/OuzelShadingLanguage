//
//  OutputHLSL.cpp
//  OSL
//
//  Created by Elviss Strazdins on 14/10/2017.
//  Copyright (c) 2017 Elviss Strazdins. All rights reserved.
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

    if (!printNode(context.getTranslationUnit(), "", code))
    {
        std::cerr << "Failed to print code" << std::endl;
        return EXIT_FAILURE;
    }

    file << code;

    return true;
}

bool OutputHLSL::printNode(const Construct* node, const std::string& prefix, std::string& code)
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
            break;
        }

        case Construct::Kind::TRANSLATION_UNIT:
        {
            const TranslationUnit* translationUnit = static_cast<const TranslationUnit*>(node);

            for (Declaration* declaration : translationUnit->declarations)
            {
                if (!printNode(declaration, prefix, code))
                {
                    return false;
                }

                code += "\n";
            }
            break;
        }

        case Construct::Kind::DECLARATION_EMPTY:
        {
            break;
        }

        case Construct::Kind::DECLARATION_STRUCT:
        {
            const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(node);
            code += prefix + "struct " + structDeclaration->type->name;

            if (structDeclaration->fieldDeclarations.empty())
            {
                code += ";";
            }
            else
            {
                code += prefix + "\n{\n";

                for (const FieldDeclaration* fieldDeclaration : structDeclaration->fieldDeclarations)
                {
                    if (!printNode(fieldDeclaration, prefix + "    ", code))
                    {
                        return false;
                    }

                    code += "\n";
                }

                code += prefix + "};";
            }

            break;
        }

        case Construct::Kind::DECLARATION_FIELD:
        {
            const FieldDeclaration* fieldDeclaration = static_cast<const FieldDeclaration*>(node);
            code += prefix + fieldDeclaration->field->type->name + " " + fieldDeclaration->field->name + ";";
            break;
        }

        case Construct::Kind::DECLARATION_FUNCTION:
        {
            const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(node);

            if (!node->reference)
            {
                std::cerr << "Invalid declaration reference" << std::endl;
                return false;
            }

            code += prefix + functionDeclaration->resultType->name + " " + functionDeclaration->name + "(";

            bool firstParameter = true;

            for (ParameterDeclaration* parameter : functionDeclaration->parameterDeclarations)
            {
                if (!firstParameter) code += ", ";
                firstParameter = false;

                if (!printNode(parameter, "", code))
                {
                    return false;
                }
            }

            if (functionDeclaration->body)
            {
                code += ")\n";

                if (!printNode(functionDeclaration->body, prefix, code))
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
            const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(node);
            code += prefix + variableDeclaration->type->name + " " + variableDeclaration->name + ";";
            break;
        }

        case Construct::Kind::DECLARATION_PARAMETER:
        {
            const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(node);
            code += prefix + parameterDeclaration->type->name + " " + parameterDeclaration->name;
            break;
        }

        case Construct::Kind::EXPRESSION_CALL:
        {
            const CallExpression* callExpression = static_cast<const CallExpression*>(node);

            if (!printNode(callExpression->declarationReference, prefix, code))
            {
                return false;
            }

            code += "(";

            bool firstParameter = true;

            for (Expression* parameter : callExpression->parameters)
            {
                if (!firstParameter) code += ", ";
                firstParameter = false;

                if (!printNode(parameter, "", code))
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
            code += prefix + expression->value;
            break;
        }

        case Construct::Kind::EXPRESSION_DECLARATION_REFERENCE:
        {
            const DeclarationReferenceExpression* declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(node);
            Declaration* declaration = declarationReferenceExpression->declaration;

            if (!printNode(declaration, prefix, code))
            {
                return false;
            }
            break;
        }

        case Construct::Kind::EXPRESSION_PAREN:
        {
            const ParenExpression* parenExpression = static_cast<const ParenExpression*>(node);

            code += prefix + "(";

            if (!printNode(parenExpression, "", code))
            {
                return false;
            }

            code += ")";
            break;
        }

        case Construct::Kind::EXPRESSION_MEMBER:
        {
            const MemberExpression* memberExpression = static_cast<const MemberExpression*>(node);

            if (!printNode(memberExpression->expression, prefix, code))
            {
                return false;
            }

            code += ".";

            if (!printNode(memberExpression->declarationReference, "", code))
            {
                return false;
            }
            break;
        }

        case Construct::Kind::EXPRESSION_ARRAY_SUBSCRIPT:
        {
            const ArraySubscriptExpression* arraySubscriptExpression = static_cast<const ArraySubscriptExpression*>(node);

            if (!printNode(arraySubscriptExpression->expression, "", code))
            {
                return false;
            }

            code += "[";

            if (!printNode(arraySubscriptExpression->declarationReference, "", code))
            {
                return false;
            }

            code += "]";
            
            break;
        }

        case Construct::Kind::STATEMENT_EMPTY:
        {
            break;
        }

        case Construct::Kind::STATEMENT_DECLARATION:
        {
            const DeclarationStatement* declarationStatement = static_cast<const DeclarationStatement*>(node);
            if (!printNode(declarationStatement->declaration, prefix, code))
            {
                return false;
            }
            break;
        }

        case Construct::Kind::STATEMENT_COMPOUND:
        {
            const CompoundStatement* compoundStatement = static_cast<const CompoundStatement*>(node);

            code += prefix + "{\n";

            for (Statement* statement : compoundStatement->statements)
            {
                if (!printNode(statement, prefix + "    ", code))
                {
                    return false;
                }

                code += ";\n";
            }

            code += prefix + "}";
            break;
        }

        case Construct::Kind::STATEMENT_IF:
        {
            const IfStatement* ifStatement = static_cast<const IfStatement*>(node);

            code += prefix + "if (";

            if (!printNode(ifStatement->condition, "", code))
            {
                return false;
            }

            code += ")\n";

            if (ifStatement->body->kind == Construct::Kind::STATEMENT_COMPOUND)
            {
                if (!printNode(ifStatement->body, prefix, code))
                {
                    return false;
                }
            }
            else
            {
                if (!printNode(ifStatement->body, prefix + "    ", code))
                {
                    return false;
                }

                code += ";\n";
            }

            if (ifStatement->elseBody)
            {
                if (ifStatement->elseBody->kind == Construct::Kind::STATEMENT_COMPOUND)
                {
                    if (!printNode(ifStatement->elseBody, prefix, code))
                    {
                        return false;
                    }
                }
                else
                {
                    if (!printNode(ifStatement->elseBody, prefix + "    ", code))
                    {
                        return false;
                    }

                    code += ";\n";
                }
            }
            break;
        }

        case Construct::Kind::STATEMENT_FOR:
        {
            const ForStatement* forStatement = static_cast<const ForStatement*>(node);

            code += prefix + "for (";

            if (!printNode(forStatement->initialization, "", code))
            {
                return false;
            }

            code += "; ";

            if (!printNode(forStatement->condition, "", code))
            {
                return false;
            }

            code += "; ";

            if (!printNode(forStatement->increment, "", code))
            {
                return false;
            }

            code += ")\n";

            if (forStatement->body->kind == Construct::Kind::STATEMENT_COMPOUND)
            {
                if (!printNode(forStatement->body, prefix, code))
                {
                    return false;
                }
            }
            else
            {
                if (!printNode(forStatement->body, prefix + "    ", code))
                {
                    return false;
                }

                code += ";\n";
            }
            break;
        }

        case Construct::Kind::STATEMENT_SWITCH:
        {
            const SwitchStatement* switchStatement = static_cast<const SwitchStatement*>(node);

            code += prefix + "switch (";

            if (!printNode(switchStatement->condition, "", code))
            {
                return false;
            }

            code += ")\n";

            if (switchStatement->body->kind == Construct::Kind::STATEMENT_COMPOUND)
            {
                if (!printNode(switchStatement->body, prefix, code))
                {
                    return false;
                }
            }
            else
            {
                if (!printNode(switchStatement->body, prefix + "    ", code))
                {
                    return false;
                }

                code += ";\n";
            }
            break;
        }

        case Construct::Kind::STATEMENT_CASE:
        {
            const CaseStatement* caseStatement = static_cast<const CaseStatement*>(node);

            code += prefix + "case ";

            if (!printNode(caseStatement->condition, "", code))
            {
                return false;
            }

            code += ":\n";

            if (caseStatement->body->kind == Construct::Kind::STATEMENT_COMPOUND)
            {
                if (!printNode(caseStatement->body, prefix, code))
                {
                    return false;
                }
            }
            else
            {
                if (!printNode(caseStatement->body, prefix + "    ", code))
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

            code += prefix + "while (";

            if (!printNode(whileStatement->condition, "", code))
            {
                return false;
            }

            code += ")\n";

            if (whileStatement->body->kind == Construct::Kind::STATEMENT_COMPOUND)
            {
                if (!printNode(whileStatement->body, prefix, code))
                {
                    return false;
                }
            }
            else
            {
                if (!printNode(whileStatement->body, prefix + "    ", code))
                {
                    return false;
                }

                code += ";\n";
            }
            break;
        }

        case Construct::Kind::STATEMENT_DO:
        {
            const DoStatement* doStatement = static_cast<const DoStatement*>(node);

            code += prefix + "do\n";

            if (doStatement->body->kind == Construct::Kind::STATEMENT_COMPOUND)
            {
                if (!printNode(doStatement->body, prefix, code))
                {
                    return false;
                }
            }
            else
            {
                if (!printNode(doStatement->body, prefix + "    ", code))
                {
                    return false;
                }

                code += ";\n";
            }

            code += prefix + "while (";

            if (!printNode(doStatement->condition, "", code))
            {
                return false;
            }

            code += ")";

            break;
        }

        case Construct::Kind::STATEMENT_BREAK:
        {
            code += prefix + "break;";
            break;
        }

        case Construct::Kind::STATEMENT_CONTINUE:
        {
            code += prefix + "continue;";
            break;
        }

        case Construct::Kind::STATEMENT_RETURN:
        {
            const ReturnStatement* returnStatement = static_cast<const ReturnStatement*>(node);

            code += prefix + "return";

            if (returnStatement->result)
            {
                code += " ";

                if (!printNode(returnStatement->result, "", code))
                {
                    return false;
                }
            }

            code += ";";
            break;
        }

        case Construct::Kind::OPERATOR_UNARY:
        {
            const UnaryOperatorExpression* unaryOperatorExpression = static_cast<const UnaryOperatorExpression*>(node);

            code += prefix + unaryOperatorExpression->value;

            if (!printNode(unaryOperatorExpression->expression, "", code))
            {
                return false;
            }
            break;
        }

        case Construct::Kind::OPERATOR_BINARY:
        {
            const BinaryOperatorExpression* binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(node);

            if (!printNode(binaryOperatorExpression->leftExpression, prefix, code))
            {
                return false;
            }

            code += " " + binaryOperatorExpression->value + " ";

            if (!printNode(binaryOperatorExpression->rightExpression, "", code))
            {
                return false;
            }
            break;
        }

        case Construct::Kind::OPERATOR_TERNARY:
        {
            const TernaryOperatorExpression* ternaryOperatorExpression = static_cast<const TernaryOperatorExpression*>(node);

            if (!printNode(ternaryOperatorExpression->condition, prefix, code))
            {
                return false;
            }

            code += " ? ";

            if (!printNode(ternaryOperatorExpression->leftExpression, "", code))
            {
                return false;
            }

            code += " : ";

            if (!printNode(ternaryOperatorExpression->rightExpression, "", code))
            {
                return false;
            }
            break;
        }
    }

    return true;
}
