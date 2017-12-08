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

    std::string name;
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

    bool isStatic = false;
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
    enum class Program
    {
        NONE,
        FRAGMENT,
        VERTEX
    };

    QualifiedType qualifiedType;
    std::vector<ParameterDeclaration*> parameterDeclarations;
    Statement* body = nullptr;

    bool isStatic = false;
    Program program = Program::NONE;
};

inline std::string programToString(FunctionDeclaration::Program program)
{
    switch (program)
    {
        case FunctionDeclaration::Program::NONE: return "NONE";
        case FunctionDeclaration::Program::FRAGMENT: return "FRAGMENT";
        case FunctionDeclaration::Program::VERTEX: return "VERTEX";
    }

    return "unknown";
}

class VariableDeclaration: public Declaration
{
public:
    QualifiedType qualifiedType;
    Expression* initialization = nullptr;

    bool isStatic = false;
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
    enum class Kind
    {
        NONE,
        NEGATION, // !
        POSITIVE, // +
        NEGATIVE // -
    };

    Expression* expression = nullptr;

    Kind operatorKind = Kind::NONE;
};

inline std::string unaryOperatorKindToString(UnaryOperatorExpression::Kind kind)
{
    switch (kind)
    {
        case UnaryOperatorExpression::Kind::NONE: return "NONE";
        case UnaryOperatorExpression::Kind::NEGATION: return "NEGATION";
        case UnaryOperatorExpression::Kind::POSITIVE: return "POSITIVE";
        case UnaryOperatorExpression::Kind::NEGATIVE: return "NEGATIVE";
    }

    return "unknown";
}

class BinaryOperatorExpression: public Expression
{
public:
    enum class Kind
    {
        NONE,
        ADDITION, // +
        SUBTRACTION, // -
        MULTIPLICATION, // *
        DIVISION, // /
        ADDITION_ASSIGNMENT, // +=
        SUBTRACTION_ASSIGNMENT, // -=
        MULTIPLICATION_ASSIGNMENT, // *=
        DIVISION_ASSIGNMENT, // /=
        LESS_THAN, // <
        LESS_THAN_EQUAL, // <=
        GREATER_THAN, // >
        GREATER_THAN_EQUAL, // >=
        EQUALITY, // ==
        INEQUALITY, // !=
        ASSIGNMENT, // =
        COMMA, // ,
    };

    Expression* leftExpression = nullptr;
    Expression* rightExpression = nullptr;

    Kind operatorKind = Kind::NONE;
};

inline std::string binaryOperatorKindToString(BinaryOperatorExpression::Kind kind)
{
    switch (kind)
    {
        case BinaryOperatorExpression::Kind::NONE: return "NONE";
        case BinaryOperatorExpression::Kind::ADDITION: return "ADDITION";
        case BinaryOperatorExpression::Kind::SUBTRACTION: return "SUBTRACTION";
        case BinaryOperatorExpression::Kind::MULTIPLICATION: return "MULTIPLICATION";
        case BinaryOperatorExpression::Kind::DIVISION: return "DIVISION";
        case BinaryOperatorExpression::Kind::ADDITION_ASSIGNMENT: return "ADDITION_ASSIGNMENT";
        case BinaryOperatorExpression::Kind::SUBTRACTION_ASSIGNMENT: return "SUBTRACTION_ASSIGNMENT";
        case BinaryOperatorExpression::Kind::MULTIPLICATION_ASSIGNMENT: return "MULTIPLICATION_ASSIGNMENT";
        case BinaryOperatorExpression::Kind::DIVISION_ASSIGNMENT: return "DIVISION_ASSIGNMENT";
        case BinaryOperatorExpression::Kind::LESS_THAN: return "LESS_THAN";
        case BinaryOperatorExpression::Kind::LESS_THAN_EQUAL: return "LESS_THAN_EQUAL";
        case BinaryOperatorExpression::Kind::GREATER_THAN: return "GREATER_THAN";
        case BinaryOperatorExpression::Kind::GREATER_THAN_EQUAL: return "GREATER_THAN_EQUAL";
        case BinaryOperatorExpression::Kind::EQUALITY: return "EQUALITY";
        case BinaryOperatorExpression::Kind::INEQUALITY: return "INEQUALITY";
        case BinaryOperatorExpression::Kind::ASSIGNMENT: return "ASSIGNMENT";
        case BinaryOperatorExpression::Kind::COMMA: return "COMMA";
    }

    return "unknown";
}

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
                    std::vector<Token>::const_iterator iterator) const
    {
        return (iterator != tokens.end() && iterator->type == tokenType);
    }

    bool checkTokens(const std::vector<Token::Type>& tokenTypes,
                     const std::vector<Token>& tokens,
                     std::vector<Token>::const_iterator iterator) const
    {
        if (iterator == tokens.end()) return false;

        for (Token::Type tokenType : tokenTypes)
        {
            if (iterator->type == tokenType) return true;
        }

        return false;
    }

    Declaration* findDeclaration(const std::string& name, const std::vector<Declaration*>& declarationScope) const
    {
        for (Declaration* declaration : declarationScope)
        {
            if (declaration->name == name) return declaration;
        }

        return nullptr;
    }

    Declaration* findDeclaration(const std::string& name, const std::vector<std::vector<Declaration*>>& declarationScopes) const
    {
        for (auto i = declarationScopes.crbegin(); i != declarationScopes.crend(); ++i)
        {
            for (Declaration* declaration : *i)
            {
                if (declaration->name == name) return declaration;
            }
        }

        return nullptr;
    }

    TypeDeclaration* findTypeDeclaration(const std::string& name, const std::vector<std::vector<Declaration*>>& declarationScopes) const
    {
        for (auto i = declarationScopes.crbegin(); i != declarationScopes.crend(); ++i)
        {
            for (Declaration* declaration : *i)
            {
                if (declaration->declarationKind == Declaration::Kind::TYPE &&
                    declaration->name == name)
                {
                    return static_cast<TypeDeclaration*>(declaration);
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
                       std::vector<Token>::const_iterator iterator,
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

    ParameterDeclaration* parseParameterDeclaration(const std::vector<Token>& tokens,
                                                    std::vector<Token>::const_iterator& iterator,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes);

    /*TypeDefinitionDeclaration* parseTypeDefinitionDeclaration(const std::vector<Token>& tokens,
                                                              std::vector<Token>::const_iterator& iterator,
                                                              std::vector<std::vector<Declaration*>>& declarationScopes);*/

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
