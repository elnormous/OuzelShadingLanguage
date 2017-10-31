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
    enum class Kind
    {
        NONE,
        TRANSLATION_UNIT,
        DECLARATION_EMPTY,
        DECLARATION_STRUCT,
        DECLARATION_FIELD,
        // DECLARATION_TYPE_DEFINITION, // typedef is not supported in GLSL
        DECLARATION_FUNCTION,
        DECLARATION_VARIABLE,
        DECLARATION_PARAMETER,
        EXPRESSION_CALL,
        EXPRESSION_LITERAL,
        EXPRESSION_DECLARATION_REFERENCE,
        EXPRESSION_PAREN,
        EXPRESSION_MEMBER,
        EXPRESSION_ARRAY_SUBSCRIPT,
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

    Kind kind = Kind::NONE;
    Semantic semantic = Semantic::NONE;
    bool isStatic = false;
    bool isConst = false;
    std::string name;
    std::string value;
    std::vector<std::unique_ptr<ASTNode>> children;
    ASTNode* reference = nullptr;
};

inline std::string nodeKindToString(ASTNode::Kind type)
{
    switch (type)
    {
        case ASTNode::Kind::NONE: return "NONE";
        case ASTNode::Kind::TRANSLATION_UNIT: return "TRANSLATION_UNIT";
        case ASTNode::Kind::DECLARATION_EMPTY: return "DECLARATION_EMPTY";
        case ASTNode::Kind::DECLARATION_STRUCT: return "DECLARATION_STRUCT";
        case ASTNode::Kind::DECLARATION_FIELD: return "DECLARATION_FIELD";
        //case ASTNode::Kind::DECLARATION_TYPE_DEFINITION: return "DECLARATION_TYPE_DEFINITION";
        case ASTNode::Kind::DECLARATION_FUNCTION: return "DECLARATION_FUNCTION";
        case ASTNode::Kind::DECLARATION_VARIABLE: return "DECLARATION_VARIABLE";
        case ASTNode::Kind::DECLARATION_PARAMETER: return "DECLARATION_PARAMETER";
        case ASTNode::Kind::STATEMENT_DECLARATION: return "STATEMENT_DECLARATION";
        case ASTNode::Kind::STATEMENT_COMPOUND: return "STATEMENT_COMPOUND";
        case ASTNode::Kind::STATEMENT_IF: return "STATEMENT_IF";
        case ASTNode::Kind::STATEMENT_FOR: return "STATEMENT_FOR";
        case ASTNode::Kind::STATEMENT_SWITCH: return "STATEMENT_SWITCH";
        case ASTNode::Kind::STATEMENT_CASE: return "STATEMENT_CASE";
        case ASTNode::Kind::STATEMENT_WHILE: return "STATEMENT_WHILE";
        case ASTNode::Kind::STATEMENT_DO: return "STATEMENT_DO";
        case ASTNode::Kind::STATEMENT_BREAK: return "STATEMENT_BREAK";
        case ASTNode::Kind::STATEMENT_CONTINUE: return "STATEMENT_CONTINUE";
        case ASTNode::Kind::STATEMENT_RETURN: return "STATEMENT_RETURN";
        case ASTNode::Kind::STATEMENT_EXPRESSION: return "STATEMENT_EXPRESSION";
        case ASTNode::Kind::EXPRESSION_CALL: return "EXPRESSION_CALL";
        case ASTNode::Kind::EXPRESSION_LITERAL: return "EXPRESSION_LITERAL";
        case ASTNode::Kind::EXPRESSION_DECLARATION_REFERENCE: return "EXPRESSION_DECLARATION_REFERENCE";
        case ASTNode::Kind::EXPRESSION_PAREN: return "EXPRESSION_PAREN";
        case ASTNode::Kind::EXPRESSION_MEMBER: return "EXPRESSION_MEMBER";
        case ASTNode::Kind::EXPRESSION_ARRAY_SUBSCRIPT: return "EXPRESSION_ARRAY_SUBSCRIPT";
        case ASTNode::Kind::OPERATOR_UNARY: return "OPERATOR_UNARY";
        case ASTNode::Kind::OPERATOR_BINARY: return "OPERATOR_BINARY";
        case ASTNode::Kind::OPERATOR_TERNARY: return "OPERATOR_TERNARY";
    }

    return "unknown";
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

class ASTContext
{
public:
    ASTContext();
    bool parse(const std::vector<Token>& tokens);

    const std::unique_ptr<ASTNode>& getTranslationUnit() const { return translationUnit; }

    void dump();

private:
    bool checkToken(Token::Type tokenType,
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

    bool checkTokens(const std::vector<Token::Type>& tokenTypes,
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

    ASTNode* findDeclaration(const std::string& name, std::vector<std::vector<ASTNode*>>& declarations)
    {
        for (auto i = declarations.crbegin(); i != declarations.crend(); ++i)
        {
            for (auto declaration = i->cbegin(); declaration != i->cend(); ++declaration)
            {
                if ((*declaration)->name == name) return *declaration;
            }
        }

        for (auto i = builtinDeclarations.cbegin(); i != builtinDeclarations.cend(); ++i)
        {
            if ((*i)->name == name) return (*i).get();
        }

        return nullptr;
    }

    ASTNode* findField(const std::string& name, ASTNode* declaration)
    {
        for (auto field = declaration->children.cbegin(); field != declaration->children.cend(); ++field)
        {
            if ((*field)->name == name) return field->get();
        }

        return nullptr;
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

    bool parseMultiplicationAssignment(const std::vector<Token>& tokens,
                                       std::vector<Token>::const_iterator& iterator,
                                       std::vector<std::vector<ASTNode*>>& declarations,
                                       std::unique_ptr<ASTNode>& result);

    bool parseAdditionAssignment(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::vector<ASTNode*>>& declarations,
                                 std::unique_ptr<ASTNode>& result);

    bool parseAssignment(const std::vector<Token>& tokens,
                         std::vector<Token>::const_iterator& iterator,
                         std::vector<std::vector<ASTNode*>>& declarations,
                         std::unique_ptr<ASTNode>& result);

    bool parseTernary(const std::vector<Token>& tokens,
                      std::vector<Token>::const_iterator& iterator,
                      std::vector<std::vector<ASTNode*>>& declarations,
                      std::unique_ptr<ASTNode>& result);

    bool parseEquality(const std::vector<Token>& tokens,
                       std::vector<Token>::const_iterator& iterator,
                       std::vector<std::vector<ASTNode*>>& declarations,
                       std::unique_ptr<ASTNode>& result);

    bool parseGreaterThan(const std::vector<Token>& tokens,
                          std::vector<Token>::const_iterator& iterator,
                          std::vector<std::vector<ASTNode*>>& declarations,
                          std::unique_ptr<ASTNode>& result);

    bool parseLessThan(const std::vector<Token>& tokens,
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

    bool parseMember(const std::vector<Token>& tokens,
                     std::vector<Token>::const_iterator& iterator,
                     std::vector<std::vector<ASTNode*>>& declarations,
                     std::unique_ptr<ASTNode>& result);

    bool parsePrimary(const std::vector<Token>& tokens,
                      std::vector<Token>::const_iterator& iterator,
                      std::vector<std::vector<ASTNode*>>& declarations,
                      std::unique_ptr<ASTNode>& result);

    void dumpNode(const std::unique_ptr<ASTNode>& node, std::string indent = std::string());

    std::unique_ptr<ASTNode> translationUnit;
    std::vector<std::unique_ptr<ASTNode>> builtinDeclarations;
};

class Type
{
public:
    std::string name;
};

class Field
{
public:
    Type* type;
    std::string name;
};

class StructType: public Type
{
public:
    std::vector<Field*> fields;
};


class Declaration: public ASTNode
{
public:
};

class TranslationUnit: public ASTNode
{
public:
    std::vector<Declaration*> declarations;
};

class TypeDeclaration: public Declaration
{
public:
    Type* type;
};

class FieldDeclaration: public ASTNode
{
public:
    Field* field;
    Type* type;
    std::string name;
};

class StructureDeclaration: public TypeDeclaration
{
public:
    std::string name;
    std::vector<FieldDeclaration*> fieldDeclarations;
};

class ParameterDeclaration: public Declaration
{
public:
    Type* type;
    std::string name;
};

class FunctionDeclaration: public Declaration
{
public:
    Type* resultType;
    std::string name;
};

class VariableDeclaration: public Declaration
{
public:
    Type* type;
    std::string name;
};

class Statement: public ASTNode
{
public:
};

class Expression: public Statement
{
public:
};

class DeclarationStatement: public Statement
{
public:
    Declaration* declaration;
};

class CompoundStatement: public Statement
{
public:
    std::vector<Statement*> statements;
};

class IfDeclaration: public Statement
{
public:
    Statement* condition;
    Statement* ifPart;
    Statement* thenPart;
};

class ForDeclaration: public Statement
{
public:
    Statement* condition;
    Statement* body;
};

class SwitchDeclaration: public Statement
{
public:
    Statement* condition;
    Statement* body;
};

class CaseDeclaration: public Statement
{
public:
    Expression* condition;
    Statement* body;
};

class WhileDeclaration: public Statement
{
public:
    Expression* condition;
    Statement* body;
};

class DoDeclaration: public Statement
{
public:
    Expression* condition;
    Statement* body;
};

class BreakDeclaration: public Statement
{
public:
};

class ContinueDeclaration: public Statement
{
public:
};

class ReturnDeclaration: public Statement
{
public:
    Statement* result;
};

class CallExpression: public Expression
{
public:
};

class LiteralExpression: public Expression
{
public:
};

class DeclarationReferenceExpression: public Expression
{
public:
};

class ParenExpression: public Expression
{
public:
};

class MemberExpression: public Expression
{
public:
};

class ArraySubscriptExpression: public Expression
{
public:
};

class UnaryOperatorExpression: public Expression
{
public:
    Expression* expression;
};

class BinaryOperatorExpression: public Expression
{
public:
    Expression* expressions[2];
};

class TernarySubscriptExpression: public Expression
{
public:
    Expression* condition;
    Expression* expressions[2];
};
