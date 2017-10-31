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

class Construct
{
public:
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
    std::vector<std::unique_ptr<Construct>> children;
    Construct* reference = nullptr;
};

inline std::string nodeKindToString(Construct::Kind type)
{
    switch (type)
    {
        case Construct::Kind::NONE: return "NONE";
        case Construct::Kind::TRANSLATION_UNIT: return "TRANSLATION_UNIT";
        case Construct::Kind::DECLARATION_EMPTY: return "DECLARATION_EMPTY";
        case Construct::Kind::DECLARATION_STRUCT: return "DECLARATION_STRUCT";
        case Construct::Kind::DECLARATION_FIELD: return "DECLARATION_FIELD";
        //case Construct::Kind::DECLARATION_TYPE_DEFINITION: return "DECLARATION_TYPE_DEFINITION";
        case Construct::Kind::DECLARATION_FUNCTION: return "DECLARATION_FUNCTION";
        case Construct::Kind::DECLARATION_VARIABLE: return "DECLARATION_VARIABLE";
        case Construct::Kind::DECLARATION_PARAMETER: return "DECLARATION_PARAMETER";
        case Construct::Kind::STATEMENT_DECLARATION: return "STATEMENT_DECLARATION";
        case Construct::Kind::STATEMENT_COMPOUND: return "STATEMENT_COMPOUND";
        case Construct::Kind::STATEMENT_IF: return "STATEMENT_IF";
        case Construct::Kind::STATEMENT_FOR: return "STATEMENT_FOR";
        case Construct::Kind::STATEMENT_SWITCH: return "STATEMENT_SWITCH";
        case Construct::Kind::STATEMENT_CASE: return "STATEMENT_CASE";
        case Construct::Kind::STATEMENT_WHILE: return "STATEMENT_WHILE";
        case Construct::Kind::STATEMENT_DO: return "STATEMENT_DO";
        case Construct::Kind::STATEMENT_BREAK: return "STATEMENT_BREAK";
        case Construct::Kind::STATEMENT_CONTINUE: return "STATEMENT_CONTINUE";
        case Construct::Kind::STATEMENT_RETURN: return "STATEMENT_RETURN";
        case Construct::Kind::STATEMENT_EXPRESSION: return "STATEMENT_EXPRESSION";
        case Construct::Kind::EXPRESSION_CALL: return "EXPRESSION_CALL";
        case Construct::Kind::EXPRESSION_LITERAL: return "EXPRESSION_LITERAL";
        case Construct::Kind::EXPRESSION_DECLARATION_REFERENCE: return "EXPRESSION_DECLARATION_REFERENCE";
        case Construct::Kind::EXPRESSION_PAREN: return "EXPRESSION_PAREN";
        case Construct::Kind::EXPRESSION_MEMBER: return "EXPRESSION_MEMBER";
        case Construct::Kind::EXPRESSION_ARRAY_SUBSCRIPT: return "EXPRESSION_ARRAY_SUBSCRIPT";
        case Construct::Kind::OPERATOR_UNARY: return "OPERATOR_UNARY";
        case Construct::Kind::OPERATOR_BINARY: return "OPERATOR_BINARY";
        case Construct::Kind::OPERATOR_TERNARY: return "OPERATOR_TERNARY";
    }

    return "unknown";
}

inline std::string semanticToString(Construct::Semantic semantic)
{
    switch (semantic)
    {
        case Construct::Semantic::NONE: return "NONE";
        case Construct::Semantic::BINORMAL: return "BINORMAL";
        case Construct::Semantic::BLEND_INDICES: return "BLEND_INDICES";
        case Construct::Semantic::BLEND_WEIGHT: return "BLEND_WEIGHT";
        case Construct::Semantic::COLOR: return "COLOR";
        case Construct::Semantic::NORMAL: return "NORMAL";
        case Construct::Semantic::POSITION: return "POSITION";
        case Construct::Semantic::POSITION_TRANSFORMED: return "POSITION_TRANSFORMED";
        case Construct::Semantic::POINT_SIZE: return "POINT_SIZE";
        case Construct::Semantic::TANGENT: return "TANGENT";
        case Construct::Semantic::TEXTURE_COORDINATES: return "TEXTURE_COORDINATES";
        default: return "unknown";
    }
}

class Type: public Construct
{
public:
    std::string name;
};

class Field: public Construct
{
public:
    Type* type = nullptr;
    std::string name;
};

class StructType: public Type
{
public:
    std::vector<Field*> fields;
};


class Declaration: public Construct
{
public:
};

class TranslationUnit: public Construct
{
public:
    std::vector<Declaration*> declarations;
};

class TypeDeclaration: public Declaration
{
public:
    Type* type = nullptr;
};

class FieldDeclaration: public Construct
{
public:
    Field* field = nullptr;
    Type* type = nullptr;
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
    Type* type = nullptr;
    std::string name;
};

class FunctionDeclaration: public Declaration
{
public:
    Type* resultType = nullptr;
    std::string name;
};

class VariableDeclaration: public Declaration
{
public:
    Type* type = nullptr;
    std::string name;
};

class Statement: public Construct
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
    Declaration* declaration = nullptr;
};

class CompoundStatement: public Statement
{
public:
    std::vector<Statement*> statements;
};

class IfStatement: public Statement
{
public:
    Statement* condition = nullptr;
    Statement* truePart = nullptr;
    Statement* falsePart = nullptr;
};

class ForStatement: public Statement
{
public:
    Statement* condition = nullptr;
    Statement* body = nullptr;
};

class SwitchStatement: public Statement
{
public:
    Statement* condition = nullptr;
    Statement* body = nullptr;
};

class CaseStatement: public Statement
{
public:
    Expression* condition = nullptr;
    Statement* body = nullptr;
};

class WhileStatement: public Statement
{
public:
    Expression* condition = nullptr;
    Statement* body = nullptr;
};

class DoStatement: public Statement
{
public:
    Expression* condition = nullptr;
    Statement* body = nullptr;
};

class BreakStatement: public Statement
{
public:
};

class ContinueStatement: public Statement
{
public:
};

class ReturnStatement: public Statement
{
public:
    Statement* result = nullptr;
};

class LiteralExpression: public Expression
{
public:
    Type* type = nullptr;
    std::string value;
};

class DeclarationReferenceExpression: public Expression
{
public:
    Declaration* declaration = nullptr;
};

class CallExpression: public Expression
{
public:
    DeclarationReferenceExpression* declarationReference = nullptr;
    std::vector<ParameterDeclaration*> parameters;
};

class ParenExpression: public Expression
{
public:
    Expression* expression = nullptr;
};

class MemberExpression: public Expression
{
public:
    DeclarationReferenceExpression* declarationReference = nullptr;
};

class ArraySubscriptExpression: public Expression
{
public:
    DeclarationReferenceExpression* declarationReference = nullptr;
    Expression* expression = nullptr;
};

class UnaryOperatorExpression: public Expression
{
public:
    Expression* expression = nullptr;
};

class BinaryOperatorExpression: public Expression
{
public:
    Expression* leftExpressions = nullptr;
    Expression* rightExpressions = nullptr;
};

class TernarySubscriptExpression: public Expression
{
public:
    Expression* condition;
    Expression* leftExpressions = nullptr;
    Expression* rightExpressions = nullptr;
};

class ASTContext
{
public:
    ASTContext();
    bool parse(const std::vector<Token>& tokens);

    const TranslationUnit* getTranslationUnit() const { return translationUnit.get(); }

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

    Construct* findDeclaration(const std::string& name, std::vector<std::vector<Construct*>>& declarations)
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

    Construct* findField(const std::string& name, Construct* declaration)
    {
        for (auto field = declaration->children.cbegin(); field != declaration->children.cend(); ++field)
        {
            if ((*field)->name == name) return field->get();
        }

        return nullptr;
    }

    bool parseTopLevel(const std::vector<Token>& tokens,
                       std::vector<Token>::const_iterator& iterator,
                       std::vector<std::vector<Construct*>>& declarations,
                       std::unique_ptr<TranslationUnit>& result);

    bool parseStructDecl(const std::vector<Token>& tokens,
                         std::vector<Token>::const_iterator& iterator,
                         std::vector<std::vector<Construct*>>& declarations,
                         std::unique_ptr<Construct>& result);

    bool parseTypedefDecl(const std::vector<Token>& tokens,
                          std::vector<Token>::const_iterator& iterator,
                          std::vector<std::vector<Construct*>>& declarations,
                          std::unique_ptr<Construct>& result);

    bool parseFunctionDecl(const std::vector<Token>& tokens,
                           std::vector<Token>::const_iterator& iterator,
                           std::vector<std::vector<Construct*>>& declarations,
                           std::unique_ptr<Construct>& result);

    bool parseVariableDecl(const std::vector<Token>& tokens,
                           std::vector<Token>::const_iterator& iterator,
                           std::vector<std::vector<Construct*>>& declarations,
                           std::unique_ptr<Construct>& result);

    bool parseCompoundStatement(const std::vector<Token>& tokens,
                                std::vector<Token>::const_iterator& iterator,
                                std::vector<std::vector<Construct*>>& declarations,
                                std::unique_ptr<Construct>& result);

    bool parseStatement(const std::vector<Token>& tokens,
                        std::vector<Token>::const_iterator& iterator,
                        std::vector<std::vector<Construct*>>& declarations,
                        std::unique_ptr<Construct>& result);

    bool parseExpression(const std::vector<Token>& tokens,
                         std::vector<Token>::const_iterator& iterator,
                         std::vector<std::vector<Construct*>>& declarations,
                         std::unique_ptr<Construct>& result);

    bool parseMultiplicationAssignment(const std::vector<Token>& tokens,
                                       std::vector<Token>::const_iterator& iterator,
                                       std::vector<std::vector<Construct*>>& declarations,
                                       std::unique_ptr<Construct>& result);

    bool parseAdditionAssignment(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::vector<Construct*>>& declarations,
                                 std::unique_ptr<Construct>& result);

    bool parseAssignment(const std::vector<Token>& tokens,
                         std::vector<Token>::const_iterator& iterator,
                         std::vector<std::vector<Construct*>>& declarations,
                         std::unique_ptr<Construct>& result);

    bool parseTernary(const std::vector<Token>& tokens,
                      std::vector<Token>::const_iterator& iterator,
                      std::vector<std::vector<Construct*>>& declarations,
                      std::unique_ptr<Construct>& result);

    bool parseEquality(const std::vector<Token>& tokens,
                       std::vector<Token>::const_iterator& iterator,
                       std::vector<std::vector<Construct*>>& declarations,
                       std::unique_ptr<Construct>& result);

    bool parseGreaterThan(const std::vector<Token>& tokens,
                          std::vector<Token>::const_iterator& iterator,
                          std::vector<std::vector<Construct*>>& declarations,
                          std::unique_ptr<Construct>& result);

    bool parseLessThan(const std::vector<Token>& tokens,
                       std::vector<Token>::const_iterator& iterator,
                       std::vector<std::vector<Construct*>>& declarations,
                       std::unique_ptr<Construct>& result);

    bool parseAddition(const std::vector<Token>& tokens,
                       std::vector<Token>::const_iterator& iterator,
                       std::vector<std::vector<Construct*>>& declarations,
                       std::unique_ptr<Construct>& result);

    bool parseMultiplication(const std::vector<Token>& tokens,
                             std::vector<Token>::const_iterator& iterator,
                             std::vector<std::vector<Construct*>>& declarations,
                             std::unique_ptr<Construct>& result);

    bool parseUnary(const std::vector<Token>& tokens,
                    std::vector<Token>::const_iterator& iterator,
                    std::vector<std::vector<Construct*>>& declarations,
                    std::unique_ptr<Construct>& result);

    bool parseMember(const std::vector<Token>& tokens,
                     std::vector<Token>::const_iterator& iterator,
                     std::vector<std::vector<Construct*>>& declarations,
                     std::unique_ptr<Construct>& result);

    bool parsePrimary(const std::vector<Token>& tokens,
                      std::vector<Token>::const_iterator& iterator,
                      std::vector<std::vector<Construct*>>& declarations,
                      std::unique_ptr<Construct>& result);

    void dumpNode(const Construct* node, std::string indent = std::string());
    
    std::unique_ptr<TranslationUnit> translationUnit;
    std::vector<std::unique_ptr<Construct>> builtinDeclarations;
};
