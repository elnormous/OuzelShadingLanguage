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
        TYPE_SIMPLE,
        TYPE_STRUCT,
        FIELD,
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
        STATEMENT_EMPTY,
        STATEMENT_EXPRESSION,
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
        OPERATOR_UNARY,
        OPERATOR_BINARY,
        OPERATOR_TERNARY,
    };

    Kind kind = Kind::NONE;
};

inline std::string nodeKindToString(Construct::Kind type)
{
    switch (type)
    {
        case Construct::Kind::NONE: return "NONE";
        case Construct::Kind::TYPE_SIMPLE: return "TYPE_SIMPLE";
        case Construct::Kind::TYPE_STRUCT: return "TYPE_STRUCT";
        case Construct::Kind::FIELD: return "FIELD";
        case Construct::Kind::TRANSLATION_UNIT: return "TRANSLATION_UNIT";
        case Construct::Kind::DECLARATION_EMPTY: return "DECLARATION_EMPTY";
        case Construct::Kind::DECLARATION_STRUCT: return "DECLARATION_STRUCT";
        case Construct::Kind::DECLARATION_FIELD: return "DECLARATION_FIELD";
        //case Construct::Kind::DECLARATION_TYPE_DEFINITION: return "DECLARATION_TYPE_DEFINITION";
        case Construct::Kind::DECLARATION_FUNCTION: return "DECLARATION_FUNCTION";
        case Construct::Kind::DECLARATION_VARIABLE: return "DECLARATION_VARIABLE";
        case Construct::Kind::DECLARATION_PARAMETER: return "DECLARATION_PARAMETER";
        case Construct::Kind::STATEMENT_EMPTY: return "STATEMENT_EMPTY";
        case Construct::Kind::STATEMENT_EXPRESSION: return "STATEMENT_EXPRESSION";
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

class Type: public Construct
{
public:
    std::string name;
    TypeDeclaration* declaration = nullptr;
};

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
};

class Expression: public Construct
{
public:
    std::string value;
    QualifiedType qualifiedType;
};

class StructType;
class FieldDeclaration;

class Field: public Construct
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

    const TranslationUnit* getTranslationUnit() const { return translationUnit; }

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

    Declaration* findDeclaration(const std::string& name, std::vector<std::vector<Declaration*>>& declarations)
    {
        for (auto i = declarations.crbegin(); i != declarations.crend(); ++i)
        {
            for (Declaration* declaration : *i)
            {
                switch (declaration->kind)
                {
                    case Construct::Kind::DECLARATION_STRUCT:
                    {
                        StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(declaration);
                        if (structDeclaration->type->name == name) return declaration;
                        break;
                    }
                    case Construct::Kind::DECLARATION_FUNCTION:
                    {
                        FunctionDeclaration* functionDeclaration = static_cast<FunctionDeclaration*>(declaration);
                        if (functionDeclaration->name == name) return declaration;
                        break;
                    }
                    case Construct::Kind::DECLARATION_VARIABLE:
                    {
                        VariableDeclaration* variableDeclaration = static_cast<VariableDeclaration*>(declaration);
                        if (variableDeclaration->name == name) return declaration;
                        break;
                    }
                    case Construct::Kind::DECLARATION_PARAMETER:
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

    Type* findType(const std::string& name, std::vector<std::vector<Declaration*>>& declarations)
    {
        for (auto i = declarations.crbegin(); i != declarations.crend(); ++i)
        {
            for (Declaration* declaration : *i)
            {
                if (declaration->kind == Construct::Kind::DECLARATION_STRUCT)
                {
                    StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(declaration);
                    if (structDeclaration->type->name == name) return structDeclaration->type;
                }
            }
        }

        for (auto i = builtinTypes.cbegin(); i != builtinTypes.cend(); ++i)
        {
            if ((*i)->name == name) return (*i).get();
        }

        return nullptr;
    }

    Field* findField(const std::string& name, StructType* structType)
    {
        for (Field* field : structType->fields)
        {
            if (field->name == name) return field;
        }

        return nullptr;
    }

    TranslationUnit* parseTopLevel(const std::vector<Token>& tokens,
                                   std::vector<Token>::const_iterator& iterator,
                                   std::vector<std::vector<Declaration*>>& declarations);

    StructDeclaration* parseStructDeclaration(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarations);

    /*TypeDefinitionDeclaration* parseTypeDefinitionDeclaration(const std::vector<Token>& tokens,
                                                              std::vector<Token>::const_iterator& iterator,
                                                              std::vector<std::vector<Declaration*>>& declarations);*/

    FunctionDeclaration* parseFunctionDeclaration(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarations);

    VariableDeclaration* parseVariableDeclaration(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarations);

    Statement* parseStatement(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<Declaration*>>& declarations);

    CompoundStatement* parseCompoundStatement(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarations);

    IfStatement* parseIfStatement(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<Declaration*>>& declarations);

    ForStatement* parseForStatement(const std::vector<Token>& tokens,
                                    std::vector<Token>::const_iterator& iterator,
                                    std::vector<std::vector<Declaration*>>& declarations);

    SwitchStatement* parseSwitchStatement(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarations);

    CaseStatement* parseCaseStatement(const std::vector<Token>& tokens,
                                      std::vector<Token>::const_iterator& iterator,
                                      std::vector<std::vector<Declaration*>>& declarations);

    WhileStatement* parseWhileStatement(const std::vector<Token>& tokens,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<std::vector<Declaration*>>& declarations);

    DoStatement* parseDoStatement(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<Declaration*>>& declarations);

    ReturnStatement* parseReturnStatement(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarations);

    Expression* parseExpression(const std::vector<Token>& tokens,
                                std::vector<Token>::const_iterator& iterator,
                                std::vector<std::vector<Declaration*>>& declarations);

    Expression* parseMultiplicationAssignment(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarations);

    Expression* parseAdditionAssignment(const std::vector<Token>& tokens,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<std::vector<Declaration*>>& declarations);

    Expression* parseAssignment(const std::vector<Token>& tokens,
                                std::vector<Token>::const_iterator& iterator,
                                std::vector<std::vector<Declaration*>>& declarations);

    Expression* parseTernary(const std::vector<Token>& tokens,
                             std::vector<Token>::const_iterator& iterator,
                             std::vector<std::vector<Declaration*>>& declarations);

    Expression* parseEquality(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<Declaration*>>& declarations);

    Expression* parseGreaterThan(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::vector<Declaration*>>& declarations);

    Expression* parseLessThan(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<Declaration*>>& declarations);

    Expression* parseAddition(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<Declaration*>>& declarations);

    Expression* parseMultiplication(const std::vector<Token>& tokens,
                                    std::vector<Token>::const_iterator& iterator,
                                    std::vector<std::vector<Declaration*>>& declarations);

    Expression* parseNot(const std::vector<Token>& tokens,
                         std::vector<Token>::const_iterator& iterator,
                         std::vector<std::vector<Declaration*>>& declarations);

    Expression* parseSign(const std::vector<Token>& tokens,
                          std::vector<Token>::const_iterator& iterator,
                          std::vector<std::vector<Declaration*>>& declarations);

    Expression* parseMember(const std::vector<Token>& tokens,
                            std::vector<Token>::const_iterator& iterator,
                            std::vector<std::vector<Declaration*>>& declarations);

    Expression* parsePrimary(const std::vector<Token>& tokens,
                             std::vector<Token>::const_iterator& iterator,
                             std::vector<std::vector<Declaration*>>& declarations);

    void dumpNode(const Construct* node, std::string indent = std::string());
    
    TranslationUnit* translationUnit = nullptr;
    std::vector<std::unique_ptr<Construct>> constructs;
    std::vector<std::unique_ptr<Type>> builtinTypes;
};
