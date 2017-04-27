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

bool parseIf()
{
    return true;
}

bool parseFor()
{
    return true;
}

bool parseWhile()
{
    return true;
}

bool parseDo()
{
    return true;
}

bool parseStatement()
{
    return true;
}

bool parseNode(const std::vector<Token>& tokens, ASTNode& node)
{
    return true;
}

bool ASTContext::parse(const std::vector<Token>& tokens)
{
    end = tokens.end();

    for (std::vector<Token>::const_iterator iterator = tokens.begin(); iterator != tokens.end(); ++iterator)
    {
        parseTopLevel(iterator);
    }

    return true;
}

bool ASTContext::parseTopLevel(std::vector<Token>::const_iterator& iterator)
{
    // TODO: parse statement
    // file must have only declaration (type, variable, function) statements at the top level

    const Token& token = *iterator;

    if (token.type == Token::Type::IDENTIFIER)
    {
    }

    return true;
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
