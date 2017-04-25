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

bool parseNode(const std::vector<Token>& tokens, ASTNode& node)
{
    return true;
}

bool parse(const std::vector<Token>& tokens, ASTContext& context)
{
    bool topLevel = true;

    for (std::vector<Token>::const_iterator i = tokens.begin(); i != tokens.end(); ++i)
    {


        // TODO: parse statement
        // file must have only declaration (type, variable, function) statements at the top level
    }

    return true;
}

void dumpNode(const ASTNode& node, std::string indent)
{
    std::cout << indent << nodeTypeToString(node.type) << std::endl;

    for (const auto child : node.children)
    {
        dumpNode(child, indent + "  ");
    }
}

void dumpContext(ASTContext& context)
{
    for (const auto node : context.nodes)
    {
        dumpNode(node);
    }
}
