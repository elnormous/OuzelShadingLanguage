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
        TYPE_DECLARATION,
        FIELD_DECLARATION,
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

    Type type = Type::NONE;
    std::vector<ASTNode> children;
};

inline std::string nodeTypeToString(ASTNode::Type type)
{
    switch (type)
    {
        case ASTNode::Type::NONE: return "NONE";
        case ASTNode::Type::TYPE_DECLARATION: return "TYPE_DECLARATION";
        case ASTNode::Type::FIELD_DECLARATION: return "FIELD_DECLARATION";
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

struct ASTContext
{
    bool parse(const std::vector<Token>& tokens);

    void dump();

private:
    bool parseTopLevel(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node);

    bool parseTypeDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node);
    bool parseVarDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node);
    bool parseFunctionDecl(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node);

    bool parseIf(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node);
    bool parseFor(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node);
    bool parseWhile(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node);
    bool parseDo(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node);
    bool parseStatement(const std::vector<Token>& tokens, std::vector<Token>::const_iterator& iterator, ASTNode& node);

    void dumpNode(const ASTNode& node, std::string indent = std::string());

    std::vector<ASTNode> nodes;
};
