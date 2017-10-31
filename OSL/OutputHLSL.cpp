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

        case Construct::Kind::TRANSLATION_UNIT:
        {
            for (const std::unique_ptr<Construct>& child : node->children)
            {
                if (!printNode(child.get(), prefix, code))
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
            code += prefix + "struct " + node->name;

            if (node->children.empty())
            {
                code += ";";
            }
            else
            {
                code += prefix + "\n{\n";

                for (const std::unique_ptr<Construct>& child : node->children)
                {
                    if (!printNode(child.get(), prefix + "    ", code))
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
            if (!node->reference)
            {
                std::cerr << "Invalid declaration reference" << std::endl;
                return false;
            }

            code += prefix + node->reference->name + " " + node->name + ";";
            break;
        }

        case Construct::Kind::DECLARATION_FUNCTION:
        {
            if (!node->reference)
            {
                std::cerr << "Invalid declaration reference" << std::endl;
                return false;
            }

            code += prefix + node->reference->name + " " + node->name + "(";

            auto i = node->children.cbegin();

            for (; i != node->children.cend() && (*i)->kind == Construct::Kind::DECLARATION_PARAMETER; ++i)
            {
                if (i != node->children.cbegin()) code += ", ";
                if (!printNode(i->get(), "", code))
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
                    if (!printNode(i->get(), prefix, code))
                    {
                        return false;
                    }
                }
            }

            break;
        }

        case Construct::Kind::DECLARATION_VARIABLE:
        {
            if (!node->reference)
            {
                std::cerr << "Invalid declaration reference" << std::endl;
                return false;
            }

            code += prefix + node->reference->name + " " + node->name + ";";
            break;
        }

        case Construct::Kind::DECLARATION_PARAMETER:
        {
            if (!node->reference)
            {
                std::cerr << "Invalid declaration reference" << std::endl;
                return false;
            }

            code += prefix + node->reference->name + " " + node->name;
            break;
        }

        case Construct::Kind::EXPRESSION_CALL:
        {
            auto i = node->children.cbegin();
            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            if (!printNode(i->get(), prefix, code))
            {
                return false;
            }

            ++i;
            code += "(";

            for (; i != node->children.cend(); ++i)
            {
                if (i != node->children.cbegin() + 1) code += ", ";
                if (!printNode(i->get(), "", code))
                {
                    return false;
                }
            }

            code += ")";
            break;
        }

        case Construct::Kind::EXPRESSION_LITERAL:
        {
            code += prefix + node->value;
            break;
        }

        case Construct::Kind::EXPRESSION_DECLARATION_REFERENCE:
        {
            if (!node->reference)
            {
                std::cerr << "Invalid declaration reference" << std::endl;
                return false;
            }

            code += prefix + node->reference->name;
            break;
        }

        case Construct::Kind::EXPRESSION_PAREN:
        {
            code += prefix + "(";

            for (const std::unique_ptr<Construct>& child : node->children)
            {
                if (!printNode(child.get(), "", code))
                {
                    return false;
                }
            }

            code += ")";
            break;
        }

        case Construct::Kind::EXPRESSION_MEMBER:
        {
            auto i = node->children.cbegin();
            if (i == node->children.end())
            {
                std::cerr << "Expected a variable name" << std::endl;
                return false;
            }

            if (!printNode(i->get(), "", code))
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

            if (!printNode(i->get(), "", code))
            {
                return false;
            }
            break;
        }

        case Construct::Kind::EXPRESSION_ARRAY_SUBSCRIPT:
        {
            auto i = node->children.cbegin();
            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            if (!printNode(i->get(), prefix, code))
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

            if (!printNode(i->get(), "", code))
            {
                return false;
            }

            code += "]";
            
            break;
        }

        case Construct::Kind::STATEMENT_DECLARATION:
        {
            if (!node->reference)
            {
                std::cerr << "Invalid declaration reference" << std::endl;
                return false;
            }

            code += prefix + node->reference->name + " " + node->name;
            break;
        }

        case Construct::Kind::STATEMENT_COMPOUND:
        {
            code += prefix + "{\n";

            for (const std::unique_ptr<Construct>& child : node->children)
            {
                if (!printNode(child.get(), prefix + "    ", code))
                {
                    return false;
                }

                code += "\n";
            }

            code += prefix + "}";
            break;
        }

        case Construct::Kind::STATEMENT_IF:
        {
            code += prefix + "if (";

            auto i = node->children.cbegin();

            if (i == node->children.end())
            {
                std::cerr << "Expected a condition" << std::endl;
                return false;
            }

            if (!printNode(i->get(), "", code))
            {
                return false;
            }

            code += ")\n";
            ++i;

            for (; i != node->children.end(); ++i)
            {
                if (!printNode(i->get(), (*i)->kind == Construct::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
                {
                    return false;
                }
            }
            break;
        }

        case Construct::Kind::STATEMENT_FOR:
        {
            code += prefix + "for (";

            auto i = node->children.cbegin();

            if (i == node->children.end())
            {
                std::cerr << "Expected an initialization" << std::endl;
                return false;
            }

            if (!printNode(i->get(), "", code))
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

            if (!printNode(i->get(), "", code))
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

            if (!printNode(i->get(), "", code))
            {
                return false;
            }

            code += ")\n";
            ++i;

            for (; i != node->children.end(); ++i)
            {
                if (!printNode(i->get(), (*i)->kind == Construct::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
                {
                    return false;
                }
            }
            break;
        }

        case Construct::Kind::STATEMENT_SWITCH:
        {
            code += prefix + "switch (";

            auto i = node->children.cbegin();

            if (i == node->children.end())
            {
                std::cerr << "Expected a condition" << std::endl;
                return false;
            }

            if (!printNode(i->get(), "", code))
            {
                return false;
            }

            code += ")\n";
            ++i;

            for (; i != node->children.end(); ++i)
            {
                if (!printNode(i->get(), (*i)->kind == Construct::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
                {
                    return false;
                }
            }
            break;
        }

        case Construct::Kind::STATEMENT_CASE:
        {
            code += prefix + "case " + node->value + ":\n";

            for (auto i = node->children.cbegin(); i != node->children.end(); ++i)
            {
                if (!printNode(i->get(), (*i)->kind == Construct::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
                {
                    return false;
                }
            }
            break;
        }

        case Construct::Kind::STATEMENT_WHILE:
        {
            code += prefix + "while (";

            auto i = node->children.cbegin();

            if (i == node->children.end())
            {
                std::cerr << "Expected a condition" << std::endl;
                return false;
            }

            if (!printNode(i->get(), "", code))
            {
                return false;
            }

            code += ")\n";
            ++i;

            for (; i != node->children.end(); ++i)
            {
                if (!printNode(i->get(), (*i)->kind == Construct::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
                {
                    return false;
                }
            }
            break;
        }

        case Construct::Kind::STATEMENT_DO:
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
                if (!printNode(i->get(), (*i)->kind == Construct::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
                {
                    return false;
                }

                code += "\n";
            }

            code += prefix + "while (";

            if (!printNode(i->get(), (*i)->kind == Construct::Kind::STATEMENT_COMPOUND ? prefix : (prefix + "    "), code))
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
            code += prefix + "return";

            if (!node->children.empty())
            {
                code += " ";

                for (const std::unique_ptr<Construct>& child : node->children)
                {
                    if (!printNode(child.get(), "", code))
                    {
                        return false;
                    }
                }
            }

            code += ";";
            break;
        }

        case Construct::Kind::STATEMENT_EXPRESSION:
        {
            code += prefix;

            for (const std::unique_ptr<Construct>& child : node->children)
            {
                if (!printNode(child.get(), "", code))
                {
                    return false;
                }
                code += ";";
            }
            break;
        }

        case Construct::Kind::OPERATOR_UNARY:
        {
            code += prefix + node->value;

            auto i = node->children.cbegin();
            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            if (!printNode(i->get(), "", code))
            {
                return false;
            }

            break;
        }

        case Construct::Kind::OPERATOR_BINARY:
        {
            auto i = node->children.cbegin();
            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            if (!printNode(i->get(), "", code))
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

            if (!printNode(i->get(), "", code))
            {
                return false;
            }

            break;
        }

        case Construct::Kind::OPERATOR_TERNARY:
        {
            auto i = node->children.cbegin();
            if (i == node->children.end())
            {
                std::cerr << "Expected an expression" << std::endl;
                return false;
            }

            if (!printNode(i->get(), "", code))
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

            if (!printNode(i->get(), "", code))
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

            if (!printNode(i->get(), "", code))
            {
                return false;
            }
            break;
        }
    }

    return true;
}
