//
//  OSL
//

#pragma once

#include <vector>
#include <string>
#include "Tokenizer.hpp"

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

class Construct
{
public:
    enum class Kind
    {
        NONE,
        DECLARATION,
        STATEMENT,
        EXPRESSION
    };

    Kind kind = Kind::NONE;
};

inline std::string constructKindToString(Construct::Kind kind)
{
    switch (kind)
    {
        case Construct::Kind::NONE: return "NONE";
        case Construct::Kind::DECLARATION: return "DECLARATION";
        case Construct::Kind::STATEMENT: return "STATEMENT";
        case Construct::Kind::EXPRESSION: return "EXPRESSION";
    }

    return "unknown";
}

inline std::string semanticToString(Semantic semantic)
{
    switch (semantic)
    {
        case Semantic::NONE: return "NONE";
        case Semantic::BINORMAL: return "BINORMAL";
        case Semantic::BLEND_INDICES: return "BLEND_INDICES";
        case Semantic::BLEND_WEIGHT: return "BLEND_WEIGHT";
        case Semantic::COLOR: return "COLOR";
        case Semantic::NORMAL: return "NORMAL";
        case Semantic::POSITION: return "POSITION";
        case Semantic::POSITION_TRANSFORMED: return "POSITION_TRANSFORMED";
        case Semantic::POINT_SIZE: return "POINT_SIZE";
        case Semantic::TANGENT: return "TANGENT";
        case Semantic::TEXTURE_COORDINATES: return "TEXTURE_COORDINATES";
        default: return "unknown";
    }
}

class TypeDeclaration;

class QualifiedType
{
public:
    TypeDeclaration* typeDeclaration = nullptr;
    bool isStatic = false;
    bool isConst = false;
    std::vector<uint32_t> dimensions;
};

class Statement: public Construct
{
public:
    enum class Kind
    {
        NONE,
        EMPTY,
        EXPRESSION,
        DECLARATION,
        COMPOUND,
        IF,
        FOR,
        SWITCH,
        CASE,
        WHILE,
        DO,
        BREAK,
        CONTINUE,
        RETURN,
    };

    Kind statementKind = Kind::NONE;
};

inline std::string statementKindToString(Statement::Kind kind)
{
    switch (kind)
    {
        case Statement::Kind::NONE: return "NONE";
        case Statement::Kind::EMPTY: return "EMPTY";
        case Statement::Kind::EXPRESSION: return "EXPRESSION";
        case Statement::Kind::DECLARATION: return "DECLARATION";
        case Statement::Kind::COMPOUND: return "COMPOUND";
        case Statement::Kind::IF: return "IF";
        case Statement::Kind::FOR: return "FOR";
        case Statement::Kind::SWITCH: return "SWITCH";
        case Statement::Kind::CASE: return "CASE";
        case Statement::Kind::WHILE: return "WHILE";
        case Statement::Kind::DO: return "DO";
        case Statement::Kind::BREAK: return "BREAK";
        case Statement::Kind::CONTINUE: return "CONTINUE";
        case Statement::Kind::RETURN: return "RETURN";
    }

    return "unknown";
}

class Expression: public Construct
{
public:
    enum class Kind
    {
        NONE,
        CALL,
        LITERAL,
        DECLARATION_REFERENCE,
        PAREN,
        MEMBER,
        ARRAY_SUBSCRIPT,
        UNARY,
        BINARY,
        TERNARY,
    };

    Kind expressionKind = Kind::NONE;

    std::string value;
    QualifiedType qualifiedType;
};

inline std::string expressionKindToString(Expression::Kind kind)
{
    switch (kind)
    {
        case Expression::Kind::NONE: return "NONE";
        case Expression::Kind::CALL: return "CALL";
        case Expression::Kind::LITERAL: return "LITERAL";
        case Expression::Kind::DECLARATION_REFERENCE: return "DECLARATION_REFERENCE";
        case Expression::Kind::PAREN: return "PAREN";
        case Expression::Kind::MEMBER: return "MEMBER";
        case Expression::Kind::ARRAY_SUBSCRIPT: return "ARRAY_SUBSCRIPT";
        case Expression::Kind::UNARY: return "UNARY";
        case Expression::Kind::BINARY: return "BINARY";
        case Expression::Kind::TERNARY: return "TERNARY";
    }

    return "unknown";
}

class Declaration: public Construct
{
public:
    enum class Kind
    {
        NONE,
        EMPTY,
        TYPE,
        FIELD,
        FUNCTION,
        VARIABLE,
        PARAMETER
    };

    Kind declarationKind = Kind::NONE;
};

class TypeDeclaration: public Declaration
{
public:
    enum class Kind
    {
        NONE,
        SIMPLE,
        STRUCT,
        //TYPE_DEFINITION, // typedef is not supported in GLSL
    };

    Kind typeKind = Kind::NONE;

    std::string name;
    bool isBuiltin = false;
};

inline std::string typeKindToString(TypeDeclaration::Kind kind)
{
    switch (kind)
    {
        case TypeDeclaration::Kind::NONE: return "NONE";
        case TypeDeclaration::Kind::SIMPLE: return "SIMPLE";
        case TypeDeclaration::Kind::STRUCT: return "STRUCT";
        //case TypeDeclaration::Kind::TYPE_DEFINITION: return "TYPE_DEFINITION";
    }

    return "unknown";
}

class SimpleTypeDeclaration: public TypeDeclaration
{
public:
    bool scalar = false;
};

class FieldDeclaration;

class StructDeclaration: public TypeDeclaration
{
public:
    std::vector<FieldDeclaration*> fieldDeclarations;
};

inline std::string declarationKindToString(Declaration::Kind kind)
{
    switch (kind)
    {
        case Declaration::Kind::NONE: return "NONE";
        case Declaration::Kind::EMPTY: return "EMPTY";
        case Declaration::Kind::TYPE: return "TYPE";
        case Declaration::Kind::FIELD: return "FIELD";
        case Declaration::Kind::FUNCTION: return "FUNCTION";
        case Declaration::Kind::VARIABLE: return "VARIABLE";
        case Declaration::Kind::PARAMETER: return "PARAMETER";
    }

    return "unknown";
}

class FieldDeclaration: public Declaration
{
public:
    StructDeclaration* structTypeDeclaration = nullptr;
    QualifiedType qualifiedType;
    FieldDeclaration* declaration = nullptr;
    std::string name;
    Semantic semantic = Semantic::NONE;
};

/*class TypeDefinitionDeclaration: public TypeDeclaration
{
public:
    QualifiedType qualifiedType;
};*/

class ParameterDeclaration: public Declaration
{
public:
    QualifiedType qualifiedType;
    std::string name;
};

class FunctionDeclaration: public Declaration
{
public:
    QualifiedType qualifiedType;
    std::string name;
    std::vector<ParameterDeclaration*> parameterDeclarations;
    Statement* body = nullptr;
};

class VariableDeclaration: public Declaration
{
public:
    QualifiedType qualifiedType;
    std::string name;
    Expression* initialization = nullptr;
};

class ExpressionStatement: public Statement
{
public:
    Expression* expression = nullptr;
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
    Construct* condition = nullptr;
    Statement* body = nullptr;
    Statement* elseBody = nullptr;
};

class ForStatement: public Statement
{
public:
    Construct* initialization = nullptr;
    Construct* condition = nullptr;
    Expression* increment = nullptr;
    Statement* body = nullptr;
};

class SwitchStatement: public Statement
{
public:
    Construct* condition = nullptr;
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
    Construct* condition = nullptr;
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
    Expression* result = nullptr;
};

class LiteralExpression: public Expression
{
public:
    TypeDeclaration* typeDeclaration = nullptr;
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
    std::vector<Expression*> parameters;
};

class ParenExpression: public Expression
{
public:
    Expression* expression = nullptr;
};

class MemberExpression: public Expression
{
public:
    Expression* expression = nullptr;
    FieldDeclaration* fieldDeclaration = nullptr;
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
    Expression* leftExpression = nullptr;
    Expression* rightExpression = nullptr;
};

class TernaryOperatorExpression: public Expression
{
public:
    Expression* condition;
    Expression* leftExpression = nullptr;
    Expression* rightExpression = nullptr;
};

class ASTContext
{
public:
    ASTContext();
    bool parse(const std::vector<Token>& tokens);

    void dump() const;

    std::vector<Declaration*> declarations;

private:
    bool checkToken(Token::Type tokenType,
                    const std::vector<Token>& tokens,
                    std::vector<Token>::const_iterator& iterator) const
    {
        return (iterator != tokens.end() && iterator->type == tokenType);
    }

    bool checkTokens(const std::vector<Token::Type>& tokenTypes,
                     const std::vector<Token>& tokens,
                     std::vector<Token>::const_iterator& iterator) const
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

    Declaration* findDeclaration(const std::string& name, std::vector<std::vector<Declaration*>>& declarationScopes) const
    {
        for (auto i = declarationScopes.crbegin(); i != declarationScopes.crend(); ++i)
        {
            for (Declaration* declaration : *i)
            {
                switch (declaration->declarationKind)
                {
                    case Declaration::Kind::TYPE:
                    {
                        TypeDeclaration* typeDeclaration = static_cast<TypeDeclaration*>(declaration);
                        if (typeDeclaration->name == name) return declaration;
                        break;
                    }
                    case Declaration::Kind::FUNCTION:
                    {
                        FunctionDeclaration* functionDeclaration = static_cast<FunctionDeclaration*>(declaration);
                        if (functionDeclaration->name == name) return declaration;
                        break;
                    }
                    case Declaration::Kind::VARIABLE:
                    {
                        VariableDeclaration* variableDeclaration = static_cast<VariableDeclaration*>(declaration);
                        if (variableDeclaration->name == name) return declaration;
                        break;
                    }
                    case Declaration::Kind::PARAMETER:
                    {
                        ParameterDeclaration* parameterDeclaration = static_cast<ParameterDeclaration*>(declaration);
                        if (parameterDeclaration->name == name) return declaration;
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        return nullptr;
    }

    TypeDeclaration* findTypeDeclaration(const std::string& name, std::vector<std::vector<Declaration*>>& declarationScopes) const
    {
        for (auto i = declarationScopes.crbegin(); i != declarationScopes.crend(); ++i)
        {
            for (Declaration* declaration : *i)
            {
                if (declaration->declarationKind == Declaration::Kind::TYPE)
                {
                    TypeDeclaration* typeDeclaration = static_cast<TypeDeclaration*>(declaration);
                    if (typeDeclaration->name == name) return typeDeclaration;
                }
            }
        }

        return nullptr;
    }

    FieldDeclaration* findFieldDeclaration(const std::string& name, StructDeclaration* structTypeDeclaration) const
    {
        for (FieldDeclaration* fieldDeclaration : structTypeDeclaration->fieldDeclarations)
        {
            if (fieldDeclaration->name == name) return fieldDeclaration;
        }

        return nullptr;
    }

    bool isDeclaration(const std::vector<Token>& tokens,
                       std::vector<Token>::const_iterator& iterator,
                       std::vector<std::vector<Declaration*>>& declarationScopes) const;

    Declaration* parseTopLevelDeclaration(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes);

    Declaration* parseDeclaration(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<Declaration*>>& declarationScopes);

    StructDeclaration* parseStructDeclaration(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarationScopes);

    FieldDeclaration* parseFieldDeclaration(const std::vector<Token>& tokens,
                                            std::vector<Token>::const_iterator& iterator,
                                            std::vector<std::vector<Declaration*>>& declarationScopes);

    /*TypeDefinitionDeclaration* parseTypeDefinitionDeclaration(const std::vector<Token>& tokens,
                                                              std::vector<Token>::const_iterator& iterator,
                                                              std::vector<std::vector<Declaration*>>& declarationScopes);*/

    FunctionDeclaration* parseFunctionDeclaration(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes);

    VariableDeclaration* parseVariableDeclaration(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes);

    Statement* parseStatement(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<Declaration*>>& declarationScopes);

    CompoundStatement* parseCompoundStatement(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarationScopes);

    IfStatement* parseIfStatement(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<Declaration*>>& declarationScopes);

    ForStatement* parseForStatement(const std::vector<Token>& tokens,
                                    std::vector<Token>::const_iterator& iterator,
                                    std::vector<std::vector<Declaration*>>& declarationScopes);

    SwitchStatement* parseSwitchStatement(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes);

    CaseStatement* parseCaseStatement(const std::vector<Token>& tokens,
                                      std::vector<Token>::const_iterator& iterator,
                                      std::vector<std::vector<Declaration*>>& declarationScopes);

    WhileStatement* parseWhileStatement(const std::vector<Token>& tokens,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<std::vector<Declaration*>>& declarationScopes);

    DoStatement* parseDoStatement(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parsePrimary(const std::vector<Token>& tokens,
                             std::vector<Token>::const_iterator& iterator,
                             std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseMember(const std::vector<Token>& tokens,
                            std::vector<Token>::const_iterator& iterator,
                            std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseSign(const std::vector<Token>& tokens,
                          std::vector<Token>::const_iterator& iterator,
                          std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseNot(const std::vector<Token>& tokens,
                         std::vector<Token>::const_iterator& iterator,
                         std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseMultiplication(const std::vector<Token>& tokens,
                                    std::vector<Token>::const_iterator& iterator,
                                    std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseAddition(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseLessThan(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseGreaterThan(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseEquality(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseTernary(const std::vector<Token>& tokens,
                             std::vector<Token>::const_iterator& iterator,
                             std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseAssignment(const std::vector<Token>& tokens,
                                std::vector<Token>::const_iterator& iterator,
                                std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseAdditionAssignment(const std::vector<Token>& tokens,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseMultiplicationAssignment(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseComma(const std::vector<Token>& tokens,
                           std::vector<Token>::const_iterator& iterator,
                           std::vector<std::vector<Declaration*>>& declarationScopes);

    void dumpDeclaration(const Declaration* declaration, std::string indent = std::string()) const;
    void dumpStatement(const Statement* statement, std::string indent = std::string()) const;
    void dumpExpression(const Expression* expression, std::string indent = std::string()) const;
    void dumpConstruct(const Construct* construct, std::string indent = std::string()) const;

    std::vector<std::unique_ptr<Construct>> constructs;
};
