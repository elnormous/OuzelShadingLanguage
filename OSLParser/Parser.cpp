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
        std::unique_ptr<ASTNode> node(new ASTNode());

        if (iterator->type == Token::Type::KEYWORD_CONST)
        {
            node->constType = true;
            ++iterator;
        }

        if (iterator != tokens.end() &&
            iterator->type == Token::Type::IDENTIFIER)
        {
            node->typeName = iterator->value;

            if (++iterator != tokens.end() &&
                iterator->type == Token::Type::IDENTIFIER)
            {
                node->name = iterator->value;

                if (++iterator != tokens.end())
                {
                    if (iterator->type == Token::Type::SEMICOLON)
                    {
                        node->type = ASTNode::Type::VARIABLE_DECLARATION;
                    }
                    else if (iterator->type == Token::Type::LEFT_PARENTHESIS)
                    {
                        node->type = ASTNode::Type::FUNCTION_DECLARATION;

                        // TODO parse arguments
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

        return node;
    }
}

std::unique_ptr<ASTNode> ASTContext::parseStructDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator)
{
    if (iterator->type == Token::Type::KEYWORD_STRUCT)
    {
        if (++iterator != tokens.end() && iterator->type == Token::Type::IDENTIFIER)
        {
            std::unique_ptr<ASTNode> node(new ASTNode());
            node->type = ASTNode::Type::STRUCT_DECLARATION;
            node->name = iterator->value;

            if (++iterator != tokens.end())
            {
                if (iterator->type == Token::Type::LEFT_BRACE)
                {
                    node->definition = true;
                    ++iterator;

                    for (; iterator != tokens.end(); ++iterator)
                    {
                        if (iterator->type == Token::Type::RIGHT_BRACE)
                        {
                            if (++iterator != tokens.end() && iterator->type == Token::Type::SEMICOLON)
                            {
                                if (!node->children.empty())
                                {
                                    return node;
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
                                node->children.push_back(std::move(field));
                            }
                        }
                        else
                        {
                            std::cerr << "Expected a type name" << std::endl;
                            return nullptr;
                        }
                    }
                }
                else if (iterator->type == Token::Type::SEMICOLON)
                {
                    return node;
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
        std::unique_ptr<ASTNode> field(new ASTNode());
        field->type = ASTNode::Type::FIELD_DECLARATION;

        field->typeName = iterator->value;

        while (++iterator != tokens.end())
        {
            if (iterator->type == Token::Type::IDENTIFIER)
            {
                field->name = iterator->value;

                if (++iterator != tokens.end() && iterator->type == Token::Type::SEMICOLON)
                {
                    return field;
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
                                field->semantic = semantic;
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
            std::unique_ptr<ASTNode> node(new ASTNode());
            node->type = ASTNode::Type::TYPE_DEFINITION_DECLARATION;
            node->typeName = iterator->value;

            if (++iterator != tokens.end() && iterator->type == Token::Type::IDENTIFIER)
            {
                node->name = iterator->value;

                if (++iterator != tokens.end() && iterator->type == Token::Type::SEMICOLON)
                {
                    return node;
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
