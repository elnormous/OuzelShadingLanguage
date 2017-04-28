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
        ASTNode node;
        if (parseTopLevel(tokens, iterator, node))
        {
            nodes.push_back(node);
        }
        else
        {
            std::cout << "Failed to parse top level" << std::endl;
            return false;
        }
    }

    return true;
}

bool ASTContext::parseTopLevel(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node)
{
    const Token& token = *iterator;

    if (token.type == Token::Type::KEYWORD_STRUCT)
    {
        return parseStructDecl(tokens, iterator, node);
    }
    else if (token.type == Token::Type::KEYWORD_TYPEDEF)
    {
        return parseTypedefDecl(tokens, iterator, node);
    }
    else if (token.type == Token::Type::IDENTIFIER || token.type == Token::Type::KEYWORD_CONST)
    {
        return true;
    }
    else
    {
        std::cout << "Expected a declaration" << std::endl;
        return false;
    }
}

bool ASTContext::parseStructDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node)
{
    std::vector<Token>::const_iterator currentIterator = iterator;

    if (currentIterator->type == Token::Type::KEYWORD_STRUCT)
    {
        if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::IDENTIFIER)
        {
            node.name = currentIterator->value;

            if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::LEFT_BRACE)
            {
                ++currentIterator;

                for (; currentIterator != tokens.end(); ++currentIterator)
                {
                    if (currentIterator->type == Token::Type::IDENTIFIER)
                    {
                        ASTNode field;
                        field.type = ASTNode::Type::FIELD_DECLARATION;

                        if (currentIterator->value == "__semantic")
                        {
                            if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::LEFT_PARENTHESIS)
                            {
                                if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::IDENTIFIER)
                                {
                                    ASTNode::Semantic semantic = ASTNode::Semantic::NONE;

                                    if (currentIterator->value == "binormal") semantic = ASTNode::Semantic::BINORMAL;
                                    else if (currentIterator->value == "blend_indices") semantic = ASTNode::Semantic::BLEND_INDICES;
                                    else if (currentIterator->value == "blend_weight") semantic = ASTNode::Semantic::BLEND_WEIGHT;
                                    else if (currentIterator->value == "color") semantic = ASTNode::Semantic::COLOR;
                                    else if (currentIterator->value == "normal") semantic = ASTNode::Semantic::NORMAL;
                                    else if (currentIterator->value == "position") semantic = ASTNode::Semantic::POSITION;
                                    else if (currentIterator->value == "position_transformed") semantic = ASTNode::Semantic::POSITION_TRANSFORMED;
                                    else if (currentIterator->value == "point_size") semantic = ASTNode::Semantic::POINT_SIZE;
                                    else if (currentIterator->value == "tangent") semantic = ASTNode::Semantic::TANGENT;
                                    else if (currentIterator->value == "texture_coordinates") semantic = ASTNode::Semantic::TEXTURE_COORDINATES;
                                    else
                                    {
                                        std::cout << "Invalid semantic" << std::endl;
                                        return false;
                                    }

                                    if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::RIGHT_PARENTHESIS)
                                    {
                                        field.semantic = semantic;
                                        ++currentIterator;
                                    }
                                    else
                                    {
                                        std::cout << "Expected a right parenthesis" << std::endl;
                                        return false;
                                    }
                                }
                                else
                                {
                                    std::cout << "Expected a semantic name" << std::endl;
                                    return false;
                                }
                            }
                            else
                            {
                                std::cout << "Expected a left parenthesis" << std::endl;
                                return false;
                            }
                        }

                        field.typeName = currentIterator->value;

                        if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::IDENTIFIER)
                        {
                            field.name = currentIterator->value;

                            if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::SEMICOLON)
                            {
                                node.children.push_back(field);
                            }
                            else
                            {
                                std::cout << "Expected a semicolon" << std::endl;
                                return false;
                            }
                        }
                        else
                        {
                            std::cout << "Expected a field name" << std::endl;
                            return false;
                        }
                    }
                    else if (currentIterator->type == Token::Type::RIGHT_BRACE)
                    {
                        if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::SEMICOLON)
                        {
                            node.type = ASTNode::Type::STRUCT_DECLARATION;
                            return true;
                        }
                        else
                        {
                            std::cout << "Expected a semicolon" << std::endl;
                            return false;
                        }
                    }
                    else
                    {
                        std::cout << "Expected a type name" << std::endl;
                        return false;
                    }
                }
            }
            else
            {
                std::cout << "Expected a left brace" << std::endl;
                return false;
            }
        }
    }

    return false;
}

bool ASTContext::parseTypedefDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node)
{
    node.type = ASTNode::Type::TYPE_DEFINITION_DECLARATION;

    return false;
}

bool ASTContext::parseVarDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node)
{
    const Token& token = *iterator;

    if (token.type == Token::Type::IDENTIFIER)
    {
        node.type = ASTNode::Type::VARIABLE_DECLARATION;
    }
    else if (token.type == Token::Type::KEYWORD_CONST)
    {
        node.type = ASTNode::Type::VARIABLE_DECLARATION;
    }

    return false;
}

bool ASTContext::parseFunctionDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node)
{
    const Token& token = *iterator;

    if (token.type == Token::Type::IDENTIFIER)
    {
        node.type = ASTNode::Type::FUNCTION_DECLARATION;
    }

    return false;
}

bool ASTContext::parseIf(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node)
{
    return false;
}

bool ASTContext::parseFor(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node)
{
    return false;
}

bool ASTContext::parseWhile(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node)
{
    return false;
}

bool ASTContext::parseDo(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node)
{
    return false;
}

bool ASTContext::parseStatement(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node)
{
    return false;
}

void ASTContext::dump()
{
    for (const auto node : nodes)
    {
        ASTContext::dumpNode(node);
    }
}

void ASTContext::dumpNode(const ASTNode& node, std::string indent)
{
    std::cout << indent << nodeTypeToString(node.type);

    if (!node.name.empty()) std::cout << ", name: " << node.name;
    if (!node.typeName.empty()) std::cout << ", type: " << node.typeName;
    if (node.semantic != ASTNode::Semantic::NONE) std::cout << ", semantic: " << semanticToString(node.semantic);

    std::cout << std::endl;

    for (const auto child : node.children)
    {
        ASTContext::dumpNode(child, indent + "  ");
    }
}
