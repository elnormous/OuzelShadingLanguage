//
//  OSL
//

#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
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

    Construct(Kind initKind): kind(initKind) {}

    inline Kind getKind() const { return kind; }

    Construct* parent = nullptr;

protected:
    Kind kind = Kind::NONE;
};

class TypeDeclaration;

class QualifiedType
{
public:
    bool operator<(const QualifiedType& other) const
    {
        if (typeDeclaration != other.typeDeclaration)
            return typeDeclaration < other.typeDeclaration;
        else if (isConst != other.isConst)
            return isConst < other.isConst;
        else if (isVolatile != other.isVolatile)
            return isVolatile < other.isVolatile;
        else return true;
    }

    TypeDeclaration* typeDeclaration = nullptr;
    bool isConst = false;
    bool isVolatile = false;
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
        DEFAULT,
        WHILE,
        DO,
        BREAK,
        CONTINUE,
        RETURN,
    };

    Statement(Kind initStatementKind): Construct(Construct::Kind::STATEMENT), statementKind(initStatementKind) {}

    inline Kind getStatementKind() const { return statementKind; }

protected:
    Kind statementKind = Kind::NONE;
};

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
        TEMPORARY_OBJECT,
        INITIALIZER_LIST,
        CAST
    };

    Expression(Kind initExpressionKind): Construct(Construct::Kind::EXPRESSION), expressionKind(initExpressionKind) {}

    inline Kind getExpressionKind() const { return expressionKind; }

    QualifiedType qualifiedType;
    bool isLValue = false;

protected:
    Kind expressionKind = Kind::NONE;
};

class Declaration: public Construct
{
public:
    enum class Kind
    {
        NONE,
        EMPTY,
        TYPE,
        FIELD,
        CALLABLE,
        VARIABLE,
        PARAMETER
    };

    Declaration(Kind initDeclarationKind): Construct(Construct::Kind::DECLARATION), declarationKind(initDeclarationKind) {}

    inline Kind getDeclarationKind() const { return declarationKind; }

    Declaration* getFirstDeclaration()
    {
        Declaration* result = this;

        while (result->previousDeclaration) result = result->previousDeclaration;

        return result;
    }

    std::string name;
    Declaration* previousDeclaration = nullptr;
    Declaration* definition = nullptr;

protected:
    Kind declarationKind = Kind::NONE;
};

class TypeDeclaration: public Declaration
{
public:
    enum class Kind
    {
        NONE,
        ARRAY,
        SCALAR,
        STRUCT
        //TYPE_DEFINITION // typedef is not supported in GLSL
    };

    TypeDeclaration(Kind initTypeKind): Declaration(Declaration::Kind::TYPE), typeKind(initTypeKind) {}

    inline Kind getTypeKind() const { return typeKind; }

    bool isBuiltin = false;

protected:
    Kind typeKind = Kind::NONE;
};

class ArrayTypeDeclaration: public TypeDeclaration
{
public:
    ArrayTypeDeclaration(): TypeDeclaration(TypeDeclaration::Kind::ARRAY)
    {
        definition = this;
    }

    QualifiedType elementType;
    uint32_t size = 0;
};

class ScalarTypeDeclaration: public TypeDeclaration
{
public:
    enum class Kind
    {
        NONE,
        BOOLEAN,
        INTEGER,
        FLOATING_POINT
    };

    ScalarTypeDeclaration(Kind initScalarTypeKind): TypeDeclaration(TypeDeclaration::Kind::SCALAR), scalarTypeKind(initScalarTypeKind)
    {
        definition = this;
    }

    inline Kind getScalarTypeKind() const { return scalarTypeKind; }

protected:
    Kind scalarTypeKind;
};

class FieldDeclaration: public Declaration
{
public:
    FieldDeclaration(): Declaration(Declaration::Kind::FIELD)
    {
        definition = this;
    }

    QualifiedType qualifiedType;

    Semantic semantic = Semantic::NONE;
};

class ParameterDeclaration: public Declaration
{
public:
    ParameterDeclaration(): Declaration(Declaration::Kind::PARAMETER)
    {
        definition = this;
    }

    QualifiedType qualifiedType;
};

/*class TypeDefinitionDeclaration: public TypeDeclaration
{
public:
    QualifiedType qualifiedType;
};*/

class CallableDeclaration: public Declaration
{
public:
    enum Kind
    {
        NONE,
        FUNCTION,
        CONSTRUCTOR,
        METHOD
    };

    CallableDeclaration(Kind initCallableDeclarationKind): Declaration(Declaration::Kind::CALLABLE), callableDeclarationKind(initCallableDeclarationKind) {}

    inline Kind getCallableDeclarationKind() const { return callableDeclarationKind; }

    QualifiedType qualifiedType;
    std::vector<ParameterDeclaration*> parameterDeclarations;
    Statement* body = nullptr;

protected:
    Kind callableDeclarationKind;
};

class FunctionDeclaration: public CallableDeclaration
{
public:
    enum class Program
    {
        NONE,
        FRAGMENT,
        VERTEX
    };

    FunctionDeclaration(): CallableDeclaration(CallableDeclaration::Kind::FUNCTION) {}

    bool isInline = false;
    bool isStatic = false;
    bool isBuiltin = false;
    Program program = Program::NONE;
};

class ConstructorDeclaration: public CallableDeclaration
{
public:
    ConstructorDeclaration(): CallableDeclaration(CallableDeclaration::Kind::CONSTRUCTOR) {}
};

class StructDeclaration: public TypeDeclaration
{
public:
    StructDeclaration(): TypeDeclaration(TypeDeclaration::Kind::STRUCT) {}

    ConstructorDeclaration* findConstructorDeclaration(const std::vector<QualifiedType>& parameters) const
    {
        for (Declaration* declaration : memberDeclarations)
        {
            if (declaration->getDeclarationKind() == Declaration::Kind::CALLABLE)
            {
                CallableDeclaration* callableDeclaration = static_cast<CallableDeclaration*>(declaration);

                if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::CONSTRUCTOR)
                {
                    ConstructorDeclaration* constructorDeclaration = static_cast<ConstructorDeclaration*>(callableDeclaration);

                    if (constructorDeclaration->parameterDeclarations.size() == parameters.size())
                    {
                        if (std::equal(parameters.begin(), parameters.end(),
                                       constructorDeclaration->parameterDeclarations.begin(),
                                       [](const QualifiedType& qualifiedType,
                                          const ParameterDeclaration* parameterDeclaration) {
                                           return qualifiedType.typeDeclaration->getFirstDeclaration() == parameterDeclaration->qualifiedType.typeDeclaration->getFirstDeclaration(); // TODO: type promotion
                                       }))
                        {
                            return constructorDeclaration;
                        }
                    }
                }
            }
        }

        return nullptr;
    }

    Declaration* findMemberDeclaration(const std::string& name) const
    {
        for (Declaration* memberDeclaration : memberDeclarations)
        {
            if (memberDeclaration->name == name) return memberDeclaration;
        }

        return nullptr;
    }

    std::vector<Declaration*> memberDeclarations;
};

class VariableDeclaration: public Declaration
{
public:
    VariableDeclaration(): Declaration(Declaration::Kind::VARIABLE)
    {
        definition = this;
    }

    QualifiedType qualifiedType;
    Expression* initialization = nullptr;

    bool isStatic = false;
};

class ExpressionStatement: public Statement
{
public:
    ExpressionStatement(): Statement(Statement::Kind::EXPRESSION) {}

    Expression* expression = nullptr;
};

class DeclarationStatement: public Statement
{
public:
    DeclarationStatement(): Statement(Statement::Kind::DECLARATION) {}

    Declaration* declaration = nullptr;
};

class CompoundStatement: public Statement
{
public:
    CompoundStatement(): Statement(Statement::Kind::COMPOUND) {}

    std::vector<Statement*> statements;
};

class IfStatement: public Statement
{
public:
    IfStatement(): Statement(Statement::Kind::IF) {}

    Construct* condition = nullptr;
    Statement* body = nullptr;
    Statement* elseBody = nullptr;
};

class ForStatement: public Statement
{
public:
    ForStatement(): Statement(Statement::Kind::FOR) {}

    Construct* initialization = nullptr;
    Construct* condition = nullptr;
    Expression* increment = nullptr;
    Statement* body = nullptr;
};

class SwitchStatement: public Statement
{
public:
    SwitchStatement(): Statement(Statement::Kind::SWITCH) {}

    Construct* condition = nullptr;
    Statement* body = nullptr;
};

class CaseStatement: public Statement
{
public:
    CaseStatement(): Statement(Statement::Kind::CASE) {}

    Expression* condition = nullptr;
    Statement* body = nullptr;
};

class DefaultStatement: public Statement
{
public:
    DefaultStatement(): Statement(Statement::Kind::DEFAULT) {}

    Statement* body = nullptr;
};

class WhileStatement: public Statement
{
public:
    WhileStatement(): Statement(Statement::Kind::WHILE) {}

    Construct* condition = nullptr;
    Statement* body = nullptr;
};

class DoStatement: public Statement
{
public:
    DoStatement(): Statement(Statement::Kind::DO) {}

    Expression* condition = nullptr;
    Statement* body = nullptr;
};

class BreakStatement: public Statement
{
public:
    BreakStatement(): Statement(Statement::Kind::BREAK) {}
};

class ContinueStatement: public Statement
{
public:
    ContinueStatement(): Statement(Statement::Kind::CONTINUE) {}
};

class ReturnStatement: public Statement
{
public:
    ReturnStatement(): Statement(Statement::Kind::RETURN) {}

    Expression* result = nullptr;
};

class LiteralExpression: public Expression
{
public:
    enum class Kind
    {
        NONE,
        BOOLEAN,
        INTEGER,
        FLOATING_POINT,
        STRING
    };

    LiteralExpression(Kind initLiteralKind): Expression(Expression::Kind::LITERAL), literalKind(initLiteralKind) {}

    inline Kind getLiteralKind() const { return literalKind; }

    TypeDeclaration* typeDeclaration = nullptr;

protected:
    Kind literalKind = Kind::NONE;
};

class BooleanLiteralExpression: public LiteralExpression
{
public:
    BooleanLiteralExpression(): LiteralExpression(LiteralExpression::Kind::BOOLEAN) {}
    bool value;
};

class IntegerLiteralExpression: public LiteralExpression
{
public:
    IntegerLiteralExpression(): LiteralExpression(LiteralExpression::Kind::INTEGER) {}

    int64_t value;
};

class FloatingPointLiteralExpression: public LiteralExpression
{
public:
    FloatingPointLiteralExpression(): LiteralExpression(LiteralExpression::Kind::FLOATING_POINT) {}

    double value;
};

class StringLiteralExpression: public LiteralExpression
{
public:
    StringLiteralExpression(): LiteralExpression(LiteralExpression::Kind::STRING) {}

    std::string value;
};

class DeclarationReferenceExpression: public Expression
{
public:
    DeclarationReferenceExpression(): Expression(Expression::Kind::DECLARATION_REFERENCE) {}

    Declaration* declaration = nullptr;
};

class CallExpression: public Expression
{
public:
    CallExpression(): Expression(Expression::Kind::CALL) {}

    DeclarationReferenceExpression* declarationReference = nullptr;
    std::vector<Expression*> parameters;
};

class ParenExpression: public Expression
{
public:
    ParenExpression(): Expression(Expression::Kind::PAREN) {}

    Expression* expression = nullptr;
};

class MemberExpression: public Expression
{
public:
    MemberExpression(): Expression(Expression::Kind::MEMBER) {}

    Expression* expression = nullptr;
    FieldDeclaration* fieldDeclaration = nullptr;
};

class ArraySubscriptExpression: public Expression
{
public:
    ArraySubscriptExpression(): Expression(Expression::Kind::ARRAY_SUBSCRIPT) {}

    Expression* expression = nullptr;
    Expression* subscript = nullptr;
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

    UnaryOperatorExpression(): Expression(Expression::Kind::UNARY) {}

    Expression* expression = nullptr;

    Kind operatorKind = Kind::NONE;
};

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
        OR, // ||
        AND, // &&
        COMMA // ,
    };

    BinaryOperatorExpression(): Expression(Expression::Kind::BINARY) {}

    Expression* leftExpression = nullptr;
    Expression* rightExpression = nullptr;

    Kind operatorKind = Kind::NONE;
};

class TernaryOperatorExpression: public Expression
{
public:
    TernaryOperatorExpression(): Expression(Expression::Kind::TERNARY) {}

    Expression* condition;
    Expression* leftExpression = nullptr;
    Expression* rightExpression = nullptr;
};

class TemporaryObjectExpression: public Expression
{
public:
    TemporaryObjectExpression(): Expression(Expression::Kind::TEMPORARY_OBJECT) {}

    ConstructorDeclaration* constructorDeclaration = nullptr;
    std::vector<Expression*> parameters;
};

class InitializerListExpression: public Expression
{
public:
    InitializerListExpression(): Expression(Expression::Kind::INITIALIZER_LIST) {}

    std::vector<Expression*> expressions;
};

class CastExpression: public Expression
{
public:
    enum class Kind
    {
        NONE,
        IMPLICIT,
        EXPLICIT
    };

    CastExpression(Kind initCastKind): Expression(Expression::Kind::CAST), castKind(initCastKind) {}

    inline Kind getCastKind() const { return castKind; }

    Expression* expression;

protected:
    Kind castKind;
};

class ASTContext
{
public:
    ASTContext();
    bool parse(const std::vector<Token>& tokens);

    void dump() const;

    std::vector<Declaration*> declarations;

private:
    static bool isToken(Token::Type tokenType,
                        const std::vector<Token>& tokens,
                        std::vector<Token>::const_iterator iterator)
    {
        return (iterator != tokens.end() && iterator->type == tokenType);
    }

    static bool isToken(const std::vector<Token::Type>& tokenTypes,
                        const std::vector<Token>& tokens,
                        std::vector<Token>::const_iterator iterator)
    {
        if (iterator == tokens.end()) return false;

        for (Token::Type tokenType : tokenTypes)
        {
            if (iterator->type == tokenType) return true;
        }

        return false;
    }

    static Declaration* findDeclaration(const std::string& name, const std::vector<Declaration*>& declarationScope)
    {
        for (auto declarationIterator = declarationScope.crbegin(); declarationIterator != declarationScope.crend(); ++declarationIterator)
        {
            if ((*declarationIterator)->name == name) return *declarationIterator;
        }

        return nullptr;
    }

    static Declaration* findDeclaration(const std::string& name, const std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        for (auto scopeIterator = declarationScopes.crbegin(); scopeIterator != declarationScopes.crend(); ++scopeIterator)
        {
            for (auto declarationIterator = scopeIterator->crbegin(); declarationIterator != scopeIterator->crend(); ++declarationIterator)
            {
                if ((*declarationIterator)->name == name) return *declarationIterator;
            }
        }

        return nullptr;
    }

    static TypeDeclaration* findTypeDeclaration(const std::string& name, const std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        Declaration* declaration = findDeclaration(name, declarationScopes);

        if (declaration && declaration->getDeclarationKind() == Declaration::Kind::TYPE)
            return static_cast<TypeDeclaration*>(declaration);

        return nullptr;
    }

    static StructDeclaration* findStructDeclaration(const std::string& name, const std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        TypeDeclaration* typeDeclaration = findTypeDeclaration(name, declarationScopes);

        if (typeDeclaration && typeDeclaration->getTypeKind() == TypeDeclaration::Kind::STRUCT)
            return static_cast<StructDeclaration*>(typeDeclaration);

        return nullptr;
    }

    static FunctionDeclaration* findFunctionDeclaration(const std::string& name,
                                                        const std::vector<std::vector<Declaration*>>& declarationScopes,
                                                        const std::vector<QualifiedType>& parameters)
    {
        for (auto scopeIterator = declarationScopes.crbegin(); scopeIterator != declarationScopes.crend(); ++scopeIterator)
        {
            for (auto declarationIterator = scopeIterator->crbegin(); declarationIterator != scopeIterator->crend(); ++declarationIterator)
            {
                if ((*declarationIterator)->name == name)
                {
                    if ((*declarationIterator)->getDeclarationKind() != Declaration::Kind::CALLABLE) return nullptr;

                    CallableDeclaration* callableDeclaration = static_cast<CallableDeclaration*>(*declarationIterator);

                    if (callableDeclaration->getCallableDeclarationKind() != CallableDeclaration::Kind::FUNCTION) return nullptr;

                    FunctionDeclaration* functionDeclaration = static_cast<FunctionDeclaration*>(callableDeclaration);

                    if (functionDeclaration->parameterDeclarations.size() == parameters.size())
                    {
                        if (std::equal(parameters.begin(), parameters.end(),
                                       functionDeclaration->parameterDeclarations.begin(),
                                       [](const QualifiedType& qualifiedType,
                                          const ParameterDeclaration* parameterDeclaration) {
                                           return qualifiedType.typeDeclaration->getFirstDeclaration() == parameterDeclaration->qualifiedType.typeDeclaration->getFirstDeclaration();
                                       }))
                        {
                            return functionDeclaration;
                        }
                    }
                }
            }
        }

        return nullptr;
    }

    static FunctionDeclaration* compareFunctionDeclarations(FunctionDeclaration* functionDeclaration1,
                                                            FunctionDeclaration* functionDeclaration2,
                                                            const std::vector<QualifiedType>& parameters);

    static FunctionDeclaration* resolveFunctionDeclaration(const std::string& name,
                                                           const std::vector<std::vector<Declaration*>>& declarationScopes,
                                                           const std::vector<QualifiedType>& parameters);

    ArrayTypeDeclaration* getArrayTypeDeclaration(QualifiedType qualifiedType, uint32_t size);
    
    static bool isType(const std::vector<Token>& tokens,
                       std::vector<Token>::const_iterator iterator,
                       std::vector<std::vector<Declaration*>>& declarationScopes);

    TypeDeclaration* parseType(const std::vector<Token>& tokens,
                               std::vector<Token>::const_iterator& iterator,
                               std::vector<std::vector<Declaration*>>& declarationScopes);

    static bool isDeclaration(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator iterator,
                              std::vector<std::vector<Declaration*>>& declarationScopes);

    Declaration* parseTopLevelDeclaration(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes);

    struct Specifiers
    {
        bool isConst = false;
        bool isInline = false;
        bool isStatic = false;
        bool isVolatile = false;
    };

    static bool parseSpecifiers(const std::vector<Token>& tokens,
                                std::vector<Token>::const_iterator& iterator,
                                Specifiers& specifiers);

    static bool parseAttributes(const std::vector<Token>& tokens,
                                std::vector<Token>::const_iterator& iterator,
                                std::vector<std::pair<std::string, std::vector<std::string>>>& attributes);

    Declaration* parseDeclaration(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                  Construct* parent);

    StructDeclaration* parseStructDeclaration(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent);

    Declaration* parseMemberDeclaration(const std::vector<Token>& tokens,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                        Construct* parent);

    ParameterDeclaration* parseParameterDeclaration(const std::vector<Token>& tokens,
                                                    std::vector<Token>::const_iterator& iterator,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                                    Construct* parent);

    /*TypeDefinitionDeclaration* parseTypeDefinitionDeclaration(const std::vector<Token>& tokens,
                                                                std::vector<Token>::const_iterator& iterator,
                                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                                Construct* parent);*/

    Statement* parseStatement(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<Declaration*>>& declarationScopes,
                              Construct* parent);

    CompoundStatement* parseCompoundStatement(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent);

    IfStatement* parseIfStatement(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                  Construct* parent);

    ForStatement* parseForStatement(const std::vector<Token>& tokens,
                                    std::vector<Token>::const_iterator& iterator,
                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                    Construct* parent);

    SwitchStatement* parseSwitchStatement(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent);

    CaseStatement* parseCaseStatement(const std::vector<Token>& tokens,
                                      std::vector<Token>::const_iterator& iterator,
                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                      Construct* parent);

    DefaultStatement* parseDefaultStatement(const std::vector<Token>& tokens,
                                            std::vector<Token>::const_iterator& iterator,
                                            std::vector<std::vector<Declaration*>>& declarationScopes,
                                            Construct* parent);

    WhileStatement* parseWhileStatement(const std::vector<Token>& tokens,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                        Construct* parent);

    DoStatement* parseDoStatement(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                  Construct* parent);

    CastExpression* addImplicitCast(Expression* expression,
                                    TypeDeclaration* typeDeclaration);

    Expression* parsePrimaryExpression(const std::vector<Token>& tokens,
                                       std::vector<Token>::const_iterator& iterator,
                                       std::vector<std::vector<Declaration*>>& declarationScopes,
                                       Construct* parent);

    Expression* parseSubscriptExpression(const std::vector<Token>& tokens,
                                         std::vector<Token>::const_iterator& iterator,
                                         std::vector<std::vector<Declaration*>>& declarationScopes,
                                         Construct* parent);

    Expression* parseMemberExpression(const std::vector<Token>& tokens,
                                      std::vector<Token>::const_iterator& iterator,
                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                      Construct* parent);

    Expression* parseSignExpression(const std::vector<Token>& tokens,
                                    std::vector<Token>::const_iterator& iterator,
                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                    Construct* parent);

    Expression* parseNotExpression(const std::vector<Token>& tokens,
                                   std::vector<Token>::const_iterator& iterator,
                                   std::vector<std::vector<Declaration*>>& declarationScopes,
                                   Construct* parent);

    Expression* parseMultiplicationExpression(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent);

    Expression* parseAdditionExpression(const std::vector<Token>& tokens,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                        Construct* parent);

    Expression* parseLessThanExpression(const std::vector<Token>& tokens,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                        Construct* parent);

    Expression* parseGreaterThanExpression(const std::vector<Token>& tokens,
                                           std::vector<Token>::const_iterator& iterator,
                                           std::vector<std::vector<Declaration*>>& declarationScopes,
                                           Construct* parent);

    Expression* parseEqualityExpression(const std::vector<Token>& tokens,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                        Construct* parent);

    Expression* parseLogicalAndExpression(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent);

    Expression* parseLogicalOrExpression(const std::vector<Token>& tokens,
                                         std::vector<Token>::const_iterator& iterator,
                                         std::vector<std::vector<Declaration*>>& declarationScopes,
                                         Construct* parent);

    Expression* parseTernaryExpression(const std::vector<Token>& tokens,
                                       std::vector<Token>::const_iterator& iterator,
                                       std::vector<std::vector<Declaration*>>& declarationScopes,
                                       Construct* parent);

    Expression* parseAssignmentExpression(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent);

    Expression* parseAdditionAssignmentExpression(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  Construct* parent);

    Expression* parseMultiplicationAssignmentExpression(const std::vector<Token>& tokens,
                                                        std::vector<Token>::const_iterator& iterator,
                                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                                        Construct* parent);

    Expression* parseCommaExpression(const std::vector<Token>& tokens,
                                     std::vector<Token>::const_iterator& iterator,
                                     std::vector<std::vector<Declaration*>>& declarationScopes,
                                     Construct* parent);

    void dumpDeclaration(const Declaration* declaration, std::string indent = std::string()) const;
    void dumpStatement(const Statement* statement, std::string indent = std::string()) const;
    void dumpExpression(const Expression* expression, std::string indent = std::string()) const;
    void dumpConstruct(const Construct* construct, std::string indent = std::string()) const;

    std::vector<std::unique_ptr<Construct>> constructs;

    std::map<std::pair<QualifiedType, uint32_t>, ArrayTypeDeclaration*> arrayTypeDeclarations;

    ScalarTypeDeclaration boolTypeDeclaration;
    ScalarTypeDeclaration intTypeDeclaration;
    ScalarTypeDeclaration floatTypeDeclaration;
    StructDeclaration float2TypeDeclaration;
    StructDeclaration float3TypeDeclaration;
    StructDeclaration float4TypeDeclaration;
    FieldDeclaration fieldDeclarations[2 * (30 + 120 + 340)];
    ConstructorDeclaration constructorDeclarations[6];
    ParameterDeclaration parameterDeclarations[3 + 4 + 5];
    StructDeclaration float2x2TypeDeclaration;
    StructDeclaration float3x3TypeDeclaration;
    StructDeclaration float4x4TypeDeclaration;
    StructDeclaration stringTypeDeclaration;
    StructDeclaration samplerStateTypeDeclaration;
    StructDeclaration texture2DTypeDeclaration;

    ParameterDeclaration samplerParameterDeclaration;
    ParameterDeclaration coordParameterDeclaration;
    FunctionDeclaration mulFunctionDeclaration;
};
