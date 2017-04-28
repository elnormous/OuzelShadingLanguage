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
    // only type, variable and function declarations allowed at top level
    if (parseTypeDecl(tokens, iterator, node))
    {
        return true;
    }

    if (parseVarDecl(tokens, iterator, node))
    {
        return true;
    }

    if (parseFunctionDecl(tokens, iterator, node))
    {
        return true;
    }

    return false;
}

bool ASTContext::parseTypeDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node)
{
    const Token& token = *iterator;

    if (token.type == Token::Type::IDENTIFIER)
    {
    }

    return false;
}

bool ASTContext::parseVarDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node)
{
    const Token& token = *iterator;

    if (token.type == Token::Type::IDENTIFIER)
    {
    }

    return false;
}

bool ASTContext::parseFunctionDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node)
{
    const Token& token = *iterator;

    if (token.type == Token::Type::IDENTIFIER)
    {
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
    std::cout << indent << nodeTypeToString(node.type) << std::endl;

    for (const auto child : node.children)
    {
        ASTContext::dumpNode(child, indent + "  ");
    }
}
