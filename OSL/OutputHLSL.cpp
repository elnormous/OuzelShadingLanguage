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

bool OutputHLSL::printNode(const std::unique_ptr<ASTNode>& node, const std::string& prefix, std::string& code)
{
    switch (node->kind)
    {
        case ASTNode::Kind::NONE:
        {
            break;
        }

        case ASTNode::Kind::TRANSLATION_UNIT:
        {
            for (std::unique_ptr<ASTNode>& child : node->children)
            {
                if (!printNode(child, prefix, code))
                {
                    return false;
                }

                code += "\n";
            }
            break;
        }

        case ASTNode::Kind::DECLARATION_EMPTY:
        {
            break;
        }

        case ASTNode::Kind::DECLARATION_STRUCT:
        {
            code += prefix + "struct " + node->name;

            if (node->children.empty())
            {
                code += ";";
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

                    code += "\n";
                }

                code += prefix + "};";
            }

            break;
        }

        case ASTNode::Kind::DECLARATION_FIELD:
        {
            if (!node->reference)
            {
                std::cerr << "Invalid declaration reference" << std::endl;
                return false;
            }

            code += prefix + node->reference->name + " " + node->name + ";";
            break;
        }

        case ASTNode::Kind::DECLARATION_FUNCTION:
        {
            if (!node->reference)
            {
                std::cerr << "Invalid declaration reference" << std::endl;
                return false;
            }

            code += prefix + node->reference->name + " " + node->name + "(";

            auto i = node->children.cbegin();

            for (; i != node->children.cend() && (*i)->kind == ASTNode::Kind::DECLARATION_PARAMETER; ++i)
            {
                if (i != node->children.cbegin()) code += ", ";
                if (!printNode(*i, "", code))
                {
                    return false;
                }
            }

            if (i == node->children.cend())
            {
                code += ");"; // does not have a definition
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

        case ASTNode::Kind::DECLARATION_VARIABLE:
        {
            if (!node->reference)
            {
                std::cerr << "Invalid declaration reference" << std::endl;
                return false;
            }

            code += prefix + node->reference->name + " " + node->name + ";";
            break;
        }

        case ASTNode::Kind::DECLARATION_PARAMETER:
        {
            if (!node->reference)
            {
                std::cerr << "Invalid declaration reference" << std::endl;
                return false;
            }

            code += prefix + node->reference->name + " " + node->name;
            break;
        }

        case ASTNode::Kind::EXPRESSION_CALL:
        {
            auto i = node->children.cbegin();
            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            if (!printNode(*i, prefix, code))
            {
                return false;
            }

            ++i;
            code += "(";

            for (; i != node->children.cend(); ++i)
            {
                if (i != node->children.cbegin() + 1) code += ", ";
                if (!printNode(*i, "", code))
                {
                    return false;
                }
            }

            code += ")";
            break;
        }

        case ASTNode::Kind::EXPRESSION_LITERAL:
        {
            code += prefix + node->value;
            break;
        }

        case ASTNode::Kind::EXPRESSION_DECLARATION_REFERENCE:
        {
            if (!node->reference)
            {
                std::cerr << "Invalid declaration reference" << std::endl;
                return false;
            }

            code += prefix + node->reference->name;
            break;
        }

        case ASTNode::Kind::EXPRESSION_PAREN:
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

        case ASTNode::Kind::EXPRESSION_MEMBER:
        {
            auto i = node->children.cbegin();
            if (i == node->children.end())
            {
                std::cerr << "Expected a variable name" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }

            code += ".";
            ++i;

            if (i == node->children.end())
            {
                std::cerr << "Expected a field name" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }
            break;
        }

        case ASTNode::Kind::EXPRESSION_ARRAY_SUBSCRIPT:
        {
            auto i = node->children.cbegin();
            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            if (!printNode(*i, prefix, code))
            {
                return false;
            }

            ++i;
            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            code += "[";

            if (!printNode(*i, "", code))
            {
                return false;
            }

            code += "]";
            
            break;
        }

        case ASTNode::Kind::STATEMENT_DECLARATION:
        {
            if (!node->reference)
            {
                std::cerr << "Invalid declaration reference" << std::endl;
                return false;
            }

            code += prefix + node->reference->name + " " + node->name;
            break;
        }

        case ASTNode::Kind::STATEMENT_COMPOUND:
        {
            code += prefix + "{\n";

            for (std::unique_ptr<ASTNode>& child : node->children)
            {
                if (!printNode(child, prefix + "    ", code))
                {
                    return false;
                }

                code += "\n";
            }

            code += prefix + "}";
            break;
        }

        case ASTNode::Kind::STATEMENT_IF:
        {
            code += prefix + "if (";

            auto i = node->children.cbegin();

            if (i == node->children.end())
            {
                std::cerr << "Expected a condition" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }

            code += ")\n";
            ++i;

            for (; i != node->children.end(); ++i)
            {
                if (!printNode(*i, (*i)->kind == ASTNode::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
                {
                    return false;
                }
            }
            break;
        }

        case ASTNode::Kind::STATEMENT_FOR:
        {
            code += prefix + "for (";

            auto i = node->children.cbegin();

            if (i == node->children.end())
            {
                std::cerr << "Expected an initialization" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }

            code += "; ";
            ++i;

            if (i == node->children.end())
            {
                std::cerr << "Expected a condition" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }

            code += "; ";
            ++i;

            if (i == node->children.end())
            {
                std::cerr << "Expected an increment" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }

            code += ")\n";
            ++i;

            for (; i != node->children.end(); ++i)
            {
                if (!printNode(*i, (*i)->kind == ASTNode::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
                {
                    return false;
                }
            }
            break;
        }

        case ASTNode::Kind::STATEMENT_SWITCH:
        {
            code += prefix + "switch (";

            auto i = node->children.cbegin();

            if (i == node->children.end())
            {
                std::cerr << "Expected a condition" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }

            code += ")\n";
            ++i;

            for (; i != node->children.end(); ++i)
            {
                if (!printNode(*i, (*i)->kind == ASTNode::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
                {
                    return false;
                }
            }
            break;
        }

        case ASTNode::Kind::STATEMENT_CASE:
        {
            code += prefix + "case " + node->value + ":\n";

            for (auto i = node->children.cbegin(); i != node->children.end(); ++i)
            {
                if (!printNode(*i, (*i)->kind == ASTNode::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
                {
                    return false;
                }
            }
            break;
        }

        case ASTNode::Kind::STATEMENT_WHILE:
        {
            code += prefix + "while (";

            auto i = node->children.cbegin();

            if (i == node->children.end())
            {
                std::cerr << "Expected a condition" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }

            code += ")\n";
            ++i;

            for (; i != node->children.end(); ++i)
            {
                if (!printNode(*i, (*i)->kind == ASTNode::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
                {
                    return false;
                }
            }
            break;
        }

        case ASTNode::Kind::STATEMENT_DO:
        {
            code += prefix + "do\n";

            auto i = node->children.cbegin();
            if (i == node->children.end())
            {
                std::cerr << "Expected a statement" << std::endl;
                return false;
            }

            for (; i + 1 != node->children.end(); ++i)
            {
                if (!printNode(*i, (*i)->kind == ASTNode::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
                {
                    return false;
                }

                code += "\n";
            }

            code += prefix + "while (";

            if (!printNode(*i, (*i)->kind == ASTNode::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
            {
                return false;
            }

            code += ")";

            break;
        }

        case ASTNode::Kind::STATEMENT_BREAK:
        {
            code += prefix + "break;";
            break;
        }

        case ASTNode::Kind::STATEMENT_CONTINUE:
        {
            code += prefix + "continue;";
            break;
        }

        case ASTNode::Kind::STATEMENT_RETURN:
        {
            code += prefix + "return";

            if (!node->children.empty())
            {
                code += " ";

                for (std::unique_ptr<ASTNode>& child : node->children)
                {
                    if (!printNode(child, "", code))
                    {
                        return false;
                    }
                }
            }

            code += ";";
            break;
        }

        case ASTNode::Kind::STATEMENT_EXPRESSION:
        {
            code += prefix;

            for (std::unique_ptr<ASTNode>& child : node->children)
            {
                if (!printNode(child, "", code))
                {
                    return false;
                }
                code += ";";
            }
            break;
        }

        case ASTNode::Kind::OPERATOR_UNARY:
        {
            code += prefix + node->value;

            auto i = node->children.cbegin();
            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }

            break;
        }

        case ASTNode::Kind::OPERATOR_BINARY:
        {
            auto i = node->children.cbegin();
            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }

            code += " " + node->value + " ";
            ++i;

            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }

            break;
        }

        case ASTNode::Kind::OPERATOR_TERNARY:
        {
            auto i = node->children.cbegin();
            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }

            code += " ? ";
            ++i;

            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }

            code += " : ";
            ++i;

            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            if (!printNode(*i, "", code))
            {
                return false;
            }
            break;
        }
    }

    return true;
}
