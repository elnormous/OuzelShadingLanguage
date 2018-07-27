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

    std::vector<Declaration*> declarations;
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

    ParameterDeclaration boolParameterDeclaration;
    ParameterDeclaration intParameterDeclaration;
    ParameterDeclaration floatParameterDeclaration;
    ParameterDeclaration samplerParameterDeclaration;
    ParameterDeclaration coordParameterDeclaration;
    ParameterDeclaration matParameterDeclaration;
    ParameterDeclaration vec2ParameterDeclaration;
    ParameterDeclaration vec4ParameterDeclaration;
    FunctionDeclaration texture2DFunctionDeclaration;
    FunctionDeclaration mulMatMatFunctionDeclaration;
    FunctionDeclaration mulMatVecFunctionDeclaration;
    FunctionDeclaration mulVecMatFunctionDeclaration;

    OperatorDeclaration boolNegation;

    OperatorDeclaration intPositive;
    OperatorDeclaration floatPositive;
    OperatorDeclaration intNegative;
    OperatorDeclaration floatNegative;

    OperatorDeclaration intAddition;
    OperatorDeclaration floatAddition;

    OperatorDeclaration intSubtraction;
    OperatorDeclaration floatSubtraction;

    OperatorDeclaration intMultiplication;
    OperatorDeclaration floatMultiplication;

    OperatorDeclaration intDivision;
    OperatorDeclaration floatDivision;
};
