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

    printNode(context.translationUnit, "", code);

    file << code;

    return true;
}

bool OutputHLSL::printNode(const std::unique_ptr<ASTNode>& node, const std::string& prefix, std::string& code)
{
    switch (node->type)
    {
        case ASTNode::Type::NONE:
        {
            break;
        }

        case ASTNode::Type::TRANSLATION_UNIT:
        {
            for (std::unique_ptr<ASTNode>& child : node->children)
            {
                if (!printNode(child, prefix, code))
                {
                    return false;
                }
            }
            break;
        }

        case ASTNode::Type::DECLARATION_EMPTY:
        {
            break;
        }

        case ASTNode::Type::DECLARATION_STRUCT:
        {
            code += prefix + "struct " + node->name;

            if (node->children.empty())
            {
                code += ";\n";
            }
            else
            {
                code += prefix + "\n{\n";

                for (std::unique_ptr<ASTNode>& child : node->children)
                {
                    if (!printNode(child, prefix + "    ", code))
                    {
                        return false;
                    }
                }

                code += prefix + "}\n";
            }

            break;
        }

        case ASTNode::Type::DECLARATION_FIELD:
        {
            code += prefix + node->typeName + " " + node->name + ";\n";

            break;
        }

        case ASTNode::Type::DECLARATION_TYPE_DEFINITION:
        {
            break;
        }

        case ASTNode::Type::DECLARATION_FUNCTION:
        {
            code += prefix + node->typeName + " " + node->name + "();\n";

            code += prefix + "{\n";

            for (std::unique_ptr<ASTNode>& child : node->children)
            {
                if (!printNode(child, prefix + "    ", code))
                {
                    return false;
                }
            }

            code += prefix + "}\n";

            break;
        }

        case ASTNode::Type::DECLARATION_VARIABLE:
        {
            code += prefix + node->typeName + " " + node->name + ";\n";
            break;
        }

        case ASTNode::Type::DECLARATION_PARAMETER:
        {
            code += prefix + node->typeName + " " + node->name;
            break;
        }

        case ASTNode::Type::EXPRESSION_CALL:
        {
            code += prefix + node->name + "()";
            break;
        }

        case ASTNode::Type::EXPRESSION_LITERAL:
        {
            code += node->value;
            break;
        }

        case ASTNode::Type::EXPRESSION_DECLARATION_REFERENCE:
        {
            code += node->name;
            break;
        }

        case ASTNode::Type::EXPRESSION_PAREN:
        {
            break;
        }

        case ASTNode::Type::STATEMENT_DECLARATION:
        {
            code += prefix + node->typeName + " " + node->name;
            break;
        }

        case ASTNode::Type::STATEMENT_COMPOUND:
        {
            code += prefix + "{\n";

            for (std::unique_ptr<ASTNode>& child : node->children)
            {
                if (!printNode(child, prefix + "    ", code))
                {
                    return false;
                }
            }

            code += prefix + "}\n";
            break;
        }

        case ASTNode::Type::STATEMENT_IF:
        {
            break;
        }

        case ASTNode::Type::STATEMENT_FOR:
        {
            break;
        }

        case ASTNode::Type::STATEMENT_SWITCH:
        {
            break;
        }

        case ASTNode::Type::STATEMENT_CASE:
        {
            break;
        }

        case ASTNode::Type::STATEMENT_WHILE:
        {
            break;
        }

        case ASTNode::Type::STATEMENT_DO:
        {
            break;
        }

        case ASTNode::Type::STATEMENT_BREAK:
        {
            break;
        }

        case ASTNode::Type::STATEMENT_CONTINUE:
        {
            break;
        }

        case ASTNode::Type::STATEMENT_RETURN:
        {
            break;
        }

        case ASTNode::Type::STATEMENT_EXPRESSION:
        {
            break;
        }

        case ASTNode::Type::OPERATOR_UNARY:
        {
            break;
        }

        case ASTNode::Type::OPERATOR_BINARY:
        {
            break;
        }

        case ASTNode::Type::OPERATOR_TERNARY:
        {
            break;
        }

        default: return false;
    }

    return true;
}
