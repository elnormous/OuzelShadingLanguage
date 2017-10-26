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

    if (!printNode(context.translationUnit, "", code))
    {
        std::cerr << "Failed to print code" << std::endl;
        return EXIT_FAILURE;
    }

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
            code += prefix + node->typeName + " " + node->name + "(";

            auto i = node->children.cbegin();

            for (; i != node->children.cend() && (*i)->type == ASTNode::Type::DECLARATION_PARAMETER; ++i)
            {
                if (i != node->children.cbegin()) code += ", ";
                if (!printNode(*i, "", code))
                {
                    return false;
                }
            }

            if (i == node->children.end())
            {
                code += ");\n"; // does not have a definition
            }
            else
            {
                code += ")\n";

                for (; i != node->children.cend(); ++i)
                {
                    if (!printNode(*i, prefix, code))
                    {
                        return false;
                    }
                }
            }

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
            code += prefix + node->name + "(";

            for (auto i = node->children.cbegin(); i != node->children.cend(); ++i)
            {
                if (i != node->children.cbegin()) code += ", ";
                if (!printNode(*i, "", code))
                {
                    return false;
                }
            }

            code += ")";
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
            code += prefix + "(";

            for (std::unique_ptr<ASTNode>& child : node->children)
            {
                if (!printNode(child, "", code))
                {
                    return false;
                }
            }

            code += ")";
            break;
        }

        case ASTNode::Type::EXPRESSION_MEMBER:
        {
            break;
        }

        case ASTNode::Type::EXPRESSION_ARRAY_SUBSCRIPT:
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
            code += prefix + "if ()";

            for (std::unique_ptr<ASTNode>& child : node->children)
            {
                if (!printNode(child, prefix + "    ", code))
                {
                    return false;
                }
            }
            break;
        }

        case ASTNode::Type::STATEMENT_FOR:
        {
            code += prefix + "for ()";

            for (std::unique_ptr<ASTNode>& child : node->children)
            {
                if (!printNode(child, prefix + "    ", code))
                {
                    return false;
                }
            }
            break;
        }

        case ASTNode::Type::STATEMENT_SWITCH:
        {
            code += prefix + "switch ()";

            for (std::unique_ptr<ASTNode>& child : node->children)
            {
                if (!printNode(child, prefix + "    ", code))
                {
                    return false;
                }
            }
            break;
        }

        case ASTNode::Type::STATEMENT_CASE:
        {
            break;
        }

        case ASTNode::Type::STATEMENT_WHILE:
        {
            code += prefix + "while ()";

            for (std::unique_ptr<ASTNode>& child : node->children)
            {
                if (!printNode(child, prefix + "    ", code))
                {
                    return false;
                }
            }
            break;
        }

        case ASTNode::Type::STATEMENT_DO:
        {
            break;
        }

        case ASTNode::Type::STATEMENT_BREAK:
        {
            code += "break;";
            break;
        }

        case ASTNode::Type::STATEMENT_CONTINUE:
        {
            code += "continue;";
            break;
        }

        case ASTNode::Type::STATEMENT_RETURN:
        {
            code += "return;";
            break;
        }

        case ASTNode::Type::STATEMENT_EXPRESSION:
        {
            code += prefix;

            for (std::unique_ptr<ASTNode>& child : node->children)
            {
                if (!printNode(child, "", code))
                {
                    return false;
                }
                code += ";\n";
            }
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
    }

    return true;
}
