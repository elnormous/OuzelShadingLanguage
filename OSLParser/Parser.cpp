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
            std::cout << "Failed to parse top level" << std::endl;
            return false;
        }
    }

    return true;
}

std::unique_ptr<ASTNode> ASTContext::parseTopLevel(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    const Token& token = *iterator;

    if (token.type == Token::Type::KEYWORD_STRUCT)
    {
        return parseStructDecl(tokens, iterator);
    }
    else if (token.type == Token::Type::KEYWORD_TYPEDEF)
    {
        return parseTypedefDecl(tokens, iterator);
    }
    else if (token.type == Token::Type::IDENTIFIER || token.type == Token::Type::KEYWORD_CONST)
    {
        return nullptr;
    }
    else
    {
        std::cout << "Expected a declaration" << std::endl;
        return nullptr;
    }
}

std::unique_ptr<ASTNode> ASTContext::parseStructDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    std::vector<Token>::const_iterator currentIterator = iterator;

    std::unique_ptr<ASTNode> node(new ASTNode());

    if (currentIterator->type == Token::Type::KEYWORD_STRUCT)
    {
        if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::IDENTIFIER)
        {
            node->name = currentIterator->value;

            if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::LEFT_BRACE)
            {
                ++currentIterator;

                for (; currentIterator != tokens.end(); ++currentIterator)
                {
                    if (currentIterator->type == Token::Type::RIGHT_BRACE)
                    {
                        if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::SEMICOLON)
                        {
                            if (!node->children.empty())
                            {
                                node->type = ASTNode::Type::STRUCT_DECLARATION;
                                iterator = currentIterator;
                                return node;
                            }
                            else
                            {
                                std::cout << "Structure must have at least one member" << std::endl;
                                return nullptr;
                            }
                        }
                        else
                        {
                            std::cout << "Expected a semicolon" << std::endl;
                            return nullptr;
                        }
                    }
                    else if (currentIterator->type == Token::Type::IDENTIFIER)
                    {
                        if (std::unique_ptr<ASTNode> field = parseFieldDecl(tokens, currentIterator))
                        {
                            node->children.push_back(std::move(field));
                        }
                    }
                    else
                    {
                        std::cout << "Expected a type name" << std::endl;
                        return nullptr;
                    }
                }
            }
            else
            {
                std::cout << "Expected a left brace" << std::endl;
                return nullptr;
            }
        }
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseFieldDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    std::vector<Token>::const_iterator currentIterator = iterator;

    if (currentIterator->type == Token::Type::IDENTIFIER)
    {
        std::unique_ptr<ASTNode> field(new ASTNode());
        field->type = ASTNode::Type::FIELD_DECLARATION;

        if (currentIterator->value == "__semantic")
        {
            if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::LEFT_PARENTHESIS)
            {
                if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::IDENTIFIER)
                {
                    ASTNode::Semantic semantic = ASTNode::Semantic::NONE;

                    // TODO: find slot number
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
                        return nullptr;
                    }

                    if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::RIGHT_PARENTHESIS)
                    {
                        field->semantic = semantic;
                        ++currentIterator;
                    }
                    else
                    {
                        std::cout << "Expected a right parenthesis" << std::endl;
                        return nullptr;
                    }
                }
                else
                {
                    std::cout << "Expected a semantic name" << std::endl;
                    return nullptr;
                }
            }
            else
            {
                std::cout << "Expected a left parenthesis" << std::endl;
                return nullptr;
            }
        }

        field->typeName = currentIterator->value;

        if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::IDENTIFIER)
        {
            field->name = currentIterator->value;

            if (++currentIterator != tokens.end() && currentIterator->type == Token::Type::SEMICOLON)
            {
                iterator = currentIterator;
                return field;
            }
            else
            {
                std::cout << "Expected a semicolon" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cout << "Expected a field name" << std::endl;
            return nullptr;
        }
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseTypedefDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    //node.type = ASTNode::Type::TYPE_DEFINITION_DECLARATION;

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseVarDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    const Token& token = *iterator;

    if (token.type == Token::Type::IDENTIFIER)
    {
        //node.type = ASTNode::Type::VARIABLE_DECLARATION;
    }
    else if (token.type == Token::Type::KEYWORD_CONST)
    {
        //node.type = ASTNode::Type::VARIABLE_DECLARATION;
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseFunctionDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    const Token& token = *iterator;

    if (token.type == Token::Type::IDENTIFIER)
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
    if (!node->typeName.empty()) std::cout << ", type: " << node->typeName;
    if (node->semantic != ASTNode::Semantic::NONE) std::cout << ", semantic: " << semanticToString(node->semantic);

    std::cout << std::endl;

    for (const std::unique_ptr<ASTNode>& child : node->children)
    {
        ASTContext::dumpNode(child, indent + "  ");
    }
}
