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
        DECLARATION_STRUCT,
        DECLARATION_FIELD,
        DECLARATION_TYPE_DEFINITION,
        DECLARATION_FUNCTION,
        DECLARATION_VARIABLE,
        DECLARATION_PARAMETER,
        EXPRESSION_CALL,
        EXPRESSION_LITERAL,
        EXPRESSION_DECLARATION_REFERENCE,
        STATEMENT_COMPOUND,
        STATEMENT_IF,
        STATEMENT_FOR,
        STATEMENT_WHILE,
        STATEMENT_DO,
        STATEMENT_BREAK,
        STATEMENT_CONTINUE,
        STATEMENT_RETURN,
        OPERATOR_UNARY,
        OPERATOR_BINARY,
        OPERATOR_TERNARY,
    };

    enum class Attribute
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
    Attribute attribute = Attribute::NONE;
    std::string typeName;
    bool isConst = false;
    bool isInline = false;
    bool isExtern = false;
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> children;
};

inline std::string nodeTypeToString(ASTNode::Type type)
{
    switch (type)
    {
        case ASTNode::Type::NONE: return "NONE";
        case ASTNode::Type::DECLARATION_STRUCT: return "DECLARATION_STRUCT";
        case ASTNode::Type::DECLARATION_FIELD: return "DECLARATION_FIELD";
        case ASTNode::Type::DECLARATION_TYPE_DEFINITION: return "DECLARATION_TYPE_DEFINITION";
        case ASTNode::Type::DECLARATION_FUNCTION: return "DECLARATION_FUNCTION";
        case ASTNode::Type::DECLARATION_VARIABLE: return "DECLARATION_VARIABLE";
        case ASTNode::Type::DECLARATION_PARAMETER: return "DECLARATION_PARAMETER";
        case ASTNode::Type::STATEMENT_COMPOUND: return "STATEMENT_COMPOUND";
        case ASTNode::Type::STATEMENT_RETURN: return "STATEMENT_RETURN";
        case ASTNode::Type::EXPRESSION_CALL: return "EXPRESSION_CALL";
        case ASTNode::Type::EXPRESSION_LITERAL: return "EXPRESSION_LITERAL";
        case ASTNode::Type::EXPRESSION_DECLARATION_REFERENCE: return "EXPRESSION_DECLARATION_REFERENCE";
        case ASTNode::Type::STATEMENT_IF: return "STATEMENT_IF";
        case ASTNode::Type::STATEMENT_FOR: return "STATEMENT_FOR";
        case ASTNode::Type::STATEMENT_WHILE: return "STATEMENT_WHILE";
        case ASTNode::Type::STATEMENT_DO: return "STATEMENT_DO";
        case ASTNode::Type::OPERATOR_UNARY: return "OPERATOR_UNARY";
        case ASTNode::Type::OPERATOR_BINARY: return "OPERATOR_BINARY";
        case ASTNode::Type::OPERATOR_TERNARY: return "OPERATOR_TERNARY";
        default: return "unknwon";
    }
}

inline std::string attributeToString(ASTNode::Attribute attribute)
{
    switch (attribute)
    {
        case ASTNode::Attribute::NONE: return "NONE";
        case ASTNode::Attribute::BINORMAL: return "BINORMAL";
        case ASTNode::Attribute::BLEND_INDICES: return "BLEND_INDICES";
        case ASTNode::Attribute::BLEND_WEIGHT: return "BLEND_WEIGHT";
        case ASTNode::Attribute::COLOR: return "COLOR";
        case ASTNode::Attribute::NORMAL: return "NORMAL";
        case ASTNode::Attribute::POSITION: return "POSITION";
        case ASTNode::Attribute::POSITION_TRANSFORMED: return "POSITION_TRANSFORMED";
        case ASTNode::Attribute::POINT_SIZE: return "POINT_SIZE";
        case ASTNode::Attribute::TANGENT: return "TANGENT";
        case ASTNode::Attribute::TEXTURE_COORDINATES: return "TEXTURE_COORDINATES";
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
    std::unique_ptr<ASTNode> parseParamDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseCompoundStatement(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseStatement(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseFunctionDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);

    std::unique_ptr<ASTNode> parseIf(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseFor(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseWhile(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);
    std::unique_ptr<ASTNode> parseDo(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator);

    void dumpNode(const std::unique_ptr<ASTNode>& node, std::string indent = std::string());

    std::vector<std::unique_ptr<ASTNode>> nodes;
};
