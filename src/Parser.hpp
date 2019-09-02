//
//  OSL
//

#ifndef PARSER_HPP
#define PARSER_HPP

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
                        std::vector<Token>::const_iterator iterator,
                        std::vector<Token>::const_iterator end)
    {
        return (iterator != end && iterator->type == tokenType);
    }

    static void expectToken(Token::Type tokenType,
                            std::vector<Token>::const_iterator iterator,
                            std::vector<Token>::const_iterator end)
    {
        if (iterator == end)
            throw std::runtime_error("Unexpected end of file");
        if (iterator->type != tokenType)
            throw std::runtime_error("Expected " + toString(tokenType));
    }

    static bool isToken(const std::vector<Token::Type>& tokenTypes,
                        std::vector<Token>::const_iterator iterator,
                        std::vector<Token>::const_iterator end)
    {
        if (iterator == end) return false;

        for (Token::Type tokenType : tokenTypes)
            if (iterator->type == tokenType) return true;

        return false;
    }

    static void expectToken(const std::vector<Token::Type>& tokenTypes,
                            std::vector<Token>::const_iterator iterator,
                            std::vector<Token>::const_iterator end)
    {
        if (iterator == end)
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
            if ((*declarationIterator)->name == name) return *declarationIterator;

        return nullptr;
    }

    static Declaration* findDeclaration(const std::string& name, const std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        for (auto scopeIterator = declarationScopes.crbegin(); scopeIterator != declarationScopes.crend(); ++scopeIterator)
        {
            for (auto declarationIterator = scopeIterator->crbegin(); declarationIterator != scopeIterator->crend(); ++declarationIterator)
                if ((*declarationIterator)->name == name) return *declarationIterator;
        }

        return nullptr;
    }

    static TypeDeclaration* findTypeDeclaration(const std::string& name, const std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        Declaration* declaration = findDeclaration(name, declarationScopes);

        if (declaration && declaration->getDeclarationKind() == Declaration::Kind::Type)
            return static_cast<TypeDeclaration*>(declaration);

        return nullptr;
    }

    static StructDeclaration* findStructDeclaration(const std::string& name, const std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        TypeDeclaration* typeDeclaration = findTypeDeclaration(name, declarationScopes);

        if (typeDeclaration && typeDeclaration->getTypeKind() == TypeDeclaration::Kind::Struct)
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
                    if ((*declarationIterator)->getDeclarationKind() != Declaration::Kind::Callable) return nullptr;

                    CallableDeclaration* callableDeclaration = static_cast<CallableDeclaration*>(*declarationIterator);

                    if (callableDeclaration->getCallableDeclarationKind() != CallableDeclaration::Kind::Function) return nullptr;

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

    static const CallableDeclaration* compareCallableDeclarations(const CallableDeclaration* callableDeclaration1,
                                                            const CallableDeclaration* callableDeclaration2,
                                                            const std::vector<QualifiedType>& arguments);

    static FunctionDeclaration* resolveFunctionDeclaration(const std::string& name,
                                                           const std::vector<std::vector<Declaration*>>& declarationScopes,
                                                           const std::vector<QualifiedType>& arguments);

    static OperatorDeclaration* resolveOperatorDeclaration(Operator op,
                                                           const std::vector<std::vector<Declaration*>>& declarationScopes,
                                                           const std::vector<QualifiedType>& arguments);

    ArrayTypeDeclaration* getArrayTypeDeclaration(QualifiedType qualifiedType, uint32_t size);
    
    static bool isType(std::vector<Token>::const_iterator iterator,
                       std::vector<Token>::const_iterator end,
                       std::vector<std::vector<Declaration*>>& declarationScopes);

    TypeDeclaration* parseType(std::vector<Token>::const_iterator& iterator,
                               std::vector<Token>::const_iterator end,
                               std::vector<std::vector<Declaration*>>& declarationScopes);

    static bool isDeclaration(std::vector<Token>::const_iterator iterator,
                              std::vector<Token>::const_iterator end,
                              std::vector<std::vector<Declaration*>>& declarationScopes);

    Declaration* parseTopLevelDeclaration(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes);

    struct Specifiers
    {
        bool isConst = false;
        bool isInline = false;
        bool isStatic = false;
        bool isExtern = false;
        bool isVolatile = false;
    };

    static Specifiers parseSpecifiers(std::vector<Token>::const_iterator& iterator,
                                      std::vector<Token>::const_iterator end);

    static std::vector<std::pair<std::string, std::vector<std::string>>> parseAttributes(std::vector<Token>::const_iterator& iterator,
                                                                                         std::vector<Token>::const_iterator end);

    Declaration* parseDeclaration(std::vector<Token>::const_iterator& iterator,
                                  std::vector<Token>::const_iterator end,
                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                  Construct* parent);

    StructDeclaration* parseStructDeclaration(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent);

    Declaration* parseMemberDeclaration(std::vector<Token>::const_iterator& iterator,
                                        std::vector<Token>::const_iterator end,
                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                        Construct* parent);

    ParameterDeclaration* parseParameterDeclaration(std::vector<Token>::const_iterator& iterator,
                                                    std::vector<Token>::const_iterator end,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                                    Construct* parent);

    /*TypeDefinitionDeclaration* parseTypeDefinitionDeclaration(const std::vector<Token>& tokens,
                                                                std::vector<Token>::const_iterator& iterator,
                                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                                Construct* parent);*/

    Statement* parseStatement(std::vector<Token>::const_iterator& iterator,
                              std::vector<Token>::const_iterator end,
                              std::vector<std::vector<Declaration*>>& declarationScopes,
                              Construct* parent);

    CompoundStatement* parseCompoundStatement(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent);

    IfStatement* parseIfStatement(std::vector<Token>::const_iterator& iterator,
                                  std::vector<Token>::const_iterator end,
                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                  Construct* parent);

    ForStatement* parseForStatement(std::vector<Token>::const_iterator& iterator,
                                    std::vector<Token>::const_iterator end,
                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                    Construct* parent);

    SwitchStatement* parseSwitchStatement(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent);

    CaseStatement* parseCaseStatement(std::vector<Token>::const_iterator& iterator,
                                      std::vector<Token>::const_iterator end,
                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                      Construct* parent);

    DefaultStatement* parseDefaultStatement(std::vector<Token>::const_iterator& iterator,
                                            std::vector<Token>::const_iterator end,
                                            std::vector<std::vector<Declaration*>>& declarationScopes,
                                            Construct* parent);

    WhileStatement* parseWhileStatement(std::vector<Token>::const_iterator& iterator,
                                        std::vector<Token>::const_iterator end,
                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                        Construct* parent);

    DoStatement* parseDoStatement(std::vector<Token>::const_iterator& iterator,
                                  std::vector<Token>::const_iterator end,
                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                  Construct* parent);

    CastExpression* addImplicitCast(Expression* expression,
                                    TypeDeclaration* typeDeclaration,
                                    Expression::Category category);

    Expression* parsePrimaryExpression(std::vector<Token>::const_iterator& iterator,
                                       std::vector<Token>::const_iterator end,
                                       std::vector<std::vector<Declaration*>>& declarationScopes,
                                       Construct* parent);

    Expression* parseSubscriptExpression(std::vector<Token>::const_iterator& iterator,
                                         std::vector<Token>::const_iterator end,
                                         std::vector<std::vector<Declaration*>>& declarationScopes,
                                         Construct* parent);

    Expression* parseMemberExpression(std::vector<Token>::const_iterator& iterator,
                                      std::vector<Token>::const_iterator end,
                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                      Construct* parent);

    Expression* parseSignExpression(std::vector<Token>::const_iterator& iterator,
                                    std::vector<Token>::const_iterator end,
                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                    Construct* parent);

    Expression* parseNotExpression(std::vector<Token>::const_iterator& iterator,
                                   std::vector<Token>::const_iterator end,
                                   std::vector<std::vector<Declaration*>>& declarationScopes,
                                   Construct* parent);

    Expression* parseSizeofExpression(std::vector<Token>::const_iterator& iterator,
                                      std::vector<Token>::const_iterator end,
                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                      Construct* parent);
    
    Expression* parseMultiplicationExpression(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent);

    Expression* parseAdditionExpression(std::vector<Token>::const_iterator& iterator,
                                        std::vector<Token>::const_iterator end,
                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                        Construct* parent);

    Expression* parseLessThanExpression(std::vector<Token>::const_iterator& iterator,
                                        std::vector<Token>::const_iterator end,
                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                        Construct* parent);

    Expression* parseGreaterThanExpression(std::vector<Token>::const_iterator& iterator,
                                           std::vector<Token>::const_iterator end,
                                           std::vector<std::vector<Declaration*>>& declarationScopes,
                                           Construct* parent);

    Expression* parseEqualityExpression(std::vector<Token>::const_iterator& iterator,
                                        std::vector<Token>::const_iterator end,
                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                        Construct* parent);

    Expression* parseLogicalAndExpression(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent);

    Expression* parseLogicalOrExpression(std::vector<Token>::const_iterator& iterator,
                                         std::vector<Token>::const_iterator end,
                                         std::vector<std::vector<Declaration*>>& declarationScopes,
                                         Construct* parent);

    Expression* parseTernaryExpression(std::vector<Token>::const_iterator& iterator,
                                       std::vector<Token>::const_iterator end,
                                       std::vector<std::vector<Declaration*>>& declarationScopes,
                                       Construct* parent);

    Expression* parseAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent);

    Expression* parseAdditionAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  Construct* parent);

    Expression* parseMultiplicationAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                                        std::vector<Token>::const_iterator end,
                                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                                        Construct* parent);

    Expression* parseCommaExpression(std::vector<Token>::const_iterator& iterator,
                                     std::vector<Token>::const_iterator end,
                                     std::vector<std::vector<Declaration*>>& declarationScopes,
                                     Construct* parent);
    
    Expression* parseExpression(std::vector<Token>::const_iterator& iterator,
                                std::vector<Token>::const_iterator end,
                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                Construct* parent)
    {
        return parseCommaExpression(iterator, end, declarationScopes, parent);
    }

    static void dumpDeclaration(const Declaration* declaration, std::string indent = std::string());
    static void dumpStatement(const Statement* statement, std::string indent = std::string());
    static void dumpExpression(const Expression* expression, std::string indent = std::string());
    static void dumpConstruct(const Construct* construct, std::string indent = std::string());

    ScalarTypeDeclaration* addScalarTypeDeclaration(const std::string& name,
                                                    ScalarTypeDeclaration::Kind kind,
                                                    uint32_t size,
                                                    bool isUnsigned,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        ScalarTypeDeclaration* scalarTypeDeclaration;
        constructs.push_back(std::unique_ptr<Construct>(scalarTypeDeclaration = new ScalarTypeDeclaration(kind)));

        scalarTypeDeclaration->name = name;
        scalarTypeDeclaration->size = size;
        scalarTypeDeclaration->isUnsigned = isUnsigned;
        scalarTypeDeclaration->isBuiltin = true;
        scalarTypeDeclaration->definition = scalarTypeDeclaration;
        declarationScopes.back().push_back(scalarTypeDeclaration);

        addOperatorDeclaration(Operator::Comma, scalarTypeDeclaration, {nullptr, scalarTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::Assignment, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::Equality, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::Inequality, scalarTypeDeclaration, {scalarTypeDeclaration, scalarTypeDeclaration}, declarationScopes);

        return scalarTypeDeclaration;
    }

    StructDeclaration* addStructDeclaration(const std::string& name,
                                            uint32_t size,
                                            std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        StructDeclaration* structDeclaration;
        constructs.push_back(std::unique_ptr<Construct>(structDeclaration = new StructDeclaration()));

        structDeclaration->name = name;
        structDeclaration->size = size;
        structDeclaration->isBuiltin = true;
        structDeclaration->definition = structDeclaration;
        declarationScopes.back().push_back(structDeclaration);

        addOperatorDeclaration(Operator::Comma, structDeclaration, {nullptr, structDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::Assignment, structDeclaration, {structDeclaration, structDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::Equality, structDeclaration, {structDeclaration, structDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::Inequality, structDeclaration, {structDeclaration, structDeclaration}, declarationScopes);

        return structDeclaration;
    }

    FieldDeclaration* addFieldDeclaration(StructDeclaration* structDeclaration,
                                          const std::string& name,
                                          TypeDeclaration* type,
                                          bool isConst,
                                          std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        FieldDeclaration* fieldDeclaration;
        constructs.push_back(std::unique_ptr<Construct>(fieldDeclaration = new FieldDeclaration()));

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
        FunctionDeclaration* functionDeclaration;
        constructs.push_back(std::unique_ptr<Construct>(functionDeclaration = new FunctionDeclaration()));

        functionDeclaration->name = name;
        functionDeclaration->qualifiedType.typeDeclaration = resultType;

        for (TypeDeclaration* parameter : parameters)
        {
            ParameterDeclaration* parameterDeclaration;
            constructs.push_back(std::unique_ptr<Construct>(parameterDeclaration = new ParameterDeclaration()));

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
        OperatorDeclaration* operatorDeclaration;
        constructs.push_back(std::unique_ptr<Construct>(operatorDeclaration = new OperatorDeclaration()));

        operatorDeclaration->op = op;
        operatorDeclaration->qualifiedType.typeDeclaration = resultType;

        for (TypeDeclaration* parameter : parameters)
        {
            ParameterDeclaration* parameterDeclaration;
            constructs.push_back(std::unique_ptr<Construct>(parameterDeclaration = new ParameterDeclaration()));

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
};

#endif // PARSER_HPP
