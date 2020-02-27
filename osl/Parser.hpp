//
//  OSL
//

#ifndef PARSER_HPP
#define PARSER_HPP

#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>
#include "Tokenizer.hpp"
#include "Declarations.hpp"
#include "Expressions.hpp"
#include "Statements.hpp"
#include "Utils.hpp"

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
        explicit ASTContext(const std::vector<Token>& tokens)
        {
            std::vector<std::vector<Declaration*>> declarationScopes;
            declarationScopes.push_back(std::vector<Declaration*>());

            addBuiltinFunctionDeclaration("discard", nullptr, {}, declarationScopes);

            auto voidTypePtr = create<Type>(Type::Kind::Void, 0);
            voidTypePtr->name = "void";
            voidType = voidTypePtr;

            boolType = addScalarType("bool", ScalarType::Kind::Boolean, 1, false, declarationScopes);
            intType = addScalarType("int", ScalarType::Kind::Integer, 4, false, declarationScopes);
            uintType = addScalarType("uint", ScalarType::Kind::Integer, 4, true, declarationScopes);
            floatType = addScalarType("float", ScalarType::Kind::FloatingPoint, 4, false, declarationScopes);

            for (uint8_t components = 2; components <= 4; ++components)
            {
                auto vectorType = addVectorType("float" + std::to_string(components),
                                                floatType, components, declarationScopes);

                auto matrixType = addMatrixType("float" + std::to_string(components) + 'x' + std::to_string(components),
                                                floatType, components, components, declarationScopes);

                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Multiplication, vectorType, matrixType, vectorType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Multiplication, vectorType, vectorType, matrixType);
            }

            stringType = addStructType("string", 8, declarationScopes);
            auto texture2DType = addStructType("Texture2D", 0, declarationScopes);

            //addBuiltinFunctionDeclaration("sample", float4Type, {texture2DType, float2Type}, declarationScopes);

            auto texture2DMSType = addStructType("Texture2DMS", 0, declarationScopes);

            //addBuiltinFunctionDeclaration("load", float4Type, {texture2DMSType, float2Type}, declarationScopes);

            for (auto iterator = tokens.begin(); iterator != tokens.end();)
            {
                auto declaration = parseTopLevelDeclaration(iterator, tokens.end(), declarationScopes);
                declarations.push_back(declaration);
            }
        }

        void dump() const
        {
            for (auto declaration : declarations)
                dumpConstruct(declaration);
        }

        inline const std::vector<Declaration*>& getDeclarations() const noexcept
        {
            return declarations;
        }

    private:
        static bool isToken(Token::Type tokenType,
                            std::vector<Token>::const_iterator iterator,
                            std::vector<Token>::const_iterator end) noexcept
        {
            return iterator != end && iterator->type == tokenType;
        }

        template <size_t N>
        static bool isToken(const Token::Type (&tokenTypes)[N],
                            std::vector<Token>::const_iterator iterator,
                            std::vector<Token>::const_iterator end) noexcept
        {
            if (iterator == end) return false;

            for (auto tokenType : tokenTypes)
                if (iterator->type == tokenType) return true;

            return false;
        }

        static bool skipToken(Token::Type tokenType,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<Token>::const_iterator end) noexcept
        {
            bool result = iterator != end && iterator->type == tokenType;
            if (result) ++iterator;
            return result;
        }

        static const Token& expectToken(Token::Type tokenType,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<Token>::const_iterator end)
        {
            if (iterator == end)
                throw ParseError("Unexpected end of file");
            if (iterator->type != tokenType)
                throw ParseError("Expected " + toString(tokenType));

            return *iterator++;
        }

        static bool isBooleanType(const Type* type)
        {
            if (type->getTypeKind() != Type::Kind::Scalar)
                return false;

            auto scalarType = static_cast<const ScalarType*>(type);

            return scalarType->getScalarTypeKind() == ScalarType::Kind::Boolean;
        }

        static bool isIntegerType(const Type* type) noexcept
        {
            if (type->getTypeKind() != Type::Kind::Scalar)
                return false;

            auto scalarType = static_cast<const ScalarType*>(type);

            return scalarType->getScalarTypeKind() == ScalarType::Kind::Boolean ||
                scalarType->getScalarTypeKind() == ScalarType::Kind::Integer;
        }

        static Declaration* findDeclaration(const std::string& name,
                                            const std::vector<Declaration*>& declarationScope)
        {
            for (auto declarationIterator = declarationScope.crbegin(); declarationIterator != declarationScope.crend(); ++declarationIterator)
                if ((*declarationIterator)->name == name) return *declarationIterator;

            return nullptr;
        }

        static size_t parseIndex(std::vector<Token>::const_iterator& iterator,
                                 std::vector<Token>::const_iterator end)
        {
            size_t result = 0;

            if (skipToken(Token::Type::LeftParenthesis, iterator, end))
            {
                int index = std::stoi(expectToken(Token::Type::IntLiteral, iterator, end).value);
                if (index < 0)
                    throw ParseError("Index must be positibe");

                result = static_cast<size_t>(index);

                expectToken(Token::Type::RightParenthesis, iterator, end);
            }

            return result;
        }

        static Declaration* findDeclaration(const std::string& name,
                                            const std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            for (auto scopeIterator = declarationScopes.crbegin(); scopeIterator != declarationScopes.crend(); ++scopeIterator)
                for (auto declarationIterator = scopeIterator->crbegin(); declarationIterator != scopeIterator->crend(); ++declarationIterator)
                    if ((*declarationIterator)->name == name) return *declarationIterator;

            return nullptr;
        }

        Type* findType(const std::string& name,
                       const std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto declaration = findDeclaration(name, declarationScopes);

            if (declaration && declaration->getDeclarationKind() == Declaration::Kind::Type)
                return static_cast<TypeDeclaration*>(declaration)->type;

            for (const auto& type : types)
                if (type->name == name)
                    return type.get();

            return nullptr;
        }

        StructType* findStructType(const std::string& name,
                                   const std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto type = findType(name, declarationScopes);

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
                for (auto declarationIterator = scopeIterator->crbegin(); declarationIterator != scopeIterator->crend(); ++declarationIterator)
                    if ((*declarationIterator)->name == name)
                    {
                        if ((*declarationIterator)->getDeclarationKind() != Declaration::Kind::Callable) return nullptr;

                        auto callableDeclaration = static_cast<CallableDeclaration*>(*declarationIterator);

                        if (callableDeclaration->getCallableDeclarationKind() != CallableDeclaration::Kind::Function) return nullptr;

                        auto functionDeclaration = static_cast<FunctionDeclaration*>(callableDeclaration);

                        if (functionDeclaration->parameterDeclarations.size() == parameters.size() &&
                            std::equal(parameters.begin(), parameters.end(),
                            functionDeclaration->parameterDeclarations.begin(),
                            [](const QualifiedType& qualifiedType,
                               const ParameterDeclaration* parameterDeclaration) {
                                return qualifiedType.type == parameterDeclaration->qualifiedType.type;
                            }))
                            return functionDeclaration;
                    }

            return nullptr;
        }

        static const FunctionDeclaration* resolveFunctionDeclaration(const std::string& name,
                                                                     const std::vector<std::vector<Declaration*>>& declarationScopes,
                                                                     const std::vector<QualifiedType>& arguments)
        {
            std::vector<const FunctionDeclaration*> candidateFunctionDeclarations;

            for (auto scopeIterator = declarationScopes.crbegin(); scopeIterator != declarationScopes.crend(); ++scopeIterator)
                for (auto declarationIterator = scopeIterator->crbegin(); declarationIterator != scopeIterator->crend(); ++declarationIterator)
                    if ((*declarationIterator)->name == name)
                    {
                        if ((*declarationIterator)->getDeclarationKind() != Declaration::Kind::Callable) return nullptr;

                        auto callableDeclaration = static_cast<const CallableDeclaration*>(*declarationIterator);

                        if (callableDeclaration->getCallableDeclarationKind() != CallableDeclaration::Kind::Function) return nullptr;

                        auto functionDeclaration = static_cast<const FunctionDeclaration*>(callableDeclaration->firstDeclaration);

                        if (std::find(candidateFunctionDeclarations.begin(),
                                      candidateFunctionDeclarations.end(),
                                      functionDeclaration) == candidateFunctionDeclarations.end())
                            candidateFunctionDeclarations.push_back(functionDeclaration);
                    }

            std::vector<const FunctionDeclaration*> viableFunctionDeclarations;

            for (auto functionDeclaration : candidateFunctionDeclarations)
                if (functionDeclaration->parameterDeclarations.size() == arguments.size() &&
                    std::equal(arguments.begin(), arguments.end(),
                               functionDeclaration->parameterDeclarations.begin(),
                               [](const QualifiedType& qualifiedType, const ParameterDeclaration* parameterDeclaration) {
                                   bool scalar = qualifiedType.type->getTypeKind() == Type::Kind::Scalar &&
                                       qualifiedType.type->getTypeKind() == Type::Kind::Scalar;

                                   return (scalar || qualifiedType.type == parameterDeclaration->qualifiedType.type);
                               }))
                    viableFunctionDeclarations.push_back(functionDeclaration);

            if (viableFunctionDeclarations.empty())
                throw ParseError("No matching function to call " + name + " found");
            else if (viableFunctionDeclarations.size() == 1)
                return *viableFunctionDeclarations.begin();
            else
            {
                if (arguments.empty()) // two or more functions with zero parameters
                    throw ParseError("Ambiguous call to " + name);

                const FunctionDeclaration* result = nullptr;

                for (auto viableFunctionDeclaration : viableFunctionDeclarations)
                    if (arguments.size() == viableFunctionDeclaration->parameterDeclarations.size())
                    {
                        bool valid = true;
                        for (size_t i = 0; i < arguments.size(); ++i)
                        {
                            const QualifiedType& argument = arguments[i];
                            const QualifiedType& parameter = viableFunctionDeclaration->parameterDeclarations[i]->qualifiedType;

                            if (parameter.type != argument.type)
                            {
                                valid = false;
                                break;
                            }
                        }

                        if (valid)
                        {
                            if (result)
                                throw ParseError("Ambiguous call to " + name);
                            else
                                result = viableFunctionDeclaration;
                        }
                    }

                return result;
            }
        }

        const ArrayType* getArrayType(const Type* type, size_t count)
        {
            QualifiedType qualifiedType;
            qualifiedType.type = type;

            auto i = arrayTypes.find(std::make_pair(qualifiedType, count));

            if (i == arrayTypes.end())
            {
                auto result = create<ArrayType>(qualifiedType, count);
                result->elementType = qualifiedType;

                arrayTypes[std::make_pair(qualifiedType, count)] = result;
                return result;
            }
            else
                return i->second;
        }

        bool isType(std::vector<Token>::const_iterator iterator,
                    std::vector<Token>::const_iterator end,
                    std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            if (iterator == end)
                throw ParseError("Unexpected end of file");

            return iterator->type == Token::Type::Void ||
                iterator->type == Token::Type::Bool ||
                iterator->type == Token::Type::Int ||
                iterator->type == Token::Type::Float ||
                iterator->type == Token::Type::Double ||
                (iterator->type == Token::Type::Identifier &&
                 findType(iterator->value, declarationScopes));
        }

        const Type* parseType(std::vector<Token>::const_iterator& iterator,
                              std::vector<Token>::const_iterator end,
                              std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            if (iterator == end)
                throw ParseError("Unexpected end of file");

            const Type* result;

            if (iterator->type == Token::Type::Void)
                result = voidType;
            else if (iterator->type == Token::Type::Bool)
                result = boolType;
            else if (iterator->type == Token::Type::Int)
                result = intType;
            else if (iterator->type == Token::Type::Float)
                result = floatType;
            else if (iterator->type == Token::Type::Double)
                throw ParseError("Double precision floating point numbers are not supported");
            else if (iterator->type == Token::Type::Identifier)
            {
                if (!(result = findType(iterator->value, declarationScopes)))
                    throw ParseError("Invalid type: " + iterator->value);
            }
            else
                throw ParseError("Expected a type name");

            ++iterator;

            while (skipToken(Token::Type::LeftBracket, iterator, end))
            {
                const int size = std::stoi(expectToken(Token::Type::IntLiteral, iterator, end).value);

                if (size <= 0)
                    throw ParseError("Array size must be greater than zero");

                result = getArrayType(result, static_cast<size_t>(size));

                expectToken(Token::Type::RightBracket, iterator, end);
            }

            return result;
        }

        Attribute* parseAttribute(std::vector<Token>::const_iterator& iterator,
                                  std::vector<Token>::const_iterator end)
        {
            if (iterator == end)
                throw ParseError("Unexpected end of file");

            const auto& name = expectToken(Token::Type::Identifier, iterator, end).value;

            if (name == "Binormal")
                return create<BinormalAttribute>(parseIndex(iterator, end));
            else if (name == "BlendIndices")
                return create<BlendIndicesAttribute>(parseIndex(iterator, end));
            else if (name == "BlendWeight")
                return create<BlendWeightAttribute>(parseIndex(iterator, end));
            else if (name == "Color")
                return create<ColorAttribute>(parseIndex(iterator, end));
            else if (name == "Depth")
                return create<DepthAttribute>(parseIndex(iterator, end));
            else if (name == "Fog")
                return create<FogAttribute>();
            else if (name == "Normal")
                return create<NormalAttribute>(parseIndex(iterator, end));
            else if (name == "Position")
                return create<PositionAttribute>(parseIndex(iterator, end));
            else if (name == "PositionTransformed")
                return create<PositionTransformedAttribute>();
            else if (name == "PointSize")
                return create<PointSizeAttribute>(parseIndex(iterator, end));
            else if (name == "Tangent")
                return create<TangentAttribute>(parseIndex(iterator, end));
            else if (name == "TesselationFactor")
                return create<TesselationFactorAttribute>(parseIndex(iterator, end));
            else if (name == "TextureCoordinates")
                return create<TextureCoordinatesAttribute>(parseIndex(iterator, end));
            else
                throw ParseError("Invalid attribute");
        }

        static bool isDeclaration(std::vector<Token>::const_iterator iterator,
                                  std::vector<Token>::const_iterator end)
        {
            if (iterator == end)
                throw ParseError("Unexpected end of file");

            return iterator->type == Token::Type::Const ||
                iterator->type == Token::Type::Extern ||
                iterator->type == Token::Type::Var;
        }

        Declaration* parseTopLevelDeclaration(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto declaration = parseDeclaration(iterator, end, declarationScopes);

            if (declaration->getDeclarationKind() == Declaration::Kind::Callable)
            {
                auto callableDeclaration = static_cast<const CallableDeclaration*>(declaration);

                // semicolon is not needed after a function definition
                if (!callableDeclaration->body)
                    expectToken(Token::Type::Semicolon, iterator, end);
            }
            else if (declaration->getDeclarationKind() == Declaration::Kind::Type)
            {
                auto typeDeclaration = static_cast<const TypeDeclaration*>(declaration);

                // semicolon is not needed after a struct definition
                if (typeDeclaration->definition != typeDeclaration)
                    expectToken(Token::Type::Semicolon, iterator, end);
            }
            else
                expectToken(Token::Type::Semicolon, iterator, end);

            return declaration;
        }

        Declaration* parseDeclaration(std::vector<Token>::const_iterator& iterator,
                                      std::vector<Token>::const_iterator end,
                                      std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            if (isToken(Token::Type::Semicolon, iterator, end))
            {
                auto declaration = create<Declaration>(Declaration::Kind::Empty);
                declarationScopes.back().push_back(declaration);

                return declaration;
            }
            else if (isToken(Token::Type::Struct, iterator, end))
                return parseStructTypeDeclaration(iterator, end, declarationScopes);
            else if (isToken(Token::Type::Function, iterator, end))
                return parseFunctionDeclaration(iterator, end, declarationScopes);
            else if (isToken({Token::Type::Const, Token::Type::Extern, Token::Type::Var}, iterator, end))
                return parseVariableDeclaration(iterator, end, declarationScopes);
            else
                throw ParseError("Unknown declaration type");
        }

        Declaration* parseFunctionDeclaration(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            expectToken(Token::Type::Function, iterator, end);

            auto result = create<FunctionDeclaration>();
            result->name = expectToken(Token::Type::Identifier, iterator, end).value;

            expectToken(Token::Type::LeftParenthesis, iterator, end);

            std::vector<QualifiedType> parameters;

            if (!isToken(Token::Type::RightParenthesis, iterator, end))
            {
                for (;;)
                {
                    auto parameterDeclaration = parseParameterDeclaration(iterator, end, declarationScopes);
                    result->parameterDeclarations.push_back(parameterDeclaration);
                    parameters.push_back(parameterDeclaration->qualifiedType);

                    if (!skipToken(Token::Type::Comma, iterator, end))
                        break;
                }
            }

            expectToken(Token::Type::RightParenthesis, iterator, end);

            // TODO: forbid declaring a function with the same name as a declared type (not supported by GLSL)
            auto previousDeclaration = findFunctionDeclaration(result->name, declarationScopes, parameters);

            if (previousDeclaration)
            {
                result->previousDeclaration = previousDeclaration;
                result->firstDeclaration = previousDeclaration->firstDeclaration;
                result->definition = previousDeclaration->definition;
            }
            else
                result->firstDeclaration = result;

            if (skipToken(Token::Type::Colon, iterator, end))
                result->qualifiedType.type = parseType(iterator, end, declarationScopes);

            if (skipToken(Token::Type::Arrow, iterator, end))
                while (isToken(Token::Type::Identifier, iterator, end))
                    result->attributes.push_back(parseAttribute(iterator, end));

            declarationScopes.back().push_back(result);

            if (isToken(Token::Type::LeftBrace, iterator, end))
            {
                // check if only one definition exists
                if (result->definition)
                    throw ParseError("Redefinition of " + result->name);

                declarationScopes.push_back(std::vector<Declaration*>()); // add scope for parameters

                for (auto parameterDeclaration : result->parameterDeclarations)
                    declarationScopes.back().push_back(parameterDeclaration);

                std::vector<ReturnStatement*> returnStatements;
                // parse body
                auto body = parseCompoundStatement(iterator, end, declarationScopes, returnStatements);
                result->body = body;

                if (!returnStatements.empty())
                    for (const auto returnStatement : returnStatements)
                    {
                        const auto returnType = returnStatement->result->qualifiedType.type;
                        if (!result->qualifiedType.type)
                            result->qualifiedType.type = returnType;
                        else if (result->qualifiedType.type != returnType)
                            throw ParseError("Failed to deduce the return type");
                    }
                else
                    result->qualifiedType.type = voidType;

                // set the definition pointer and type of all previous declarations
                Declaration* declaration = result;
                while (declaration)
                {
                    declaration->definition = result;
                    if (!declaration->qualifiedType.type)
                        declaration->qualifiedType.type = result->qualifiedType.type;
                    else if (declaration->qualifiedType.type != result->qualifiedType.type)
                        throw ParseError("Redeclaring function with a different return type");
                    declaration = declaration->previousDeclaration;
                }

                declarationScopes.pop_back();
            }

            return result;
        }

        Declaration* parseVariableDeclaration(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = create<VariableDeclaration>();

            if (skipToken(Token::Type::Const, iterator, end))
                result->qualifiedType.qualifiers = Qualifiers::Const;
            else if (skipToken(Token::Type::Extern, iterator, end))
                result->storageClass = StorageClass::Extern;
            else if (skipToken(Token::Type::Var, iterator, end))
                result->qualifiedType.qualifiers = Qualifiers::None;
            else
                throw ParseError("Expected a variable declaration");

            result->name = expectToken(Token::Type::Identifier, iterator, end).value;

            if (skipToken(Token::Type::Colon, iterator, end))
            {
                result->qualifiedType.type = parseType(iterator, end, declarationScopes);

                if (result->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                    throw ParseError("Variable can not have a void type");
            }

            if (skipToken(Token::Type::Assignment, iterator, end))
            {
                auto initialization = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);

                if (initialization->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                    throw ParseError("Initialization with a void type");

                result->initialization = initialization;

                if (!result->qualifiedType.type)
                    result->qualifiedType.type = initialization->qualifiedType.type;
                else if (result->qualifiedType.type != initialization->qualifiedType.type)
                    throw ParseError("Initializer type does not match the variable type");
            }

            if (!result->qualifiedType.type)
                throw ParseError("Missing type for the variable");

            declarationScopes.back().push_back(result);

            return result;
        }

        TypeDeclaration* parseStructTypeDeclaration(std::vector<Token>::const_iterator& iterator,
                                                    std::vector<Token>::const_iterator end,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            expectToken(Token::Type::Struct, iterator, end);

            auto result = create<TypeDeclaration>();
            result->name = expectToken(Token::Type::Identifier, iterator, end).value;

            auto previousDeclaration = findDeclaration(result->name, declarationScopes);

            StructType* structType = nullptr;

            if (previousDeclaration)
            {
                if (previousDeclaration->getDeclarationKind() != Declaration::Kind::Type)
                    throw ParseError("Redeclaration of " + result->name);

                auto typeDeclaration = static_cast<TypeDeclaration*>(previousDeclaration);

                if (typeDeclaration->type->getTypeKind() != Type::Kind::Struct)
                    throw ParseError("Redeclaration of " + result->name);

                structType = static_cast<StructType*>(typeDeclaration->type);

                result->firstDeclaration = typeDeclaration->firstDeclaration;
                result->previousDeclaration = typeDeclaration;
                result->definition = typeDeclaration->definition;
                result->type = structType;
            }
            else
            {
                structType = create<StructType>(0);
                structType->name = result->name;
                structType->declaration = result;
                result->firstDeclaration = result;
                result->type = structType;
            }

            declarationScopes.back().push_back(result);

            if (skipToken(Token::Type::LeftBrace, iterator, end))
            {
                // check if only one definition exists
                if (result->definition)
                    throw ParseError("Redefinition of " + result->name);

                // set the definition pointer of all previous declarations
                Declaration* declaration = result;
                while (declaration)
                {
                    declaration->definition = result;
                    declaration = declaration->previousDeclaration;
                }

                for (;;)
                {
                    if (skipToken(Token::Type::RightBrace, iterator, end))
                        break;
                    else
                    {
                        auto memberDeclaration = parseMemberDeclaration(iterator, end, declarationScopes);

                        expectToken(Token::Type::Semicolon, iterator, end);

                        if (structType->findMemberDeclaration(memberDeclaration->name))
                            throw ParseError("Redefinition of member " + memberDeclaration->name);

                        structType->memberDeclarations.push_back(memberDeclaration);

                        structType->size += memberDeclaration->qualifiedType.type->size;
                    }
                }
            }

            return result;
        }

        Declaration* parseMemberDeclaration(std::vector<Token>::const_iterator& iterator,
                                            std::vector<Token>::const_iterator end,
                                            std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            expectToken(Token::Type::Var, iterator, end);

            auto result = create<FieldDeclaration>();
            result->name = expectToken(Token::Type::Identifier, iterator, end).value;

            expectToken(Token::Type::Colon, iterator, end);

            result->qualifiedType.type = parseType(iterator, end, declarationScopes);

            if (result->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                throw ParseError("Member can not have a void type");

            if (result->qualifiedType.type->getTypeKind() == Type::Kind::Struct)
            {
                auto structType = static_cast<const StructType*>(result->qualifiedType.type);
                if (!structType->declaration->definition)
                    throw ParseError("Incomplete type " + result->qualifiedType.type->name);
            }

            if (skipToken(Token::Type::Arrow, iterator, end))
                while (isToken(Token::Type::Identifier, iterator, end))
                    result->attributes.push_back(parseAttribute(iterator, end));

            return result;
        }

        ParameterDeclaration* parseParameterDeclaration(std::vector<Token>::const_iterator& iterator,
                                                        std::vector<Token>::const_iterator end,
                                                        std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = create<ParameterDeclaration>();

            switch (iterator->type)
            {
                case Token::Type::In:
                    result->inputModifier = InputModifier::In;
                    ++iterator;
                    break;
                case Token::Type::Out:
                    result->inputModifier = InputModifier::Out;
                    ++iterator;
                    break;
                case Token::Type::Inout:
                    result->inputModifier = InputModifier::Inout;
                    ++iterator;
                    break;
                default:
                    break;
            }

            result->name = expectToken(Token::Type::Identifier, iterator, end).value;

            expectToken(Token::Type::Colon, iterator, end);

            result->qualifiedType.type = parseType(iterator, end, declarationScopes);

            if (result->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                throw ParseError("Parameter can not have a void type");

            if (skipToken(Token::Type::Arrow, iterator, end))
                while (isToken(Token::Type::Identifier, iterator, end))
                    result->attributes.push_back(parseAttribute(iterator, end));

            return result;
        }

        Statement* parseStatement(std::vector<Token>::const_iterator& iterator,
                                  std::vector<Token>::const_iterator end,
                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                  std::vector<ReturnStatement*>& returnStatements)
        {
            if (isToken(Token::Type::LeftBrace, iterator, end))
                return parseCompoundStatement(iterator, end, declarationScopes, returnStatements);
            else if (isToken(Token::Type::If, iterator, end))
                return parseIfStatement(iterator, end, declarationScopes, returnStatements);
            else if (isToken(Token::Type::For, iterator, end))
                return parseForStatement(iterator, end, declarationScopes, returnStatements);
            else if (isToken(Token::Type::Switch, iterator, end))
                return parseSwitchStatement(iterator, end, declarationScopes, returnStatements);
            else if (isToken(Token::Type::Case, iterator, end))
                return parseCaseStatement(iterator, end, declarationScopes, returnStatements);
            else if (isToken(Token::Type::Default, iterator, end))
                return parseDefaultStatement(iterator, end, declarationScopes, returnStatements);
            else if (isToken(Token::Type::While, iterator, end))
                return parseWhileStatement(iterator, end, declarationScopes, returnStatements);
            else if (isToken(Token::Type::Do, iterator, end))
                return parseDoStatement(iterator, end, declarationScopes, returnStatements);
            else if (skipToken(Token::Type::Break, iterator, end))
            {
                expectToken(Token::Type::Semicolon, iterator, end);
                return create<BreakStatement>();
            }
            else if (skipToken(Token::Type::Continue, iterator, end))
            {
                expectToken(Token::Type::Semicolon, iterator, end);
                return create<ContinueStatement>();
            }
            else if (skipToken(Token::Type::Return, iterator, end))
            {
                auto result = create<ReturnStatement>(parseExpression(iterator, end, declarationScopes));
                returnStatements.push_back(result);

                expectToken(Token::Type::Semicolon, iterator, end);

                return result;
            }
            else if (skipToken(Token::Type::Semicolon, iterator, end))
            {
                auto statement = create<Statement>(Statement::Kind::Empty);
                return statement;
            }
            else if (isToken(Token::Type::Asm, iterator, end))
                throw ParseError("asm statements are not supported");
            else if (isToken(Token::Type::Goto, iterator, end))
                throw ParseError("goto statements are not supported");
            else if (isToken({Token::Type::Try, Token::Type::Catch, Token::Type::Throw}, iterator, end))
                throw ParseError("Exceptions are not supported");
            else if (iterator == end || isToken(Token::Type::RightBrace, iterator, end))
                throw ParseError("Expected a statement");
            else if (isDeclaration(iterator, end))
            {
                auto declaration = parseDeclaration(iterator, end, declarationScopes);

                if (declaration->getDeclarationKind() != Declaration::Kind::Variable)
                    throw ParseError("Expected a variable declaration");

                expectToken(Token::Type::Semicolon, iterator, end);

                return create<DeclarationStatement>(declaration);
            }
            else
            {
                auto expression = parseExpression(iterator, end, declarationScopes);
                expectToken(Token::Type::Semicolon, iterator, end);
                return create<ExpressionStatement>(expression);
            }
        }

        CompoundStatement* parseCompoundStatement(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::LeftBrace, iterator, end);

            declarationScopes.push_back(std::vector<Declaration*>());

            auto result = create<CompoundStatement>();

            for (;;)
            {
                if (skipToken(Token::Type::RightBrace, iterator, end))
                    break;
                else
                {
                    auto statement = parseStatement(iterator, end, declarationScopes, returnStatements);
                    result->statements.push_back(statement);
                }
            }

            declarationScopes.pop_back();

            return result;
        }

        IfStatement* parseIfStatement(std::vector<Token>::const_iterator& iterator,
                                      std::vector<Token>::const_iterator end,
                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                      std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::If, iterator, end);
            expectToken(Token::Type::LeftParenthesis, iterator, end);

            auto result = create<IfStatement>();

            if (isDeclaration(iterator, end))
            {
                auto declaration = parseDeclaration(iterator, end, declarationScopes);

                if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
                    declaration->getDeclarationKind() != Declaration::Kind::Parameter)
                    throw ParseError("Expected a variable declaration");

                result->condition = declaration;
            }
            else
            {
                auto condition = parseExpression(iterator, end, declarationScopes);

                if (!isBooleanType(condition->qualifiedType.type))
                    throw ParseError("Condition is not a boolean");

                result->condition = condition;
            }

            expectToken(Token::Type::RightParenthesis, iterator, end);

            result->body = parseStatement(iterator, end, declarationScopes, returnStatements);

            if (skipToken(Token::Type::Else, iterator, end))
                result->elseBody = parseStatement(iterator, end, declarationScopes, returnStatements);

            return result;
        }

        ForStatement* parseForStatement(std::vector<Token>::const_iterator& iterator,
                                        std::vector<Token>::const_iterator end,
                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                        std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::For, iterator, end);
            expectToken(Token::Type::LeftParenthesis, iterator, end);

            auto result = create<ForStatement>();

            if (isDeclaration(iterator, end))
            {
                auto declaration = parseDeclaration(iterator, end, declarationScopes);

                if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
                    declaration->getDeclarationKind() != Declaration::Kind::Parameter)
                    throw ParseError("Expected a variable declaration");

                result->initialization = declaration;

                expectToken(Token::Type::Semicolon, iterator, end);
            }
            else if (skipToken(Token::Type::Semicolon, iterator, end))
                result->initialization = nullptr;
            else
            {
                auto initialization = parseExpression(iterator, end, declarationScopes);
                result->initialization = initialization;

                expectToken(Token::Type::Semicolon, iterator, end);
            }

            if (isDeclaration(iterator, end))
            {
                auto declaration = parseDeclaration(iterator, end, declarationScopes);

                if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
                    declaration->getDeclarationKind() != Declaration::Kind::Parameter)
                    throw ParseError("Expected a variable declaration");

                result->condition = declaration;

                expectToken(Token::Type::Semicolon, iterator, end);
            }
            else if (skipToken(Token::Type::Semicolon, iterator, end))
                result->condition = nullptr;
            else
            {
                auto condition = parseExpression(iterator, end, declarationScopes);

                if (!isBooleanType(condition->qualifiedType.type))
                    throw ParseError("Condition is not a boolean");

                result->condition = condition;

                expectToken(Token::Type::Semicolon, iterator, end);
            }

            if (skipToken(Token::Type::RightParenthesis, iterator, end))
                result->increment = nullptr;
            else
            {
                auto increment = parseExpression(iterator, end, declarationScopes);
                result->increment = increment;

                expectToken(Token::Type::RightParenthesis, iterator, end);
            }

            auto body = parseStatement(iterator, end, declarationScopes, returnStatements);
            result->body = body;

            return result;
        }

        SwitchStatement* parseSwitchStatement(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::Switch, iterator, end);
            expectToken(Token::Type::LeftParenthesis, iterator, end);

            auto result = create<SwitchStatement>();

            if (isDeclaration(iterator, end))
            {
                auto declaration = parseDeclaration(iterator, end, declarationScopes);

                if (declaration->getDeclarationKind() != Declaration::Kind::Variable)
                    throw ParseError("Expected a variable declaration");

                auto variableDeclaration = static_cast<const VariableDeclaration*>(declaration);

                if (!isIntegerType(variableDeclaration->qualifiedType.type))
                    throw ParseError("Statement requires expression of integer type");

                result->condition = declaration;
            }
            else
            {
                auto condition = parseExpression(iterator, end, declarationScopes);

                if (!isIntegerType(condition->qualifiedType.type))
                    throw ParseError("Statement requires expression of integer type");

                result->condition = condition;
            }

            expectToken(Token::Type::RightParenthesis, iterator, end);

            auto body = parseStatement(iterator, end, declarationScopes, returnStatements);
            result->body = body;

            return result;
        }

        CaseStatement* parseCaseStatement(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::Case, iterator, end);

            auto condition = parseExpression(iterator, end, declarationScopes);

            if (!isIntegerType(condition->qualifiedType.type))
                throw ParseError("Statement requires expression of integer type");

            if ((condition->qualifiedType.qualifiers & Qualifiers::Const) != Qualifiers::Const)
                throw ParseError("Expression must be constant");

            expectToken(Token::Type::Colon, iterator, end);

            auto body = parseStatement(iterator, end, declarationScopes, returnStatements);

            auto result = create<CaseStatement>();
            result->condition = condition;
            result->body = body;
            return result;
        }

        DefaultStatement* parseDefaultStatement(std::vector<Token>::const_iterator& iterator,
                                                std::vector<Token>::const_iterator end,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::Default, iterator, end);
            expectToken(Token::Type::Colon, iterator, end);

            return create<DefaultStatement>(parseStatement(iterator, end, declarationScopes, returnStatements));
        }

        WhileStatement* parseWhileStatement(std::vector<Token>::const_iterator& iterator,
                                            std::vector<Token>::const_iterator end,
                                            std::vector<std::vector<Declaration*>>& declarationScopes,
                                            std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::While, iterator, end);
            expectToken(Token::Type::LeftParenthesis, iterator, end);

            auto result = create<WhileStatement>();

            if (isDeclaration(iterator, end))
            {
                auto declaration = parseDeclaration(iterator, end, declarationScopes);

                if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
                    declaration->getDeclarationKind() != Declaration::Kind::Parameter)
                    throw ParseError("Expected a variable declaration");

                result->condition = declaration;
            }
            else
            {
                auto condition = parseExpression(iterator, end, declarationScopes);

                if (!isBooleanType(condition->qualifiedType.type))
                    throw ParseError("Condition is not a boolean");

                result->condition = condition;
            }

            expectToken(Token::Type::RightParenthesis, iterator, end);

            auto body = parseStatement(iterator, end, declarationScopes, returnStatements);
            result->body = body;

            return result;
        }

        DoStatement* parseDoStatement(std::vector<Token>::const_iterator& iterator,
                                      std::vector<Token>::const_iterator end,
                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                      std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::Do, iterator, end);

            auto body = parseStatement(iterator, end, declarationScopes, returnStatements);

            expectToken(Token::Type::While, iterator, end);
            expectToken(Token::Type::LeftParenthesis, iterator, end);

            auto condition = parseExpression(iterator, end, declarationScopes);

            if (!isBooleanType(condition->qualifiedType.type))
                throw ParseError("Condition is not a boolean");

            expectToken(Token::Type::RightParenthesis, iterator, end);
            expectToken(Token::Type::Semicolon, iterator, end);

            return create<DoStatement>(condition, body);
        }

        Expression* parsePrimaryExpression(std::vector<Token>::const_iterator& iterator,
                                           std::vector<Token>::const_iterator end,
                                           std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            if (isToken(Token::Type::IntLiteral, iterator, end))
            {
                auto result = create<IntegerLiteralExpression>(strtoll(iterator->value.c_str(), nullptr, 0));
                result->qualifiedType.type = intType;
                result->qualifiedType.qualifiers = Qualifiers::Const;
                result->category = Expression::Category::Rvalue;

                ++iterator;

                return result;
            }
            else if (isToken(Token::Type::FloatLiteral, iterator, end))
            {
                auto result = create<FloatingPointLiteralExpression>(strtod(iterator->value.c_str(), nullptr));
                result->qualifiedType.type = floatType;
                result->qualifiedType.qualifiers = Qualifiers::Const;
                result->category = Expression::Category::Rvalue;

                ++iterator;

                return result;
            }
            else if (isToken(Token::Type::DoubleLiteral, iterator, end))
                throw ParseError("Double precision floating point numbers are not supported");
            else if (isToken(Token::Type::StringLiteral, iterator, end))
            {
                auto result = create<StringLiteralExpression>(iterator->value);
                result->qualifiedType.type = stringType;
                result->qualifiedType.qualifiers = Qualifiers::Const;
                result->category = Expression::Category::Rvalue;

                ++iterator;

                return result;
            }
            else if (isToken({Token::Type::True, Token::Type::False}, iterator, end))
            {
                auto result = create<BooleanLiteralExpression>(iterator->type == Token::Type::True);
                result->qualifiedType.type = boolType;
                result->qualifiedType.qualifiers = Qualifiers::Const;
                result->category = Expression::Category::Rvalue;

                ++iterator;

                return result;
            }
            else if (isToken({Token::Type::Bool, Token::Type::Int, Token::Type::Float, Token::Type::Double}, iterator, end))
            {
                const Type* type = nullptr;

                if (skipToken(Token::Type::Bool, iterator, end)) type = boolType;
                else if(skipToken(Token::Type::Int, iterator, end)) type = intType;
                else if(skipToken(Token::Type::Float, iterator, end)) type = floatType;
                else if(skipToken(Token::Type::Double, iterator, end))
                    throw ParseError("Double precision floating point numbers are not supported");

                expectToken(Token::Type::LeftParenthesis, iterator, end);

                auto expression = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);
                auto result = create<CastExpression>(CastExpression::Kind::Functional, expression);
                result->qualifiedType.type = type;

                expectToken(Token::Type::RightParenthesis, iterator, end);

                return result;
            }
            else if (skipToken(Token::Type::LeftBrace, iterator, end))
            {
                auto result = create<InitializerListExpression>();

                const Type* type = nullptr;

                for (;;)
                {
                    auto expression = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);

                    if (!type)
                        type = expression->qualifiedType.type;
                    else if (type != expression->qualifiedType.type)
                        throw ParseError("Expression type does not match previous expressions in initializer list");

                    result->expressions.push_back(expression);

                    if (!skipToken(Token::Type::Comma, iterator, end))
                        break;
                }

                expectToken(Token::Type::RightBrace, iterator, end);

                result->qualifiedType.type = getArrayType(type, result->expressions.size());

                return result;
            }
            else if (isToken(Token::Type::Identifier, iterator, end))
            {
                std::string name = iterator->value;
                ++iterator;

                if (skipToken(Token::Type::LeftParenthesis, iterator, end))
                {
                    if (auto type = findType(name, declarationScopes))
                    {
                        std::vector<Expression*> parameters;
                        std::vector<QualifiedType> parameterTypes;

                        if (!skipToken(Token::Type::RightParenthesis, iterator, end)) // has arguments
                        {
                            for (;;)
                            {
                                auto parameter = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);

                                parameters.push_back(parameter);
                                parameterTypes.push_back(parameter->qualifiedType);

                                if (!skipToken(Token::Type::Comma, iterator, end))
                                    break;
                            }

                            expectToken(Token::Type::RightParenthesis, iterator, end);
                        }

                        switch (type->getTypeKind())
                        {
                            case Type::Kind::Struct:
                            {
                                auto structType = static_cast<const StructType*>(type);

                                auto result = create<TemporaryObjectExpression>();
                                result->qualifiedType.type = type;
                                result->qualifiedType.qualifiers = Qualifiers::Const;
                                result->category = Expression::Category::Rvalue;

                                for (auto parameter : parameters)
                                    result->parameters.push_back(parameter);

                                if (!(result->constructorDeclaration = structType->findConstructorDeclaration(parameterTypes)))
                                    throw ParseError("No matching constructor found");

                                return result;
                            }
                            case Type::Kind::Vector:
                            {
                                auto vectorType = static_cast<const VectorType*>(type);

                                auto result = create<VectorInitializeExpression>();
                                result->qualifiedType.type = vectorType;
                                result->qualifiedType.qualifiers = Qualifiers::Const;
                                result->category = Expression::Category::Rvalue;

                                size_t componentCount = 0;

                                for (auto parameter : parameters)
                                {
                                    auto parameterType = parameter->qualifiedType.type;
                                    if (parameterType->getTypeKind() == Type::Kind::Scalar)
                                    {
                                        if (parameterType != vectorType->componentType)
                                            throw ParseError("Invalid vector initialization");

                                        ++componentCount;
                                    }
                                    else if (parameterType->getTypeKind() == Type::Kind::Vector)
                                    {
                                        auto vectorParameterType = static_cast<const VectorType*>(parameterType);
                                        if (vectorParameterType->componentType != vectorType->componentType)
                                            throw ParseError("Invalid vector initialization");

                                        componentCount += vectorParameterType->componentCount;
                                    }

                                    result->parameters.push_back(parameter);
                                }

                                if (parameters.empty())
                                    throw ParseError(vectorType->name + " cannot not have an empty initializer");

                                if (componentCount != vectorType->componentCount)
                                    throw ParseError("Invalid vector initialization");

                                return result;
                            }
                            case Type::Kind::Matrix:
                            {
                                auto matrixType = static_cast<const MatrixType*>(type);

                                auto result = create<MatrixInitializeExpression>();
                                result->qualifiedType.type = matrixType;
                                result->qualifiedType.qualifiers = Qualifiers::Const;
                                result->category = Expression::Category::Rvalue;

                                size_t rowCount = 0;

                                for (auto parameter : parameters)
                                {
                                    auto parameterType = parameter->qualifiedType.type;
                                    if (parameterType->getTypeKind() == Type::Kind::Vector)
                                    {
                                        auto vectorParameterType = static_cast<const VectorType*>(parameterType);

                                        if (vectorParameterType->componentType != matrixType->componentType ||
                                            vectorParameterType->componentCount != matrixType->columnCount)
                                            throw ParseError("Invalid matrix initialization");

                                        ++rowCount;
                                    }
                                    else if (parameterType->getTypeKind() == Type::Kind::Matrix)
                                    {
                                        auto matrixParameterType = static_cast<const MatrixType*>(parameterType);

                                        if (matrixParameterType->componentType != matrixType->componentType ||
                                            matrixParameterType->rowCount != matrixType->rowCount ||
                                            matrixParameterType->columnCount != matrixType->columnCount)
                                            throw ParseError("Invalid matrix initialization");

                                        rowCount += matrixParameterType->rowCount;
                                    }

                                    result->parameters.push_back(parameter);
                                }

                                if (parameters.empty())
                                    throw ParseError(matrixType->name + " cannot not have an empty initializer");

                                if (rowCount != matrixType->rowCount)
                                    throw ParseError("Invalid matrix initialization");

                                return result;
                            }
                            default:
                                throw ParseError("Expected a struct type");
                        }
                    }
                    else
                    {
                        auto result = create<CallExpression>();

                        std::vector<QualifiedType> arguments;

                        if (!skipToken(Token::Type::RightParenthesis, iterator, end)) // has arguments
                        {
                            for (;;)
                            {
                                auto argument = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);

                                result->arguments.push_back(argument);
                                arguments.push_back(argument->qualifiedType);

                                if (!skipToken(Token::Type::Comma, iterator, end))
                                    break;
                            }

                            expectToken(Token::Type::RightParenthesis, iterator, end);
                        }

                        auto functionDeclaration = resolveFunctionDeclaration(name, declarationScopes, arguments);
                        if (!functionDeclaration)
                            throw ParseError("Invalid function reference: " + name);

                        auto declRefExpression = create<DeclarationReferenceExpression>(functionDeclaration);
                        declRefExpression->qualifiedType = functionDeclaration->qualifiedType;
                        declRefExpression->category = Expression::Category::Lvalue;
                        result->declarationReference = declRefExpression;
                        result->qualifiedType = functionDeclaration->qualifiedType;
                        result->category = Expression::Category::Rvalue;

                        return result;
                    }
                }
                else
                {
                    auto result = create<DeclarationReferenceExpression>(findDeclaration(name, declarationScopes));
                    if (!result->declaration)
                        throw ParseError("Invalid declaration reference: " + name);

                    switch (result->declaration->getDeclarationKind())
                    {
                        case Declaration::Kind::Type:
                        {
                            auto typeDeclaration = static_cast<const TypeDeclaration*>(result->declaration);
                            result->qualifiedType.type = typeDeclaration->type;
                            result->category = Expression::Category::Rvalue;
                            break;
                        }
                        case Declaration::Kind::Variable:
                        {
                            auto variableDeclaration = static_cast<const VariableDeclaration*>(result->declaration);
                            result->qualifiedType = variableDeclaration->qualifiedType;
                            result->category = Expression::Category::Lvalue;
                            break;
                        }
                        case Declaration::Kind::Parameter:
                        {
                            auto parameterDeclaration = static_cast<const ParameterDeclaration*>(result->declaration);
                            result->qualifiedType = parameterDeclaration->qualifiedType;
                            result->category = Expression::Category::Rvalue;
                            break;
                        }
                        default:
                            throw ParseError("Invalid declaration reference " + name);
                    }

                    return result;
                }
            }
            else if (skipToken(Token::Type::LeftParenthesis, iterator, end))
            {
                if (isType(iterator, end, declarationScopes))
                {
                    auto type = parseType(iterator, end, declarationScopes);

                    expectToken(Token::Type::RightParenthesis, iterator, end);

                    auto expression = parseExpression(iterator, end, declarationScopes);
                    auto result = create<CastExpression>(CastExpression::Kind::CStyle, expression);
                    result->qualifiedType.type = type;

                    result->expression = expression;
                    result->category = Expression::Category::Rvalue;

                    return result;
                }
                else
                {
                    auto expression = parseExpression(iterator, end, declarationScopes);
                    expectToken(Token::Type::RightParenthesis, iterator, end);

                    auto result = create<ParenExpression>(expression);
                    result->qualifiedType = result->expression->qualifiedType;
                    result->category = result->expression->category;
                    return result;
                }
            }
            else if (isToken(Token::Type::StaticCast, iterator, end))
            {
                CastExpression::Kind castKind;

                switch (iterator->type)
                {
                    case Token::Type::StaticCast: castKind = CastExpression::Kind::Static; break;
                    default: throw ParseError("Invalid cast");
                }

                ++iterator;

                expectToken(Token::Type::LessThan, iterator, end);

                auto type = parseType(iterator, end, declarationScopes);

                expectToken(Token::Type::GreaterThan, iterator, end);
                expectToken(Token::Type::LeftParenthesis, iterator, end);

                auto expression = parseExpression(iterator, end, declarationScopes);
                auto result = create<CastExpression>(castKind, expression);
                result->qualifiedType.type = type;

                result->expression = expression;

                expectToken(Token::Type::RightParenthesis, iterator, end);

                result->category = Expression::Category::Rvalue;

                return result;
            }
            else if (isToken(Token::Type::This, iterator, end))
                throw ParseError("Expression \"this\" is not supported");
            else
                throw ParseError("Expected an expression");
        }

        Expression* parsePostfixExpression(std::vector<Token>::const_iterator& iterator,
                                           std::vector<Token>::const_iterator end,
                                           std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parsePrimaryExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::Increment, Token::Type::Decrement}, iterator, end))
            {
                if (result->category != Expression::Category::Lvalue)
                    throw ParseError("Expression is not assignable");

                if ((result->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                    throw ParseError("Cannot assign to const variable");

                if (result->qualifiedType.type->getTypeKind() != Type::Kind::Scalar)
                    throw ParseError("Parameter of the postfix operator must be a number");

                const auto operatorKind =
                    (iterator->type == Token::Type::Increment) ? UnaryOperatorExpression::Kind::PostfixIncrement :
                    (iterator->type == Token::Type::Decrement) ? UnaryOperatorExpression::Kind::PostfixDecrement :
                    throw ParseError("Invalid operator");

                ++iterator;

                const auto& unaryOperator = getUnaryOperator(operatorKind,
                                                             result->qualifiedType.type);

                auto expression = create<UnaryOperatorExpression>(operatorKind, result);
                expression->qualifiedType.type = unaryOperator.resultType;
                expression->category = Expression::Category::Lvalue;

                result = expression;
            }

            return result;
        }

        Expression* parseSubscriptExpression(std::vector<Token>::const_iterator& iterator,
                                             std::vector<Token>::const_iterator end,
                                             std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parsePostfixExpression(iterator, end, declarationScopes);

            while (skipToken(Token::Type::LeftBracket, iterator, end))
            {
                if (result->qualifiedType.type->getTypeKind() == Type::Kind::Array)
                {
                    auto subscript = parseExpression(iterator, end, declarationScopes);
                    if (!isIntegerType(subscript->qualifiedType.type))
                        throw ParseError("Subscript is not an integer");

                    expectToken(Token::Type::RightBracket, iterator, end);

                    auto arrayType = static_cast<const ArrayType*>(result->qualifiedType.type);

                    auto expression = create<ArraySubscriptExpression>(result, subscript);
                    expression->qualifiedType = arrayType->elementType;
                    expression->category = result->category;

                    result = expression;
                }
                else if (result->qualifiedType.type->getTypeKind() == Type::Kind::Vector ||
                         result->qualifiedType.type->getTypeKind() == Type::Kind::Matrix)
                {
                    const auto operatorKind = BinaryOperatorExpression::Kind::Subscript;

                    auto rightExpression = parseExpression(iterator, end, declarationScopes);
                    if (!isIntegerType(rightExpression->qualifiedType.type))
                        throw ParseError("Subscript is not an integer");

                    expectToken(Token::Type::RightBracket, iterator, end);

                    auto expression = create<BinaryOperatorExpression>(operatorKind, result, rightExpression);

                    if (result->qualifiedType.type->getTypeKind() == Type::Kind::Vector)
                    {
                        auto vectorType = static_cast<const VectorType*>(result->qualifiedType.type);
                        expression->qualifiedType.type = vectorType->componentType;
                    }
                    else if (result->qualifiedType.type->getTypeKind() == Type::Kind::Matrix)
                    {
                        auto matrixType = static_cast<const MatrixType*>(result->qualifiedType.type);

                        auto vectorType = findVectorType(matrixType->componentType, matrixType->columnCount);
                        if (!vectorType)
                            throw ParseError("Invalid vector type");

                        expression->qualifiedType.type = vectorType;
                    }

                    expression->category = expression->leftExpression->category;

                    result = expression;
                }
                else
                    throw ParseError("Subscript value is not an array");
            }

            return result;
        }

        static constexpr uint8_t charToComponent(char c)
        {
            return (c == 'x' || c == 'r') ? 0 :
                (c == 'y' || c == 'g') ? 1 :
                (c == 'z' || c == 'b') ? 2 :
                (c == 'w' || c == 'a') ? 3 :
                throw ParseError("Invalid component");
        }

        Expression* parseMemberExpression(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parseSubscriptExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::Dot, Token::Type::Arrow}, iterator, end))
            {
                if (isToken(Token::Type::Arrow, iterator, end))
                    throw ParseError("Pointer member access is not supported");

                ++iterator;

                if (result->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                    throw ParseError("Expression has a void type");

                if (result->qualifiedType.type->getTypeKind() == Type::Kind::Struct)
                {
                    auto structType = static_cast<const StructType*>(result->qualifiedType.type);

                    auto& name = expectToken(Token::Type::Identifier, iterator, end).value;

                    auto memberDeclaration = structType->findMemberDeclaration(name);
                    if (!memberDeclaration)
                        throw ParseError("Structure " + structType->name +  " has no member " + name);

                    if (memberDeclaration->getDeclarationKind() != Declaration::Kind::Field)
                        throw ParseError(iterator->value + " is not a field");

                    auto expression = create<MemberExpression>(result, static_cast<const FieldDeclaration*>(memberDeclaration));
                    expression->qualifiedType = expression->fieldDeclaration->qualifiedType;
                    if ((result->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                        expression->qualifiedType.qualifiers |= Qualifiers::Const;
                    expression->category = result->category;

                    result = expression;
                }
                else if (result->qualifiedType.type->getTypeKind() == Type::Kind::Vector)
                {
                    std::vector<uint8_t> components;
                    std::set<uint8_t> componentSet;

                    auto expression = create<VectorElementExpression>();
                    expression->category = result->category;

                    const auto& token = expectToken(Token::Type::Identifier, iterator, end);

                    for (const auto c : token.value)
                    {
                        uint8_t component = charToComponent(c);
                        if (!componentSet.insert(component).second) // has component repeated
                        {
                            expression->category = Expression::Category::Rvalue;
                            expression->qualifiedType.qualifiers |= Qualifiers::Const;
                        }

                        components.push_back(component);

                        expression->positions[expression->count++] = component;
                    }

                    auto vectorType = static_cast<const VectorType*>(result->qualifiedType.type);

                    auto resultType = findVectorType(vectorType->componentType, components.size());
                    if (!resultType)
                        throw ParseError("Invalid swizzle");

                    expression->qualifiedType.type = resultType;

                    for (uint8_t component : components)
                        if (component >= vectorType->componentCount)
                            throw ParseError("Invalid swizzle");

                    result = expression;
                }
                else
                    throw ParseError(result->qualifiedType.type->name + " is not a structure");
            }

            return result;
        }

        Expression* parsePrefixExpression(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            if (isToken({Token::Type::Increment, Token::Type::Decrement}, iterator, end))
            {
                const auto operatorKind =
                    (iterator->type == Token::Type::Increment) ? UnaryOperatorExpression::Kind::PrefixIncrement :
                    (iterator->type == Token::Type::Decrement) ? UnaryOperatorExpression::Kind::PrefixDecrement :
                    throw ParseError("Invalid operator");

                ++iterator;

                auto expression = parseMemberExpression(iterator, end, declarationScopes);

                if (expression->category != Expression::Category::Lvalue)
                    throw ParseError("Expression is not assignable");

                if ((expression->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                    throw ParseError("Cannot assign to const variable");

                const auto& unaryOperator = getUnaryOperator(operatorKind,
                                                             expression->qualifiedType.type);

                auto result = create<UnaryOperatorExpression>(operatorKind, expression);
                result->qualifiedType.type = unaryOperator.resultType;
                result->category = Expression::Category::Rvalue;

                return result;
            }
            else
                return parseMemberExpression(iterator, end, declarationScopes);
        }

        Expression* parseSignExpression(std::vector<Token>::const_iterator& iterator,
                                        std::vector<Token>::const_iterator end,
                                        std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            if (isToken({Token::Type::Plus, Token::Type::Minus}, iterator, end))
            {
                const auto operatorKind =
                    (iterator->type == Token::Type::Plus) ? UnaryOperatorExpression::Kind::Positive :
                    (iterator->type == Token::Type::Minus) ? UnaryOperatorExpression::Kind::Negative :
                    throw ParseError("Invalid operator");

                ++iterator;

                auto expression = parsePrefixExpression(iterator, end, declarationScopes);

                const auto& unaryOperator = getUnaryOperator(operatorKind,
                                                             expression->qualifiedType.type);

                auto result = create<UnaryOperatorExpression>(operatorKind, expression);
                result->qualifiedType.type = unaryOperator.resultType;
                result->category = Expression::Category::Rvalue;

                return result;
            }
            else
                return parsePrefixExpression(iterator, end, declarationScopes);
        }

        Expression* parseNotExpression(std::vector<Token>::const_iterator& iterator,
                                       std::vector<Token>::const_iterator end,
                                       std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            if (skipToken(Token::Type::Not, iterator, end))
            {
                const auto operatorKind = UnaryOperatorExpression::Kind::Negation;

                auto expression = parseExpression(iterator, end, declarationScopes);

                const auto& unaryOperator = getUnaryOperator(operatorKind,
                                                             expression->qualifiedType.type);

                auto result = create<UnaryOperatorExpression>(operatorKind, expression);
                result->qualifiedType.type = unaryOperator.resultType;
                result->category = Expression::Category::Rvalue;

                return result;
            }
            else
                return parseSignExpression(iterator, end, declarationScopes);
        }

        Expression* parseSizeofExpression(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            if (skipToken(Token::Type::Sizeof, iterator, end))
            {
                expectToken(Token::Type::LeftParenthesis, iterator, end);

                if (iterator == end)
                    throw ParseError("Unexpected end of file");

                if (iterator->type == Token::Type::Void)
                    throw ParseError("Parameter of sizeof can not be void");

                SizeofExpression* result = nullptr;
                if (isType(iterator, end, declarationScopes))
                    result = create<SizeofExpression>(parseType(iterator, end, declarationScopes));
                else
                    result = create<SizeofExpression>(parseExpression(iterator, end, declarationScopes));

                expectToken(Token::Type::RightParenthesis, iterator, end);

                result->qualifiedType.type = uintType;
                result->category = Expression::Category::Rvalue;

                return result;
            }
            else
                return parseNotExpression(iterator, end, declarationScopes);
        }

        Expression* parseMultiplicationExpression(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parseSizeofExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::Multiply, Token::Type::Divide}, iterator, end))
            {
                const auto operatorKind =
                    (iterator->type == Token::Type::Multiply) ? BinaryOperatorExpression::Kind::Multiplication :
                    (iterator->type == Token::Type::Divide) ? BinaryOperatorExpression::Kind::Division :
                    throw ParseError("Invalid operator");

                ++iterator;

                auto rightExpression = parseSizeofExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression->qualifiedType.type);

                auto expression = create<BinaryOperatorExpression>(operatorKind, result, rightExpression);
                expression->qualifiedType.type = binaryOperator.resultType;
                expression->category = Expression::Category::Rvalue;

                result = expression;
            }

            return result;
        }

        Expression* parseAdditionExpression(std::vector<Token>::const_iterator& iterator,
                                            std::vector<Token>::const_iterator end,
                                            std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parseMultiplicationExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::Plus, Token::Type::Minus}, iterator, end))
            {
                const auto operatorKind =
                    (iterator->type == Token::Type::Plus) ? BinaryOperatorExpression::Kind::Addition :
                    (iterator->type == Token::Type::Minus) ? BinaryOperatorExpression::Kind::Subtraction :
                    throw ParseError("Invalid operator");

                ++iterator;

                auto rightExpression = parseMultiplicationExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression->qualifiedType.type);

                auto expression = create<BinaryOperatorExpression>(operatorKind, result, rightExpression);
                expression->qualifiedType.type = binaryOperator.resultType;
                expression->category = Expression::Category::Rvalue;

                result = expression;
            }

            return result;
        }

        Expression* parseLessThanExpression(std::vector<Token>::const_iterator& iterator,
                                            std::vector<Token>::const_iterator end,
                                            std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parseAdditionExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::LessThan, Token::Type::LessThanEqual}, iterator, end))
            {
                const auto operatorKind =
                    (iterator->type == Token::Type::LessThan) ? BinaryOperatorExpression::Kind::LessThan :
                    (iterator->type == Token::Type::LessThanEqual) ? BinaryOperatorExpression::Kind::LessThanEqual :
                    throw ParseError("Invalid operator");

                ++iterator;

                auto rightExpression = parseAdditionExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression->qualifiedType.type);

                auto expression = create<BinaryOperatorExpression>(operatorKind, result, rightExpression);
                expression->qualifiedType.type = binaryOperator.resultType;
                expression->category = Expression::Category::Rvalue;

                result = expression;
            }

            return result;
        }

        Expression* parseGreaterThanExpression(std::vector<Token>::const_iterator& iterator,
                                               std::vector<Token>::const_iterator end,
                                               std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parseLessThanExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::GreaterThan, Token::Type::GreaterThanEqual}, iterator, end))
            {
                const auto operatorKind =
                    (iterator->type == Token::Type::GreaterThan) ? BinaryOperatorExpression::Kind::GreaterThan :
                    (iterator->type == Token::Type::GreaterThanEqual) ? BinaryOperatorExpression::Kind::GraterThanEqual :
                    throw ParseError("Invalid operator");

                ++iterator;

                auto rightExpression = parseLessThanExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression->qualifiedType.type);

                auto expression = create<BinaryOperatorExpression>(operatorKind, result, rightExpression);
                expression->qualifiedType.type = binaryOperator.resultType;
                expression->category = Expression::Category::Rvalue;

                result = expression;
            }

            return result;
        }

        Expression* parseEqualityExpression(std::vector<Token>::const_iterator& iterator,
                                            std::vector<Token>::const_iterator end,
                                            std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parseGreaterThanExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::Equal, Token::Type::NotEq}, iterator, end))
            {
                const auto operatorKind =
                    (iterator->type == Token::Type::Equal) ? BinaryOperatorExpression::Kind::Equality :
                    (iterator->type == Token::Type::NotEq) ? BinaryOperatorExpression::Kind::Inequality :
                    throw ParseError("Invalid operator");

                ++iterator;

                auto rightExpression = parseGreaterThanExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression->qualifiedType.type);

                auto expression = create<BinaryOperatorExpression>(operatorKind, result, rightExpression);
                expression->qualifiedType.type = binaryOperator.resultType;
                expression->category = Expression::Category::Rvalue;

                result = expression;
            }

            return result;
        }

        Expression* parseLogicalAndExpression(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parseEqualityExpression(iterator, end, declarationScopes);

            while (skipToken(Token::Type::And, iterator, end))
            {
                const auto operatorKind = BinaryOperatorExpression::Kind::And;

                auto rightExpression = parseEqualityExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression->qualifiedType.type);

                auto expression = create<BinaryOperatorExpression>(operatorKind, result, rightExpression);
                expression->qualifiedType.type = binaryOperator.resultType;
                expression->category = Expression::Category::Rvalue;

                result = expression;
            }

            return result;
        }

        Expression* parseLogicalOrExpression(std::vector<Token>::const_iterator& iterator,
                                             std::vector<Token>::const_iterator end,
                                             std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parseLogicalAndExpression(iterator, end, declarationScopes);

            while (skipToken(Token::Type::Or, iterator, end))
            {
                const auto operatorKind = BinaryOperatorExpression::Kind::Or;

                auto rightExpression = parseLogicalAndExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression->qualifiedType.type);

                auto expression = create<BinaryOperatorExpression>(operatorKind, result, rightExpression);
                expression->qualifiedType.type = binaryOperator.resultType;
                expression->category = Expression::Category::Rvalue;

                result = expression;
            }

            return result;
        }

        Expression* parseTernaryExpression(std::vector<Token>::const_iterator& iterator,
                                           std::vector<Token>::const_iterator end,
                                           std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parseLogicalOrExpression(iterator, end, declarationScopes);

            while (skipToken(Token::Type::Conditional, iterator, end))
            {
                if (!isBooleanType(result->qualifiedType.type))
                    throw ParseError("Condition is not a boolean");

                auto leftExpression = parseTernaryExpression(iterator, end, declarationScopes);

                expectToken(Token::Type::Colon, iterator, end);

                auto rightExpression = parseTernaryExpression(iterator, end, declarationScopes);

                if (leftExpression->qualifiedType.type != rightExpression->qualifiedType.type)
                    throw ParseError("Incompatible operand types");

                auto expression = create<TernaryOperatorExpression>(result, leftExpression, rightExpression);
                expression->qualifiedType = expression->leftExpression->qualifiedType;
                expression->category = (expression->leftExpression->category == Expression::Category::Lvalue &&
                                        expression->rightExpression->category == Expression::Category::Lvalue) ?
                                        Expression::Category::Lvalue : Expression::Category::Rvalue;

                result = expression;
            }

            return result;
        }

        Expression* parseAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parseTernaryExpression(iterator, end, declarationScopes);

            while (skipToken(Token::Type::Assignment, iterator, end))
            {
                const auto operatorKind = BinaryOperatorExpression::Kind::Assignment;

                if (result->category != Expression::Category::Lvalue)
                    throw ParseError("Expression is not assignable");

                if ((result->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                    throw ParseError("Cannot assign to const variable");

                auto rightExpression = parseTernaryExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression->qualifiedType.type);

                auto expression = create<BinaryOperatorExpression>(operatorKind, result, rightExpression);
                expression->qualifiedType.type = binaryOperator.resultType;
                expression->category = Expression::Category::Rvalue;

                result = expression;
            }

            return result;
        }

        Expression* parseAdditionAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                                      std::vector<Token>::const_iterator end,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parseAssignmentExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::PlusAssignment, Token::Type::MinusAssignment}, iterator, end))
            {
                const auto operatorKind =
                    (iterator->type == Token::Type::PlusAssignment) ? BinaryOperatorExpression::Kind::AdditionAssignment :
                    (iterator->type == Token::Type::MinusAssignment) ? BinaryOperatorExpression::Kind::SubtractAssignment :
                    throw ParseError("Invalid operator");

                ++iterator;

                if (result->category != Expression::Category::Lvalue)
                    throw ParseError("Expression is not assignable");

                if ((result->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                    throw ParseError("Cannot assign to const variable");

                auto rightExpression = parseAssignmentExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression->qualifiedType.type);

                auto expression = create<BinaryOperatorExpression>(operatorKind, result, rightExpression);
                expression->qualifiedType.type = binaryOperator.resultType;
                expression->category = Expression::Category::Lvalue;

                result = expression;
            }

            return result;
        }

        Expression* parseMultiplicationAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                                            std::vector<Token>::const_iterator end,
                                                            std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parseAdditionAssignmentExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::MultiplyAssignment, Token::Type::DivideAssignment}, iterator, end))
            {
                const auto operatorKind =
                    (iterator->type == Token::Type::MultiplyAssignment) ? BinaryOperatorExpression::Kind::MultiplicationAssignment :
                    (iterator->type == Token::Type::DivideAssignment) ? BinaryOperatorExpression::Kind::DivisionAssignment :
                    throw ParseError("Invalid operator");

                ++iterator;

                if (result->category != Expression::Category::Lvalue)
                    throw ParseError("Expression is not assignable");

                if ((result->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                    throw ParseError("Cannot assign to const variable");

                auto rightExpression = parseAdditionAssignmentExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression->qualifiedType.type);

                auto expression = create<BinaryOperatorExpression>(operatorKind, result, rightExpression);
                expression->qualifiedType.type = binaryOperator.resultType;
                expression->category = Expression::Category::Lvalue;

                result = expression;
            }

            return result;
        }

        Expression* parseCommaExpression(std::vector<Token>::const_iterator& iterator,
                                         std::vector<Token>::const_iterator end,
                                         std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto result = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);

            while (skipToken(Token::Type::Comma, iterator, end))
            {
                const auto operatorKind = BinaryOperatorExpression::Kind::Comma;

                auto rightExpression = parseAdditionAssignmentExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression->qualifiedType.type);

                auto expression = create<BinaryOperatorExpression>(operatorKind, result, rightExpression);
                expression->qualifiedType.type = binaryOperator.resultType;
                expression->category = expression->rightExpression->category;

                if (expression->leftExpression->qualifiedType.type !=
                    expression->rightExpression->qualifiedType.type)
                    throw ParseError("Incompatible operand types");

                result = expression;
            }

            return result;
        }

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
            auto scalarType = create<ScalarType>(kind, size, isUnsigned);
            scalarType->name = name;

            if (kind == ScalarType::Kind::Boolean)
            {
                unaryOperators.emplace_back(UnaryOperatorExpression::Kind::Negation, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Equality, scalarType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Inequality, scalarType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Assignment, scalarType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Or, scalarType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::And, scalarType, scalarType, scalarType);
            }
            else
            {
                unaryOperators.emplace_back(UnaryOperatorExpression::Kind::Positive, scalarType, scalarType);
                unaryOperators.emplace_back(UnaryOperatorExpression::Kind::Negative, scalarType, scalarType);
                unaryOperators.emplace_back(UnaryOperatorExpression::Kind::PrefixIncrement, scalarType, scalarType);
                unaryOperators.emplace_back(UnaryOperatorExpression::Kind::PrefixDecrement, scalarType, scalarType);
                unaryOperators.emplace_back(UnaryOperatorExpression::Kind::PostfixIncrement, scalarType, scalarType);
                unaryOperators.emplace_back(UnaryOperatorExpression::Kind::PostfixDecrement, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Addition, scalarType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Subtraction, scalarType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Multiplication, scalarType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Division, scalarType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::AdditionAssignment, scalarType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::SubtractAssignment, scalarType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::MultiplicationAssignment, scalarType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::DivisionAssignment, scalarType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::LessThan, boolType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::LessThanEqual, boolType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::GreaterThan, boolType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::GraterThanEqual, boolType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Equality, boolType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Inequality, boolType, scalarType, scalarType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Assignment, scalarType, scalarType, scalarType);
            }

            return scalarType;
        }

        StructType* addStructType(const std::string& name,
                                  uint32_t size,
                                  std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            StructType* structType = create<StructType>(size);
            structType->name = name;

            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Equality, boolType, structType, structType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Inequality, boolType, structType, structType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Assignment, structType, structType, structType);

            return structType;
        }

        VectorType* addVectorType(const std::string& name,
                                  const ScalarType* componentType,
                                  uint8_t componentCount,
                                  std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            VectorType* vectorType = create<VectorType>(componentType, componentCount);
            vectorType->name = name;

            vectorTypes[std::make_pair(componentType, componentCount)] = vectorType;

            unaryOperators.emplace_back(UnaryOperatorExpression::Kind::Positive, vectorType, vectorType);
            unaryOperators.emplace_back(UnaryOperatorExpression::Kind::Negative, vectorType, vectorType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Addition, vectorType, vectorType, vectorType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Subtraction, vectorType, vectorType, vectorType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Multiplication, vectorType, vectorType, vectorType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Division, vectorType, vectorType, vectorType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::AdditionAssignment, vectorType, vectorType, vectorType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::SubtractAssignment, vectorType, vectorType, vectorType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::MultiplicationAssignment, vectorType, vectorType, vectorType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::DivisionAssignment, vectorType, vectorType, vectorType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Equality, boolType, vectorType, vectorType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Inequality, boolType, vectorType, vectorType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Assignment, vectorType, vectorType, vectorType);

            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Multiplication, vectorType, vectorType, componentType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Division, vectorType, vectorType, componentType);

            return vectorType;
        }

        MatrixType* addMatrixType(const std::string& name,
                                  const ScalarType* componentType,
                                  uint8_t rowCount,
                                  uint8_t columnCount,
                                  std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            MatrixType* matrixType = create<MatrixType>(componentType, rowCount, columnCount);
            matrixType->name = name;

            unaryOperators.emplace_back(UnaryOperatorExpression::Kind::Positive, matrixType, matrixType);
            unaryOperators.emplace_back(UnaryOperatorExpression::Kind::Negative, matrixType, matrixType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Addition, matrixType, matrixType, matrixType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Subtraction, matrixType, matrixType, matrixType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Multiplication, matrixType, matrixType, matrixType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::AdditionAssignment, matrixType, matrixType, matrixType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::SubtractAssignment, matrixType, matrixType, matrixType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::MultiplicationAssignment, matrixType, matrixType, matrixType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::DivisionAssignment, matrixType, matrixType, matrixType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Equality, boolType, matrixType, matrixType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Inequality, boolType, matrixType, matrixType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Assignment, matrixType, matrixType, matrixType);

            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Multiplication, matrixType, matrixType, componentType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Division, matrixType, matrixType, componentType);
            
            return matrixType;
        }

        FieldDeclaration* addFieldDeclaration(StructType* structType,
                                              const std::string& name,
                                              Type* type,
                                              bool isConst,
                                              std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto fieldDeclaration = create<FieldDeclaration>();
            fieldDeclaration->name = name;
            fieldDeclaration->qualifiedType.type = type;
            fieldDeclaration->qualifiedType.qualifiers = (isConst ? Qualifiers::Const : Qualifiers::None);
            declarationScopes.back().push_back(fieldDeclaration);

            structType->memberDeclarations.push_back(fieldDeclaration);

            return fieldDeclaration;
        }

        FunctionDeclaration* addBuiltinFunctionDeclaration(const std::string& name,
                                                           const Type* resultType,
                                                           const std::vector<Type*>& parameters,
                                                           std::vector<std::vector<Declaration*>>& declarationScopes)
        {
            auto functionDeclaration = create<FunctionDeclaration>();
            functionDeclaration->name = name;
            functionDeclaration->qualifiedType.type = resultType;

            for (auto parameter : parameters)
            {
                auto parameterDeclaration = create<ParameterDeclaration>();
                parameterDeclaration->qualifiedType.type = parameter;
                functionDeclaration->parameterDeclarations.push_back(parameterDeclaration);
            }

            functionDeclaration->isBuiltin = true;
            declarationScopes.back().push_back(functionDeclaration);

            return functionDeclaration;
        }

        struct UnaryOperator final
        {
            UnaryOperator(UnaryOperatorExpression::Kind initUnaryOperatorKind,
                          const Type* initResultType,
                          const Type* initParameterType) noexcept:
                unaryOperatorKind(initUnaryOperatorKind),
                resultType(initResultType),
                parameterType(initParameterType) {}

            UnaryOperatorExpression::Kind unaryOperatorKind;
            const Type* resultType;
            const Type* parameterType;
        };

        const UnaryOperator& getUnaryOperator(UnaryOperatorExpression::Kind unaryOperatorKind,
                                              const Type* parameterType) const
        {
            for (const auto& unaryOperator : unaryOperators)
                if (unaryOperator.unaryOperatorKind == unaryOperatorKind &&
                    unaryOperator.parameterType == parameterType)
                    return unaryOperator;

            throw ParseError("No unary operator defined for this type");
        }

        struct BinaryOperator final
        {
            BinaryOperator(BinaryOperatorExpression::Kind initBinaryOperatorKind,
                           const Type* initResultType,
                           const Type* initFirstParameterType,
                           const Type* initSecondParameterType) noexcept:
                binaryOperatorKind(initBinaryOperatorKind),
                resultType(initResultType),
                firstParameterType(initFirstParameterType),
                secondParameterType(initSecondParameterType) {}

            BinaryOperatorExpression::Kind binaryOperatorKind;
            const Type* resultType;
            const Type* firstParameterType;
            const Type* secondParameterType;
        };

        const BinaryOperator& getBinaryOperator(BinaryOperatorExpression::Kind binaryOperatorKind,
                                                const Type* firstParameterType,
                                                const Type* secondParameterType) const
        {
            for (const auto& binaryOperator : binaryOperators)
                if (binaryOperator.binaryOperatorKind == binaryOperatorKind &&
                    binaryOperator.firstParameterType == firstParameterType &&
                    binaryOperator.secondParameterType == secondParameterType)
                    return binaryOperator;

            throw ParseError("No binary operator defined for this type");
        }

        template <class T, class ...Args, typename std::enable_if<std::is_base_of<Type, T>::value>::type* = nullptr>
        T* create(Args... args)
        {
            T* result;
            types.push_back(std::unique_ptr<Type>(result = new T(args...)));
            return result;
        }

        template <class T, class ...Args, typename std::enable_if<std::is_base_of<Construct, T>::value>::type* = nullptr>
        T* create(Args... args)
        {
            T* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new T(args...)));
            return result;
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

        std::vector<UnaryOperator> unaryOperators;
        std::vector<BinaryOperator> binaryOperators;
    };
}

#endif // PARSER_HPP
