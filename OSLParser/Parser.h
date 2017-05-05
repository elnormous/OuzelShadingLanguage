//
//  Parser.h
//  OSLParser
//
//  Created by Elviss Strazdins on 25/04/2017.
//  Copyright (c) 2017 Elviss Strazdins. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include "Tokenizer.h"

struct ASTNode
{
    enum class Type
    {
        NONE,
        STRUCT_DECLARATION,
        FIELD_DECLARATION,
        TYPE_DEFINITION_DECLARATION,
        FUNCTION_DECLARATION,
        VARIABLE_DECLARATION,
        PARAMETER_DECLARATION,
        COMPOUND_STATEMENT,
        RETURN_STATEMENT,
        CALL_EXPRESSION,
        LITERAL_EXPRESSION,
        DECLARATION_REFERENCE_EXPRESSION,
        IF_STATEMENT,
        FOR_STATEMENT,
        WHILE_STATEMENT,
        DO_STATEMENT,
        ASSIGN_OPERATOR,
        UNARY_OPERATOR,
        BINARY_OPERATOR,
        TERNARY_OPERATOR,
    };

    enum class Semantic
    {
        NONE,
        BINORMAL,
        BLEND_INDICES,
        BLEND_WEIGHT,
        COLOR,
        NORMAL,
        POSITION,
        POSITION_TRANSFORMED,
        POINT_SIZE,
        TANGENT,
        TEXTURE_COORDINATES
    };

    Type type = Type::NONE;
    Semantic semantic = Semantic::NONE;
    std::string typeName;
    bool constType = false;
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> children;
};

inline std::string nodeTypeToString(ASTNode::Type type)
{
    switch (type)
    {
        case ASTNode::Type::NONE: return "NONE";
        case ASTNode::Type::STRUCT_DECLARATION: return "STRUCT_DECLARATION";
        case ASTNode::Type::FIELD_DECLARATION: return "FIELD_DECLARATION";
        case ASTNode::Type::TYPE_DEFINITION_DECLARATION: return "TYPE_DEFINITION_DECLARATION";
        case ASTNode::Type::FUNCTION_DECLARATION: return "FUNCTION_DECLARATION";
        case ASTNode::Type::VARIABLE_DECLARATION: return "VARIABLE_DECLARATION";
        case ASTNode::Type::PARAMETER_DECLARATION: return "PARAMETER_DECLARATION";
        case ASTNode::Type::COMPOUND_STATEMENT: return "COMPOUND_STATEMENT";
        case ASTNode::Type::RETURN_STATEMENT: return "RETURN_STATEMENT";
        case ASTNode::Type::CALL_EXPRESSION: return "CALL_EXPRESSION";
        case ASTNode::Type::LITERAL_EXPRESSION: return "LITERAL_EXPRESSION";
        case ASTNode::Type::DECLARATION_REFERENCE_EXPRESSION: return "DECLARATION_REFERENCE_EXPRESSION";
        case ASTNode::Type::IF_STATEMENT: return "IF_STATEMENT";
        case ASTNode::Type::FOR_STATEMENT: return "FOR_STATEMENT";
        case ASTNode::Type::WHILE_STATEMENT: return "WHILE_STATEMENT";
        case ASTNode::Type::DO_STATEMENT: return "DO_STATEMENT";
        case ASTNode::Type::ASSIGN_OPERATOR: return "ASSIGN_OPERATOR";
        case ASTNode::Type::UNARY_OPERATOR: return "UNARY_OPERATOR";
        case ASTNode::Type::BINARY_OPERATOR: return "BINARY_OPERATOR";
        case ASTNode::Type::TERNARY_OPERATOR: return "TERNARY_OPERATOR";
        default: return "unknwon";
    }
}

inline std::string semanticToString(ASTNode::Semantic semantic)
{
    switch (semantic)
    {
        case ASTNode::Semantic::NONE: return "NONE";
        case ASTNode::Semantic::BINORMAL: return "BINORMAL";
        case ASTNode::Semantic::BLEND_INDICES: return "BLEND_INDICES";
        case ASTNode::Semantic::BLEND_WEIGHT: return "BLEND_WEIGHT";
        case ASTNode::Semantic::COLOR: return "COLOR";
        case ASTNode::Semantic::NORMAL: return "NORMAL";
        case ASTNode::Semantic::POSITION: return "POSITION";
        case ASTNode::Semantic::POSITION_TRANSFORMED: return "POSITION_TRANSFORMED";
        case ASTNode::Semantic::POINT_SIZE: return "POINT_SIZE";
        case ASTNode::Semantic::TANGENT: return "TANGENT";
        case ASTNode::Semantic::TEXTURE_COORDINATES: return "TEXTURE_COORDINATES";
        default: return "unknwon";
    }
}

struct ASTContext
{
    bool parse(const std::vector<Token>& tokens);

    void dump();

private:
    std::unique_ptr<ASTNode> parseTopLevel(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);

    std::unique_ptr<ASTNode> parseStructDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseFieldDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseTypedefDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);

    std::unique_ptr<ASTNode> parseVarDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseFunctionDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);

    std::unique_ptr<ASTNode> parseIf(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseFor(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseWhile(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseDo(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseStatement(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);

    void dumpNode(const std::unique_ptr<ASTNode>& node, std::string indent = std::string());

    std::vector<std::unique_ptr<ASTNode>> nodes;
};
