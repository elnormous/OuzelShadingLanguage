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
#include "Tokenizer.hpp"

struct ASTNode
{
    enum class Type
    {
        NONE,
        TRANSLATION_UNIT,
        DECLARATION_EMPTY,
        DECLARATION_STRUCT,
        DECLARATION_FIELD,
        DECLARATION_TYPE_DEFINITION,
        DECLARATION_FUNCTION,
        DECLARATION_VARIABLE,
        DECLARATION_PARAMETER,
        EXPRESSION_CALL,
        EXPRESSION_LITERAL,
        EXPRESSION_DECLARATION_REFERENCE,
        STATEMENT_DECLARATION,
        STATEMENT_COMPOUND,
        STATEMENT_IF,
        STATEMENT_FOR,
        STATEMENT_SWITCH,
        STATEMENT_CASE,
        STATEMENT_WHILE,
        STATEMENT_DO,
        STATEMENT_BREAK,
        STATEMENT_CONTINUE,
        STATEMENT_RETURN,
        STATEMENT_EXPRESSION,
        OPERATOR_UNARY,
        OPERATOR_BINARY,
        OPERATOR_TERNARY,
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
    bool isStatic = false;
    bool isConst = false;
    std::string typeName;
    std::string name;
    std::string value;
    std::vector<std::unique_ptr<ASTNode>> children;
};

inline std::string nodeTypeToString(ASTNode::Type type)
{
    switch (type)
    {
        case ASTNode::Type::NONE: return "NONE";
        case ASTNode::Type::TRANSLATION_UNIT: return "TRANSLATION_UNIT";
        case ASTNode::Type::DECLARATION_EMPTY: return "DECLARATION_EMPTY";
        case ASTNode::Type::DECLARATION_STRUCT: return "DECLARATION_STRUCT";
        case ASTNode::Type::DECLARATION_FIELD: return "DECLARATION_FIELD";
        case ASTNode::Type::DECLARATION_TYPE_DEFINITION: return "DECLARATION_TYPE_DEFINITION";
        case ASTNode::Type::DECLARATION_FUNCTION: return "DECLARATION_FUNCTION";
        case ASTNode::Type::DECLARATION_VARIABLE: return "DECLARATION_VARIABLE";
        case ASTNode::Type::DECLARATION_PARAMETER: return "DECLARATION_PARAMETER";
        case ASTNode::Type::EXPRESSION_CALL: return "EXPRESSION_CALL";
        case ASTNode::Type::EXPRESSION_LITERAL: return "EXPRESSION_LITERAL";
        case ASTNode::Type::EXPRESSION_DECLARATION_REFERENCE: return "EXPRESSION_DECLARATION_REFERENCE";
        case ASTNode::Type::STATEMENT_DECLARATION: return "STATEMENT_DECLARATION";
        case ASTNode::Type::STATEMENT_COMPOUND: return "STATEMENT_COMPOUND";
        case ASTNode::Type::STATEMENT_IF: return "STATEMENT_IF";
        case ASTNode::Type::STATEMENT_FOR: return "STATEMENT_FOR";
        case ASTNode::Type::STATEMENT_WHILE: return "STATEMENT_WHILE";
        case ASTNode::Type::STATEMENT_DO: return "STATEMENT_DO";
        case ASTNode::Type::STATEMENT_BREAK: return "STATEMENT_BREAK";
        case ASTNode::Type::STATEMENT_CONTINUE: return "STATEMENT_CONTINUE";
        case ASTNode::Type::STATEMENT_RETURN: return "STATEMENT_RETURN";
        case ASTNode::Type::STATEMENT_EXPRESSION: return "STATEMENT_EXPRESSION";
        case ASTNode::Type::OPERATOR_UNARY: return "OPERATOR_UNARY";
        case ASTNode::Type::OPERATOR_BINARY: return "OPERATOR_BINARY";
        case ASTNode::Type::OPERATOR_TERNARY: return "OPERATOR_TERNARY";
        default: return "unknown";
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
        default: return "unknown";
    }
}

struct ASTContext
{
    bool parse(const std::vector<Token>& tokens);

    void dump();

private:
    bool check(Token::Type tokenType,
               const std::vector<Token>& tokens,
               std::vector<Token>::const_iterator& iterator)
    {
        if (iterator != tokens.end() && iterator->type == tokenType)
        {
            ++iterator;
            return true;
        }

        return false;
    }

    bool check(const std::vector<Token::Type>& tokenTypes,
               const std::vector<Token>& tokens,
               std::vector<Token>::const_iterator& iterator)
    {
        if (iterator == tokens.end()) return false;

        for (Token::Type tokenType : tokenTypes)
        {
            if (iterator->type == tokenType)
            {
                ++iterator;
                return true;
            }
        }

        return false;
    }

    bool parseTopLevel(const std::vector<Token>& tokens,
                       std::vector<Token>::const_iterator& iterator,
                       std::vector<std::vector<ASTNode*>>& declarations,
                       std::unique_ptr<ASTNode>& result);

    bool parseStructDecl(const std::vector<Token>& tokens,
                         std::vector<Token>::const_iterator& iterator,
                         std::vector<std::vector<ASTNode*>>& declarations,
                         std::unique_ptr<ASTNode>& result);

    bool parseTypedefDecl(const std::vector<Token>& tokens,
                          std::vector<Token>::const_iterator& iterator,
                          std::vector<std::vector<ASTNode*>>& declarations,
                          std::unique_ptr<ASTNode>& result);

    bool parseFunctionDecl(const std::vector<Token>& tokens,
                           std::vector<Token>::const_iterator& iterator,
                           std::vector<std::vector<ASTNode*>>& declarations,
                           std::unique_ptr<ASTNode>& result);

    bool parseVariableDecl(const std::vector<Token>& tokens,
                           std::vector<Token>::const_iterator& iterator,
                           std::vector<std::vector<ASTNode*>>& declarations,
                           std::unique_ptr<ASTNode>& result);

    bool parseCompoundStatement(const std::vector<Token>& tokens,
                                std::vector<Token>::const_iterator& iterator,
                                std::vector<std::vector<ASTNode*>>& declarations,
                                std::unique_ptr<ASTNode>& result);

    bool parseStatement(const std::vector<Token>& tokens,
                        std::vector<Token>::const_iterator& iterator,
                        std::vector<std::vector<ASTNode*>>& declarations,
                        std::unique_ptr<ASTNode>& result);

    bool parseExpression(const std::vector<Token>& tokens,
                         std::vector<Token>::const_iterator& iterator,
                         std::vector<std::vector<ASTNode*>>& declarations,
                         std::unique_ptr<ASTNode>& result);

    bool parseEquality(const std::vector<Token>& tokens,
                       std::vector<Token>::const_iterator& iterator,
                       std::vector<std::vector<ASTNode*>>& declarations,
                       std::unique_ptr<ASTNode>& result);

    bool parseComparison(const std::vector<Token>& tokens,
                         std::vector<Token>::const_iterator& iterator,
                         std::vector<std::vector<ASTNode*>>& declarations,
                         std::unique_ptr<ASTNode>& result);

    bool parseAddition(const std::vector<Token>& tokens,
                       std::vector<Token>::const_iterator& iterator,
                       std::vector<std::vector<ASTNode*>>& declarations,
                       std::unique_ptr<ASTNode>& result);

    bool parseMultiplication(const std::vector<Token>& tokens,
                             std::vector<Token>::const_iterator& iterator,
                             std::vector<std::vector<ASTNode*>>& declarations,
                             std::unique_ptr<ASTNode>& result);

    bool parseUnary(const std::vector<Token>& tokens,
                    std::vector<Token>::const_iterator& iterator,
                    std::vector<std::vector<ASTNode*>>& declarations,
                    std::unique_ptr<ASTNode>& result);

    bool parsePrimary(const std::vector<Token>& tokens,
                      std::vector<Token>::const_iterator& iterator,
                      std::vector<std::vector<ASTNode*>>& declarations,
                      std::unique_ptr<ASTNode>& result);

    void dumpNode(const std::unique_ptr<ASTNode>& node, std::string indent = std::string());

    std::unique_ptr<ASTNode> translationUnit;
};
