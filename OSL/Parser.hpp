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

class Type
{
public:
    enum class Kind
    {
        NONE,
        BUILTIN,
        STRUCT
    };

    Kind typeKind = Kind::NONE;

    std::string name;
    TypeDeclaration* declaration = nullptr;
};

inline std::string typeKindToString(Type::Kind kind)
{
    switch (kind)
    {
        case Type::Kind::NONE: return "NONE";
        case Type::Kind::BUILTIN: return "BUILTIN";
        case Type::Kind::STRUCT: return "STRUCT";
    }

    return "unknown";
}

class QualifiedType
{
public:
    Type* type = nullptr;
    bool isStatic = false;
    bool isConst = false;
    bool isArray = false;
    uint32_t arraySize = 0;
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

class StructType;
class FieldDeclaration;

class Field
{
public:
    StructType* structType = nullptr;
    QualifiedType qualifiedType;
    FieldDeclaration* declaration = nullptr;
    std::string name;
    Semantic semantic = Semantic::NONE;
};

class SimpleType: public Type
{
public:
    bool scalar = false;
};

class StructType: public Type
{
public:
    std::vector<Field*> fields;
};

class Declaration: public Construct
{
public:
    enum class Kind
    {
        NONE,
        EMPTY,
        STRUCT,
        FIELD,
        //TYPE_DEFINITION, // typedef is not supported in GLSL
        FUNCTION,
        VARIABLE,
        PARAMETER
    };

    Kind declarationKind = Kind::NONE;
};

inline std::string declarationKindToString(Declaration::Kind kind)
{
    switch (kind)
    {
        case Declaration::Kind::NONE: return "NONE";
        case Declaration::Kind::EMPTY: return "EMPTY";
        case Declaration::Kind::STRUCT: return "STRUCT";
        case Declaration::Kind::FIELD: return "FIELD";
        //case Declaration::Kind::TYPE_DEFINITION: return "TYPE_DEFINITION";
        case Declaration::Kind::FUNCTION: return "FUNCTION";
        case Declaration::Kind::VARIABLE: return "VARIABLE";
        case Declaration::Kind::PARAMETER: return "PARAMETER";
    }

    return "unknown";
}

class TypeDeclaration: public Declaration
{
public:
    Type* type = nullptr;
};

class FieldDeclaration: public Declaration
{
public:
    Field* field = nullptr;
};

class StructDeclaration: public TypeDeclaration
{
public:
    Type* type;
    std::vector<FieldDeclaration*> fieldDeclarations;
};

/*class TypeDefinitionDeclaration: public TypeDeclaration
{
public:
    Type* type;
    std::string name;
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
    Field* field = nullptr;
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
        if (iterator != tokens.end() && iterator->type == tokenType)
        {
            ++iterator;
            return true;
        }

        return false;
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
                    case Declaration::Kind::STRUCT:
                    {
                        StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(declaration);
                        if (structDeclaration->type->name == name) return declaration;
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

    Type* findType(const std::string& name, std::vector<std::vector<Declaration*>>& declarationScopes) const
    {
        for (auto i = declarationScopes.crbegin(); i != declarationScopes.crend(); ++i)
        {
            for (Declaration* declaration : *i)
            {
                if (declaration->declarationKind == Declaration::Kind::STRUCT)
                {
                    StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(declaration);
                    if (structDeclaration->type->name == name) return structDeclaration->type;
                }
            }
        }

        for (auto i = types.cbegin(); i != types.cend(); ++i)
        {
            if (!(*i)->declaration && // built-in type
                (*i)->name == name) return (*i).get();
        }

        return nullptr;
    }

    Field* findField(const std::string& name, StructType* structType) const
    {
        for (Field* field : structType->fields)
        {
            if (field->name == name) return field;
        }

        return nullptr;
    }

    bool isDeclaration(const std::vector<Token>& tokens,
                       std::vector<Token>::const_iterator& iterator,
                       std::vector<std::vector<Declaration*>>& declarationScopes) const;

    Declaration* parseDeclaration(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<Declaration*>>& declarationScopes);

    StructDeclaration* parseStructDeclaration(const std::vector<Token>& tokens,
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

    ReturnStatement* parseReturnStatement(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseExpression(const std::vector<Token>& tokens,
                                std::vector<Token>::const_iterator& iterator,
                                std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseMultiplicationAssignment(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseAdditionAssignment(const std::vector<Token>& tokens,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseAssignment(const std::vector<Token>& tokens,
                                std::vector<Token>::const_iterator& iterator,
                                std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseTernary(const std::vector<Token>& tokens,
                             std::vector<Token>::const_iterator& iterator,
                             std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseEquality(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseGreaterThan(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseLessThan(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseAddition(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseMultiplication(const std::vector<Token>& tokens,
                                    std::vector<Token>::const_iterator& iterator,
                                    std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseNot(const std::vector<Token>& tokens,
                         std::vector<Token>::const_iterator& iterator,
                         std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseSign(const std::vector<Token>& tokens,
                          std::vector<Token>::const_iterator& iterator,
                          std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parseMember(const std::vector<Token>& tokens,
                            std::vector<Token>::const_iterator& iterator,
                            std::vector<std::vector<Declaration*>>& declarationScopes);

    Expression* parsePrimary(const std::vector<Token>& tokens,
                             std::vector<Token>::const_iterator& iterator,
                             std::vector<std::vector<Declaration*>>& declarationScopes);

    void dumpType(const Type* type, std::string indent = std::string()) const;
    void dumpField(const Field* field, std::string indent = std::string()) const;
    void dumpDeclaration(const Declaration* declaration, std::string indent = std::string()) const;
    void dumpStatement(const Statement* statement, std::string indent = std::string()) const;
    void dumpExpression(const Expression* expression, std::string indent = std::string()) const;
    void dumpConstruct(const Construct* construct, std::string indent = std::string()) const;

    std::vector<std::unique_ptr<Construct>> constructs;
    std::vector<std::unique_ptr<Type>> types;
    std::vector<std::unique_ptr<Field>> fields;
};
