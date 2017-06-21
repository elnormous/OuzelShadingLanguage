//
//  Parser.cpp
//  OSLParser
//
//  Created by Elviss Strazdins on 25/04/2017.
//  Copyright (c) 2017 Elviss Strazdins. All rights reserved.
//

#include <iostream>
#include "Parser.h"

std::vector<std::string> builtinTypes = {
    "void", "bool", "int", "uint", "float", "double",
    "vec2", "vec3", "vec4", "mat3", "mat4"
};

bool ASTContext::parse(const std::vector<Token>& tokens)
{
    for (std::vector<Token>::const_iterator iterator = tokens.begin(); iterator != tokens.end(); ++iterator)
    {
        if (std::unique_ptr<ASTNode> node = parseTopLevel(tokens, iterator))
        {
            nodes.push_back(std::move(node));
        }
        else
        {
            std::cerr << "Failed to parse top level" << std::endl;
            return false;
        }
    }

    return true;
}

std::unique_ptr<ASTNode> ASTContext::parseTopLevel(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    if (iterator->type == Token::Type::KEYWORD_STRUCT)
    {
        return parseStructDecl(tokens, iterator);
    }
    else if (iterator->type == Token::Type::KEYWORD_TYPEDEF)
    {
        return parseTypedefDecl(tokens, iterator);
    }
    else
    {
        std::unique_ptr<ASTNode> result(new ASTNode());

        if (iterator->type == Token::Type::KEYWORD_CONST)
        {
            result->constType = true;
            ++iterator;
        }

        if (iterator != tokens.end() &&
            iterator->type == Token::Type::IDENTIFIER)
        {
            result->typeName = iterator->value;

            if (++iterator != tokens.end() &&
                iterator->type == Token::Type::IDENTIFIER)
            {
                result->name = iterator->value;

                if (++iterator != tokens.end())
                {
                    if (iterator->type == Token::Type::SEMICOLON)
                    {
                        result->type = ASTNode::Type::VARIABLE_DECLARATION;
                    }
                    else if (iterator->type == Token::Type::LEFT_PARENTHESIS)
                    {
                        result->type = ASTNode::Type::FUNCTION_DECLARATION;

                        if (++iterator != tokens.end())
                        {
                            if (iterator->type == Token::Type::IDENTIFIER)
                            {
                                for (;;)
                                {
                                    if (std::unique_ptr<ASTNode> param = parseParamDecl(tokens, iterator))
                                    {
                                        result->children.push_back(std::move(param));
                                    }
                                    else
                                    {
                                        std::cerr << "Failed to parse parameter declaration" << std::endl;
                                        return nullptr;
                                    }

                                    if (++iterator != tokens.end())
                                    {
                                        if (iterator->type == Token::Type::RIGHT_PARENTHESIS)
                                        {
                                            break;
                                        }
                                        else if (iterator->type != Token::Type::COMMA)
                                        {
                                            std::cerr << "Expected a comma or right parenthesis" << std::endl;
                                            return nullptr;
                                        }
                                    }
                                    else
                                    {
                                        std::cerr << "Unexpected end of function declaration" << std::endl;
                                        return nullptr;
                                    }
                                }
                            }
                            else if (iterator->type != Token::Type::RIGHT_PARENTHESIS)
                            {
                                std::cerr << "Expected a keyword or right parenthesis" << std::endl;
                                return nullptr;
                            }

                            // parse body
                            if (++iterator != tokens.end())
                            {
                                if (iterator->type == Token::Type::LEFT_BRACE)
                                {
                                    // TODO: parse body
                                    result->definition = true;
                                    return result;
                                }
                                else if (iterator->type != Token::Type::SEMICOLON)
                                {
                                    return result;
                                }
                                else
                                {
                                    std::cerr << "Expected a left brace or semicolon" << std::endl;
                                    return nullptr;
                                }
                            }
                            else
                            {
                                std::cerr << "Unexpected end of function declaration" << std::endl;
                                return nullptr;
                            }
                        }
                        else
                        {
                            std::cerr << "Unexpected end of function declaration" << std::endl;
                            return nullptr;
                        }
                    }
                }
                else
                {
                    std::cerr << "Unexpected end of declaration" << std::endl;
                    return nullptr;
                }
            }
            else
            {
                std::cerr << "Expected a name" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cerr << "Expected a type" << std::endl;
            return nullptr;
        }

        return result;
    }
}

std::unique_ptr<ASTNode> ASTContext::parseStructDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    if (iterator->type == Token::Type::KEYWORD_STRUCT)
    {
        if (++iterator != tokens.end() && iterator->type == Token::Type::IDENTIFIER)
        {
            std::unique_ptr<ASTNode> result(new ASTNode());
            result->type = ASTNode::Type::STRUCT_DECLARATION;
            result->name = iterator->value;

            if (++iterator != tokens.end())
            {
                if (iterator->type == Token::Type::LEFT_BRACE)
                {
                    result->definition = true;

                    for (;;)
                    {
                        if (++iterator != tokens.end())
                        {
                            if (iterator->type == Token::Type::RIGHT_BRACE)
                            {
                                if (++iterator != tokens.end() && iterator->type == Token::Type::SEMICOLON)
                                {
                                    if (!result->children.empty())
                                    {
                                        return result;
                                    }
                                    else
                                    {
                                        std::cerr << "Structure must have at least one member" << std::endl;
                                        return nullptr;
                                    }
                                }
                                else
                                {
                                    std::cerr << "Expected a semicolon" << std::endl;
                                    return nullptr;
                                }
                            }
                            else if (iterator->type == Token::Type::IDENTIFIER)
                            {
                                if (std::unique_ptr<ASTNode> field = parseFieldDecl(tokens, iterator))
                                {
                                    result->children.push_back(std::move(field));
                                }
                                else
                                {
                                    std::cerr << "Failed to parse field declaration" << std::endl;
                                    return nullptr;
                                }
                            }
                            else
                            {
                                std::cerr << "Expected a type name" << std::endl;
                                return nullptr;
                            }
                        }
                        else
                        {
                            std::cerr << "Unexpected end of structure declaration" << std::endl;
                            return nullptr;
                        }
                    }
                }
                else if (iterator->type == Token::Type::SEMICOLON)
                {
                    return result;
                }
                else
                {
                    std::cerr << "Expected left brace or semicolon" << std::endl;
                    return nullptr;
                }
            }
            else
            {
                std::cerr << "Unexpected end of declaration" << std::endl;
                return nullptr;
            }
        }
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseFieldDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    if (iterator->type == Token::Type::IDENTIFIER)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::FIELD_DECLARATION;

        result->typeName = iterator->value;

        while (++iterator != tokens.end())
        {
            if (iterator->type == Token::Type::IDENTIFIER)
            {
                result->name = iterator->value;

                if (++iterator != tokens.end() && iterator->type == Token::Type::SEMICOLON)
                {
                    return result;
                }
                else
                {
                    std::cerr << "Expected a semicolon" << std::endl;
                    return nullptr;
                }
            }
            else if (iterator->type == Token::Type::LEFT_BRACKET)
            {
                if (++iterator != tokens.end() && iterator->type == Token::Type::LEFT_BRACKET)
                {
                    if (++iterator != tokens.end() && iterator->type == Token::Type::IDENTIFIER)
                    {
                        ASTNode::Semantic semantic = ASTNode::Semantic::NONE;

                        // TODO: find slot number
                        if (iterator->value == "binormal") semantic = ASTNode::Semantic::BINORMAL;
                        else if (iterator->value == "blend_indices") semantic = ASTNode::Semantic::BLEND_INDICES;
                        else if (iterator->value == "blend_weight") semantic = ASTNode::Semantic::BLEND_WEIGHT;
                        else if (iterator->value == "color") semantic = ASTNode::Semantic::COLOR;
                        else if (iterator->value == "normal") semantic = ASTNode::Semantic::NORMAL;
                        else if (iterator->value == "position") semantic = ASTNode::Semantic::POSITION;
                        else if (iterator->value == "position_transformed") semantic = ASTNode::Semantic::POSITION_TRANSFORMED;
                        else if (iterator->value == "point_size") semantic = ASTNode::Semantic::POINT_SIZE;
                        else if (iterator->value == "tangent") semantic = ASTNode::Semantic::TANGENT;
                        else if (iterator->value == "texture_coordinates") semantic = ASTNode::Semantic::TEXTURE_COORDINATES;
                        else
                        {
                            std::cerr << "Invalid semantic" << std::endl;
                            return nullptr;
                        }

                        if (++iterator != tokens.end() && iterator->type == Token::Type::RIGHT_BRACKET)
                        {
                            if (++iterator != tokens.end() && iterator->type == Token::Type::RIGHT_BRACKET)
                            {
                                result->semantic = semantic;
                            }
                            else
                            {
                                std::cerr << "Expected a right bracket" << std::endl;
                                return nullptr;
                            }
                        }
                        else
                        {
                            std::cerr << "Expected a right bracket" << std::endl;
                            return nullptr;
                        }
                    }
                }
                else
                {
                    std::cerr << "Expected a left bracket" << std::endl;
                    return nullptr;
                }
            }
            else
            {
                std::cerr << "Expected a field name" << std::endl;
                return nullptr;
            }
        }
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseTypedefDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    if (iterator->type == Token::Type::KEYWORD_TYPEDEF)
    {
        if (++iterator != tokens.end() && iterator->type == Token::Type::IDENTIFIER)
        {
            std::unique_ptr<ASTNode> result(new ASTNode());
            result->type = ASTNode::Type::TYPE_DEFINITION_DECLARATION;
            result->typeName = iterator->value;

            if (++iterator != tokens.end() && iterator->type == Token::Type::IDENTIFIER)
            {
                result->name = iterator->value;

                if (++iterator != tokens.end() && iterator->type == Token::Type::SEMICOLON)
                {
                    return result;
                }
                else
                {
                    std::cerr << "Expected a semicolon" << std::endl;
                    return nullptr;
                }
            }
            else
            {
                std::cerr << "Expected a type name" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cerr << "Expected a type name" << std::endl;
            return nullptr;
        }
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseVarDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    if (iterator->type == Token::Type::IDENTIFIER)
    {
    }
    else if (iterator->type == Token::Type::KEYWORD_CONST)
    {
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseParamDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    if (iterator->type == Token::Type::IDENTIFIER)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::PARAMETER_DECLARATION;
        result->typeName = iterator->value;

        if (++iterator != tokens.end() && iterator->type == Token::Type::IDENTIFIER)
        {
            result->name = iterator->value;
        }
        else
        {
            std::cerr << "Expected a keyword" << std::endl;
            return nullptr;
        }

        return result;
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseFunctionDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    if (iterator->type == Token::Type::IDENTIFIER)
    {
        //node.type = ASTNode::Type::FUNCTION_DECLARATION;
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseIf(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseFor(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseWhile(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseDo(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseStatement(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    return nullptr;
}

void ASTContext::dump()
{
    for (const std::unique_ptr<ASTNode>& node : nodes)
    {
        ASTContext::dumpNode(node);
    }
}

void ASTContext::dumpNode(const std::unique_ptr<ASTNode>& node, std::string indent)
{
    std::cout << indent << nodeTypeToString(node->type);

    if (!node->name.empty()) std::cout << ", name: " << node->name;
    if (!node->typeName.empty())
    {
        std::cout << ", type: ";
        if (node->constType) std::cout << "const ";
        std::cout << node->typeName;

    }
    if (node->semantic != ASTNode::Semantic::NONE) std::cout << ", semantic: " << semanticToString(node->semantic);
    if (node->definition) std::cout << ", has definition";

    std::cout << std::endl;

    for (const std::unique_ptr<ASTNode>& child : node->children)
    {
        ASTContext::dumpNode(child, indent + "  ");
    }
}
