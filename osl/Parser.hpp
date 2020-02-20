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
#include "Declarations.hpp"
#include "Expressions.hpp"
#include "Statements.hpp"
#include "QualifiedType.hpp"

namespace ouzel
{
    class ParseError final: public std::logic_error
    {
    public:
        explicit ParseError(const std::string& str): std::logic_error(str) {}
        explicit ParseError(const char* str): std::logic_error(str) {}
    };

    class ASTContext final
    {
    public:
        ASTContext() = default;
        explicit ASTContext(const std::vector<Token>& tokens);

        void dump() const;

        inline const std::vector<Declaration*>& getDeclarations() const { return declarations; }

    private:
        static Declaration* findDeclaration(const std::string& name, const std::vector<Declaration*>& declarationScope)
        {
            for (auto declarationIterator = declarationScope.crbegin(); declarationIterator != declarationScope.crend(); ++declarationIterator)
                if ((*declarationIterator)->name == name) return *declarationIterator;

            return nullptr;
        }

        static Declaration* findDeclaration(const std::string& name, const std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            for (auto scopeIterator = declarationScopes.crbegin(); scopeIterator != declarationScopes.crend(); ++scopeIterator)
                for (auto declarationIterator = scopeIterator->crbegin(); declarationIterator != scopeIterator->crend(); ++declarationIterator)
                    if ((*declarationIterator)->name == name) return *declarationIterator;

            return nullptr;
        }

        Type* findType(const std::string& name, const std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            Declaration* declaration = findDeclaration(name, declarationScopes);

            if (declaration && declaration->getDeclarationKind() == Declaration::Kind::Type)
                return static_cast<TypeDeclaration*>(declaration)->type;

            for (const auto& type : types)
                if (type->name == name)
                    return type.get();

            return nullptr;
        }

        StructType* findStructType(const std::string& name, const std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            Type* type = findType(name, declarationScopes);

            if (type && type->getTypeKind() == Type::Kind::Struct)
                return static_cast<StructType*>(type);

            return nullptr;
        }

        const VectorType* findVectorType(const Type* componentType, size_t components)
        {
            auto vectorTypeIterator = vectorTypes.find(std::make_pair(componentType, static_cast<uint8_t>(components)));

            if (vectorTypeIterator == vectorTypes.end())
                return nullptr;

            return vectorTypeIterator->second;
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

                        auto callableDeclaration = static_cast<CallableDeclaration*>(*declarationIterator);

                        if (callableDeclaration->getCallableDeclarationKind() != CallableDeclaration::Kind::Function) return nullptr;

                        auto functionDeclaration = static_cast<FunctionDeclaration*>(callableDeclaration);

                        if (functionDeclaration->parameterDeclarations.size() == parameters.size())
                        {
                            if (std::equal(parameters.begin(), parameters.end(),
                                           functionDeclaration->parameterDeclarations.begin(),
                                           [](const QualifiedType& qualifiedType,
                                              const ParameterDeclaration* parameterDeclaration) {
                                               return qualifiedType.type == parameterDeclaration->qualifiedType.type;
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

        const ArrayType* getArrayType(const Type* type, size_t size);

        bool isType(std::vector<Token>::const_iterator iterator,
                    std::vector<Token>::const_iterator end,
                    std::vector<std::vector<Declaration*>>& declarationScopes);

        const Type* parseType(std::vector<Token>::const_iterator& iterator,
                              std::vector<Token>::const_iterator end,
                              std::vector<std::vector<Declaration*>>& declarationScopes);

        std::pair<Semantic, size_t> parseSemantic(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end);

        bool isDeclaration(std::vector<Token>::const_iterator iterator,
                           std::vector<Token>::const_iterator end,
                           std::vector<std::vector<Declaration*>>& declarationScopes);

        Declaration* parseTopLevelDeclaration(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes);

        struct Specifiers
        {
            Qualifiers qualifiers = Qualifiers::None;
            bool isInline = false;
            StorageClass storageClass = StorageClass::Auto;
        };

        static Specifiers parseSpecifiers(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end);

        Declaration* parseDeclaration(std::vector<Token>::const_iterator& iterator,
                                      std::vector<Token>::const_iterator end,
                                      std::vector<std::vector<Declaration*>>& declarationScopes);

        Declaration* parseFunctionDeclaration(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes);

        Declaration* parseVariableDeclaration(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes);

        TypeDeclaration* parseStructTypeDeclaration(std::vector<Token>::const_iterator& iterator,
                                                    std::vector<Token>::const_iterator end,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes);

        Declaration* parseMemberDeclaration(std::vector<Token>::const_iterator& iterator,
                                            std::vector<Token>::const_iterator end,
                                            std::vector<std::vector<Declaration*>>& declarationScopes);

        ParameterDeclaration* parseParameterDeclaration(std::vector<Token>::const_iterator& iterator,
                                                        std::vector<Token>::const_iterator end,
                                                        std::vector<std::vector<Declaration*>>& declarationScopes);

        /*TypeDefinitionDeclaration* parseTypeDefinitionDeclaration(const std::vector<Token>& tokens,
                                                                    std::vector<Token>::const_iterator& iterator,
                                                                    std::vector<std::vector<Declaration*>>& declarationScopes);*/

        Statement* parseStatement(std::vector<Token>::const_iterator& iterator,
                                  std::vector<Token>::const_iterator end,
                                  std::vector<std::vector<Declaration*>>& declarationScopes);

        CompoundStatement* parseCompoundStatement(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes);

        IfStatement* parseIfStatement(std::vector<Token>::const_iterator& iterator,
                                      std::vector<Token>::const_iterator end,
                                      std::vector<std::vector<Declaration*>>& declarationScopes);

        ForStatement* parseForStatement(std::vector<Token>::const_iterator& iterator,
                                        std::vector<Token>::const_iterator end,
                                        std::vector<std::vector<Declaration*>>& declarationScopes);

        SwitchStatement* parseSwitchStatement(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes);

        CaseStatement* parseCaseStatement(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes);

        DefaultStatement* parseDefaultStatement(std::vector<Token>::const_iterator& iterator,
                                                std::vector<Token>::const_iterator end,
                                                std::vector<std::vector<Declaration*>>& declarationScopes);

        WhileStatement* parseWhileStatement(std::vector<Token>::const_iterator& iterator,
                                            std::vector<Token>::const_iterator end,
                                            std::vector<std::vector<Declaration*>>& declarationScopes);

        DoStatement* parseDoStatement(std::vector<Token>::const_iterator& iterator,
                                      std::vector<Token>::const_iterator end,
                                      std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parsePrimaryExpression(std::vector<Token>::const_iterator& iterator,
                                           std::vector<Token>::const_iterator end,
                                           std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parsePostfixExpression(std::vector<Token>::const_iterator& iterator,
                                           std::vector<Token>::const_iterator end,
                                           std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseSubscriptExpression(std::vector<Token>::const_iterator& iterator,
                                             std::vector<Token>::const_iterator end,
                                             std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseMemberExpression(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parsePrefixExpression(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseSignExpression(std::vector<Token>::const_iterator& iterator,
                                        std::vector<Token>::const_iterator end,
                                        std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseNotExpression(std::vector<Token>::const_iterator& iterator,
                                       std::vector<Token>::const_iterator end,
                                       std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseSizeofExpression(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseMultiplicationExpression(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseAdditionExpression(std::vector<Token>::const_iterator& iterator,
                                            std::vector<Token>::const_iterator end,
                                            std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseLessThanExpression(std::vector<Token>::const_iterator& iterator,
                                            std::vector<Token>::const_iterator end,
                                            std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseGreaterThanExpression(std::vector<Token>::const_iterator& iterator,
                                               std::vector<Token>::const_iterator end,
                                               std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseEqualityExpression(std::vector<Token>::const_iterator& iterator,
                                            std::vector<Token>::const_iterator end,
                                            std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseLogicalAndExpression(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseLogicalOrExpression(std::vector<Token>::const_iterator& iterator,
                                             std::vector<Token>::const_iterator end,
                                             std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseTernaryExpression(std::vector<Token>::const_iterator& iterator,
                                           std::vector<Token>::const_iterator end,
                                           std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseAdditionAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                                      std::vector<Token>::const_iterator end,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseMultiplicationAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                                            std::vector<Token>::const_iterator end,
                                                            std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseCommaExpression(std::vector<Token>::const_iterator& iterator,
                                         std::vector<Token>::const_iterator end,
                                         std::vector<std::vector<Declaration*>>& declarationScopes);

        Expression* parseExpression(std::vector<Token>::const_iterator& iterator,
                                    std::vector<Token>::const_iterator end,
                                    std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            return parseCommaExpression(iterator, end, declarationScopes);
        }

        ScalarType* addScalarType(const std::string& name,
                                  ScalarType::Kind kind,
                                  uint32_t size,
                                  bool isUnsigned,
                                  std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            ScalarType* scalarType;
            types.push_back(std::unique_ptr<Type>(scalarType = new ScalarType(kind)));

            scalarType->name = name;
            scalarType->size = size;
            scalarType->isUnsigned = isUnsigned;

            return scalarType;
        }

        StructType* addStructType(const std::string& name,
                                  uint32_t size,
                                  std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            StructType* structType;
            types.push_back(std::unique_ptr<Type>(structType = new StructType()));

            structType->name = name;
            structType->size = size;

            return structType;
        }

        VectorType* addVectorType(const std::string& name,
                                  const ScalarType* componentType,
                                  uint8_t componentCount,
                                  std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            VectorType* vectorType;
            types.push_back(std::unique_ptr<Type>(vectorType = new VectorType()));

            vectorType->name = name;
            vectorType->size = componentType->size * componentCount;
            vectorType->componentType = componentType;
            vectorType->componentCount = componentCount;

            vectorTypes[std::make_pair(componentType, componentCount)] = vectorType;

            return vectorType;
        }

        MatrixType* addMatrixType(const std::string& name,
                                  const ScalarType* componentType,
                                  uint8_t rowCount,
                                  uint8_t columnCount,
                                  std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            MatrixType* matrixType;
            types.push_back(std::unique_ptr<Type>(matrixType = new MatrixType()));

            matrixType->name = name;
            matrixType->size = componentType->size * rowCount * columnCount;
            matrixType->componentType = componentType;
            matrixType->rowCount = rowCount;
            matrixType->columnCount = columnCount;

            ConstructorDeclaration* constructorDeclaration;
            constructs.push_back(std::unique_ptr<Construct>(constructorDeclaration = new ConstructorDeclaration()));

            declarationScopes.back().push_back(constructorDeclaration);
            
            return matrixType;
        }

        FieldDeclaration* addFieldDeclaration(StructType* structType,
                                              const std::string& name,
                                              Type* type,
                                              bool isConst,
                                              std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            FieldDeclaration* fieldDeclaration;
            constructs.push_back(std::unique_ptr<Construct>(fieldDeclaration = new FieldDeclaration()));

            fieldDeclaration->name = name;
            fieldDeclaration->qualifiedType.type = type;
            fieldDeclaration->qualifiedType.qualifiers = (isConst ? Qualifiers::Const : Qualifiers::None);
            declarationScopes.back().push_back(fieldDeclaration);

            structType->memberDeclarations.push_back(fieldDeclaration);

            return fieldDeclaration;
        }

        FunctionDeclaration* addBuiltinFunctionDeclaration(const std::string& name,
                                                           Type* resultType,
                                                           const std::vector<Type*>& parameters,
                                                           std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            FunctionDeclaration* functionDeclaration;
            constructs.push_back(std::unique_ptr<Construct>(functionDeclaration = new FunctionDeclaration()));

            functionDeclaration->name = name;
            functionDeclaration->qualifiedType.type = resultType;

            for (auto parameter : parameters)
            {
                ParameterDeclaration* parameterDeclaration;
                constructs.push_back(std::unique_ptr<Construct>(parameterDeclaration = new ParameterDeclaration()));

                parameterDeclaration->parent = functionDeclaration;
                parameterDeclaration->qualifiedType.type = parameter;
                functionDeclaration->parameterDeclarations.push_back(parameterDeclaration);
            }

            functionDeclaration->isBuiltin = true;
            declarationScopes.back().push_back(functionDeclaration);

            return functionDeclaration;
        }

        std::vector<std::unique_ptr<Type>> types;
        std::vector<Declaration*> declarations;
        std::vector<std::unique_ptr<Construct>> constructs;

        std::map<std::pair<const Type*, uint8_t>, const VectorType*> vectorTypes;
        std::map<std::pair<QualifiedType, size_t>, const ArrayType*> arrayTypes;

        const Type* voidType = nullptr;
        const ScalarType* boolType = nullptr;
        const ScalarType* intType = nullptr;
        const ScalarType* uintType = nullptr;
        const ScalarType* floatType = nullptr;
        const StructType* stringType = nullptr;
    };
}

#endif // PARSER_HPP
