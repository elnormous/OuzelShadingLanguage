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
#include "Program.hpp"
#include "Semantic.hpp"
#include "Declarations.hpp"
#include "Expressions.hpp"
#include "Statements.hpp"
#include "QualifiedType.hpp"

class ASTContext
{
public:
    ASTContext();
    ASTContext(const std::vector<Token>& tokens);

    void dump() const;

    inline const std::vector<Declaration*>& getDeclarations() const { return declarations; }

private:
    static bool isToken(Token::Type tokenType,
                        const std::vector<Token>& tokens,
                        std::vector<Token>::const_iterator iterator)
    {
        return (iterator != tokens.end() && iterator->type == tokenType);
    }

    static void expectToken(Token::Type tokenType,
                            const std::vector<Token>& tokens,
                            std::vector<Token>::const_iterator iterator)
    {
        if (iterator == tokens.end())
            throw std::runtime_error("Unexpected end of file");
        if (iterator->type != tokenType)
            throw std::runtime_error("Expected " + toString(tokenType));
    }

    static bool isToken(const std::vector<Token::Type>& tokenTypes,
                        const std::vector<Token>& tokens,
                        std::vector<Token>::const_iterator iterator)
    {
        if (iterator == tokens.end()) return false;

        for (Token::Type tokenType : tokenTypes)
            if (iterator->type == tokenType) return true;

        return false;
    }

    static void expectToken(const std::vector<Token::Type>& tokenTypes,
                            const std::vector<Token>& tokens,
                            std::vector<Token>::const_iterator iterator)
    {
        if (iterator == tokens.end())
            throw std::runtime_error("Unexpected end of file");

        for (Token::Type tokenType : tokenTypes)
            if (iterator->type == tokenType) return;

        std::string str;
        for (Token::Type tokenType : tokenTypes)
        {
            if (!str.empty()) str += "or ";
            str += toString(tokenType);
        }

        throw std::runtime_error("Expected " + str);
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

    static CallableDeclaration* compareCallableDeclarations(CallableDeclaration* callableDeclaration1,
                                                            CallableDeclaration* callableDeclaration2,
                                                            const std::vector<QualifiedType>& parameters);

    static FunctionDeclaration* resolveFunctionDeclaration(const std::string& name,
                                                           const std::vector<std::vector<Declaration*>>& declarationScopes,
                                                           const std::vector<QualifiedType>& parameters);

    static OperatorDeclaration* resolveOperatorDeclaration(Operator op,
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

    static Specifiers parseSpecifiers(const std::vector<Token>& tokens,
                                std::vector<Token>::const_iterator& iterator);

    static std::vector<std::pair<std::string, std::vector<std::string>>> parseAttributes(const std::vector<Token>& tokens,
                                                                                         std::vector<Token>::const_iterator& iterator);

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

    Expression* parseSizeofExpression(const std::vector<Token>& tokens,
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
    
    Expression* parseExpression(const std::vector<Token>& tokens,
                                std::vector<Token>::const_iterator& iterator,
                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                Construct* parent)
    {
        return parseCommaExpression(tokens, iterator, declarationScopes, parent);
    }

    void dumpDeclaration(const Declaration* declaration, std::string indent = std::string()) const;
    void dumpStatement(const Statement* statement, std::string indent = std::string()) const;
    void dumpExpression(const Expression* expression, std::string indent = std::string()) const;
    void dumpConstruct(const Construct* construct, std::string indent = std::string()) const;

    ScalarTypeDeclaration* addScalarTypeDeclaration(const std::string& name,
                                                    ScalarTypeDeclaration::Kind kind,
                                                    uint32_t size,
                                                    bool isUnsigned,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        ScalarTypeDeclaration* scalarTypeDeclaration = new ScalarTypeDeclaration(kind);
        constructs.push_back(std::unique_ptr<Construct>(scalarTypeDeclaration));

        scalarTypeDeclaration->name = name;
        scalarTypeDeclaration->size = size;
        scalarTypeDeclaration->isUnsigned = isUnsigned;
        scalarTypeDeclaration->isBuiltin = true;
        scalarTypeDeclaration->definition = scalarTypeDeclaration;
        declarationScopes.back().push_back(scalarTypeDeclaration);

        addOperatorDeclaration(Operator::ASSIGNMENT, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::EQUALITY, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::INEQUALITY, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        // TODO: for int and float only
        addOperatorDeclaration(Operator::ADDITION, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::ADDITION_ASSIGNMENT, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::SUBTRACTION, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::SUBTRACTION_ASSIGNMENT, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::MULTIPLICATION, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::MULTIPLICATION_ASSIGNMENT, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::DIVISION, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::DIVISION_ASSIGNMENT, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::POSITIVE, scalarTypeDeclaration, {scalarTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::NEGATIVE, scalarTypeDeclaration, {scalarTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::LESS_THAN, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::LESS_THAN_EQUAL, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::GREATER_THAN, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::GREATER_THAN_EQUAL, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        // TODO: for bool only
        addOperatorDeclaration(Operator::NEGATION, boolTypeDeclaration, {boolTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::OR, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::AND, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::COMMA, scalarTypeDeclaration, {nullptr, scalarTypeDeclaration}, declarationScopes);

        return scalarTypeDeclaration;
    }

    StructDeclaration* addStructDeclaration(const std::string& name,
                                            uint32_t size,
                                            std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        StructDeclaration* structDeclaration = new StructDeclaration();
        constructs.push_back(std::unique_ptr<Construct>(structDeclaration));

        structDeclaration->name = name;
        structDeclaration->size = size;
        structDeclaration->isBuiltin = true;
        structDeclaration->definition = structDeclaration;
        declarationScopes.back().push_back(structDeclaration);

        addOperatorDeclaration(Operator::ASSIGNMENT, structDeclaration, {structDeclaration, structDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::EQUALITY, structDeclaration, {structDeclaration, structDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::INEQUALITY, structDeclaration, {structDeclaration, structDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::COMMA, structDeclaration, {nullptr, structDeclaration}, declarationScopes);

        return structDeclaration;
    }

    FieldDeclaration* addFieldDeclaration(StructDeclaration* structDeclaration,
                                          const std::string& name,
                                          TypeDeclaration* type,
                                          bool isConst,
                                          std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        FieldDeclaration* fieldDeclaration = new FieldDeclaration();
        constructs.push_back(std::unique_ptr<Construct>(fieldDeclaration));

        fieldDeclaration->parent = structDeclaration;
        fieldDeclaration->name = name;
        fieldDeclaration->qualifiedType.typeDeclaration = type;
        fieldDeclaration->qualifiedType.isConst = isConst;
        declarationScopes.back().push_back(fieldDeclaration);

        structDeclaration->memberDeclarations.push_back(fieldDeclaration);

        return fieldDeclaration;
    }

    FunctionDeclaration* addFunctionDeclaration(const std::string& name,
                                                TypeDeclaration* resultType,
                                                const std::vector<TypeDeclaration*>& parameters,
                                                std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        FunctionDeclaration* functionDeclaration = new FunctionDeclaration();
        constructs.push_back(std::unique_ptr<Construct>(functionDeclaration));

        functionDeclaration->name = name;
        functionDeclaration->qualifiedType.typeDeclaration = resultType;

        for (TypeDeclaration* parameter : parameters)
        {
            ParameterDeclaration* parameterDeclaration = new ParameterDeclaration();
            constructs.push_back(std::unique_ptr<Construct>(parameterDeclaration));

            parameterDeclaration->qualifiedType.typeDeclaration = parameter;
            functionDeclaration->parameterDeclarations.push_back(parameterDeclaration);
        }

        functionDeclaration->isBuiltin = true;
        declarationScopes.back().push_back(functionDeclaration);

        return functionDeclaration;
    }

    OperatorDeclaration* addOperatorDeclaration(Operator op,
                                                TypeDeclaration* resultType,
                                                const std::vector<TypeDeclaration*>& parameters,
                                                std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        OperatorDeclaration* operatorDeclaration = new OperatorDeclaration();
        constructs.push_back(std::unique_ptr<Construct>(operatorDeclaration));

        operatorDeclaration->op = op;
        operatorDeclaration->qualifiedType.typeDeclaration = resultType;

        for (TypeDeclaration* parameter : parameters)
        {
            ParameterDeclaration* parameterDeclaration = new ParameterDeclaration();
            constructs.push_back(std::unique_ptr<Construct>(parameterDeclaration));

            parameterDeclaration->qualifiedType.typeDeclaration = parameter;
            operatorDeclaration->parameterDeclarations.push_back(parameterDeclaration);
        }

        declarationScopes.back().push_back(operatorDeclaration);

        return operatorDeclaration;
    }

    std::vector<Declaration*> declarations;
    std::vector<std::unique_ptr<Construct>> constructs;

    std::map<std::pair<QualifiedType, uint32_t>, ArrayTypeDeclaration*> arrayTypeDeclarations;

    ScalarTypeDeclaration* boolTypeDeclaration;
    ScalarTypeDeclaration* intTypeDeclaration;
    ScalarTypeDeclaration* unsignedIntTypeDeclaration;
    ScalarTypeDeclaration* floatTypeDeclaration;
    StructDeclaration* stringTypeDeclaration;
    ConstructorDeclaration constructorDeclarations[6];
    ParameterDeclaration parameterDeclarations[3 + 4 + 5];
};
