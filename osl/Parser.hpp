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
    enum class ErrorCode
    {
        NoError,
        UnexpectedEndOfFile,
        InvalidIndex,
        InvalidSubscript,
        NoMatchingFunction,
        AmbiguousCall,
        UnsupportedFeature,
        InvalidAttribute,
        SymbolRedefinition,
        SymbolRedeclaration,
        FunctionRedeclarationWithDifferentReturnType,
        MissingType,
        IllegalVoidType,
        ConditionNotBoolean,
        MissingInitializer,
        InvalidDeclarationReference,
        DeclarationExpected,
        StatementExpected,
        FunctionDeclarationExpected,
        VariableDeclarationExpected,
        ExpressionExpected,
        IntegerTypeExpected,
        NumberTypeExpected,
        StructTypeExpected,
        ArrayTypeExpected,
        UnexpectedToken,
        InvalidType,
        WrongTypeInReturn,
        ExpressionNotConst,
        ConflictingTypesInInitializerList,
        InvalidVectorInitialization,
        InvalidMatrixInitialization,
        EmptyVectorInitializer,
        EmptyMatrixInitializer,
        InvalidCast,
        InvalidSwizzle,
        ExpressionNotAssignable,
        IncompatibleOperands,
        NoOperator,
        InvalidInitializerType,
        NoConstructorFound,
        InvalidMember,
        UnexpectedDeclaration
    };

    class ParseError final: public std::logic_error
    {
    public:
        explicit ParseError(ErrorCode code, const std::string& str): std::logic_error(str), errorCode(code) {}
        explicit ParseError(ErrorCode code, const char* str): std::logic_error(str), errorCode(code) {}

        ErrorCode getErrorCode() const noexcept { return errorCode; }

    private:
        ErrorCode errorCode;
    };

    class ASTContext final
    {
    public:
        using TokenIterator = std::vector<Token>::const_iterator;
        using DeclarationScope = std::vector<Declaration*>;
        using DeclarationScopes = std::vector<DeclarationScope>;

        explicit ASTContext(const std::vector<Token>& tokens):
            voidType(create<Type>(Type::Kind::Void, "void")),
            boolType(addScalarType("bool", ScalarType::Kind::Boolean, false)),
            intType(addScalarType("int", ScalarType::Kind::Integer, false)),
            uintType(addScalarType("uint", ScalarType::Kind::Integer, true)),
            floatType(addScalarType("float", ScalarType::Kind::FloatingPoint, false)),
            stringType(addStructType("string"))
        {
            DeclarationScopes declarationScopes;
            declarationScopes.push_back(DeclarationScope());

            addBuiltinFunctionDeclaration("discard", voidType, {}, declarationScopes);

            addBuiltinFunctionDeclaration("abs", boolType, {&boolType}, declarationScopes);
            addBuiltinFunctionDeclaration("abs", intType, {&intType}, declarationScopes);
            addBuiltinFunctionDeclaration("abs", uintType, {&uintType}, declarationScopes);
            addBuiltinFunctionDeclaration("abs", floatType, {&floatType}, declarationScopes);

            for (std::size_t components = 2; components <= 4; ++components)
            {
                const auto& vectorType = addVectorType("float" + std::to_string(components),
                                                       floatType, components);

                const auto& matrixType = addMatrixType("float" + std::to_string(components) + 'x' + std::to_string(components),
                                                       vectorType, components);

                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Multiplication, vectorType, matrixType, vectorType);
                binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Multiplication, vectorType, vectorType, matrixType);

                addBuiltinFunctionDeclaration("abs", vectorType, {&vectorType}, declarationScopes);
                addBuiltinFunctionDeclaration("abs", matrixType, {&matrixType}, declarationScopes);
            }

            const auto& texture2DType = addStructType("Texture2D");

            //addBuiltinFunctionDeclaration("sample", float4Type, {texture2DType, float2Type}, declarationScopes);

            const auto& texture2DMSType = addStructType("Texture2DMS");

            //addBuiltinFunctionDeclaration("load", float4Type, {texture2DMSType, float2Type}, declarationScopes);

            for (auto iterator = tokens.begin(); iterator != tokens.end();)
            {
                auto& declaration = parseTopLevelDeclaration(iterator, tokens.end(), declarationScopes);
                declarations.push_back(&declaration);
            }
        }

        void dump() const
        {
            for (auto declaration : declarations)
                dumpConstruct(*declaration);
        }

        inline const std::vector<Declaration*>& getDeclarations() const noexcept
        {
            return declarations;
        }

    private:
        static bool isToken(Token::Type tokenType,
                            TokenIterator iterator,
                            TokenIterator end) noexcept
        {
            return iterator != end && iterator->type == tokenType;
        }

        template <std::size_t N>
        static bool isToken(const Token::Type (&tokenTypes)[N],
                            TokenIterator iterator, TokenIterator end) noexcept
        {
            if (iterator == end) return false;

            for (auto tokenType : tokenTypes)
                if (iterator->type == tokenType) return true;

            return false;
        }

        static const Token& getToken(TokenIterator& iterator, TokenIterator end)
        {
            if (iterator == end)
                throw ParseError(ErrorCode::UnexpectedEndOfFile, "Unexpected end of file");

            return *iterator++;
        }

        static bool skipToken(Token::Type tokenType,
                              TokenIterator& iterator, TokenIterator end) noexcept
        {
            if (iterator == end || iterator->type != tokenType) return false;
            ++iterator;
            return true;
        }

        static const Token& expectToken(Token::Type tokenType,
                                        TokenIterator& iterator, TokenIterator end)
        {
            if (iterator == end)
                throw ParseError(ErrorCode::UnexpectedEndOfFile, "Unexpected end of file");
            if (iterator->type != tokenType)
                throw ParseError(ErrorCode::UnexpectedToken, "Expected " + toString(tokenType));

            return *iterator++;
        }

        static bool isBooleanType(const Type& type) noexcept
        {
            if (type.typeKind != Type::Kind::Scalar)
                return false;

            auto& scalarType = static_cast<const ScalarType&>(type);

            return scalarType.scalarTypeKind == ScalarType::Kind::Boolean;
        }

        static bool isIntegerType(const Type& type) noexcept
        {
            if (type.typeKind != Type::Kind::Scalar)
                return false;

            auto& scalarType = static_cast<const ScalarType&>(type);

            return scalarType.scalarTypeKind == ScalarType::Kind::Boolean ||
                scalarType.scalarTypeKind == ScalarType::Kind::Integer;
        }

        static std::size_t parseIndex(TokenIterator& iterator, TokenIterator end)
        {
            std::size_t result = 0;

            if (skipToken(Token::Type::LeftParenthesis, iterator, end))
            {
                int index = std::stoi(expectToken(Token::Type::IntLiteral, iterator, end).value);
                if (index < 0)
                    throw ParseError(ErrorCode::InvalidIndex, "Index must be positive");

                result = static_cast<std::size_t>(index);

                expectToken(Token::Type::RightParenthesis, iterator, end);
            }

            return result;
        }

        static Declaration* findDeclaration(const std::string& name,
                                            const DeclarationScope& declarationScope)
        {
            for (auto declarationIterator = declarationScope.rbegin(); declarationIterator != declarationScope.rend(); ++declarationIterator)
                if ((*declarationIterator)->name == name) return *declarationIterator;

            return nullptr;
        }

        static Declaration* findDeclaration(const std::string& name,
                                            const DeclarationScopes& declarationScopes)
        {
            for (auto scopeIterator = declarationScopes.rbegin(); scopeIterator != declarationScopes.rend(); ++scopeIterator)
                for (auto declarationIterator = scopeIterator->rbegin(); declarationIterator != scopeIterator->rend(); ++declarationIterator)
                    if ((*declarationIterator)->name == name) return *declarationIterator;

            return nullptr;
        }

        const Type* findType(const std::string& name,
                             const DeclarationScopes& declarationScopes)
        {
            auto declaration = findDeclaration(name, declarationScopes);

            if (declaration && declaration->declarationKind == Declaration::Kind::Type)
                return &static_cast<TypeDeclaration*>(declaration)->type;

            for (const auto& type : types)
                if (type->name == name)
                    return type.get();

            return nullptr;
        }

        const StructType* findStructType(const std::string& name,
                                         const DeclarationScopes& declarationScopes)
        {
            auto type = findType(name, declarationScopes);

            if (type && type->typeKind == Type::Kind::Struct)
                return static_cast<const StructType*>(type);

            return nullptr;
        }

        const VectorType* findVectorType(const Type& componentType, std::size_t components)
        {
            auto vectorTypeIterator = vectorTypes.find(std::make_pair(&componentType, static_cast<std::uint8_t>(components)));

            if (vectorTypeIterator == vectorTypes.end())
                return nullptr;

            return vectorTypeIterator->second;
        }

        static FunctionDeclaration* findFunctionDeclaration(const std::string& name,
                                                            const DeclarationScopes& declarationScopes,
                                                            const std::vector<QualifiedType>& parameters)
        {
            for (auto scopeIterator = declarationScopes.rbegin(); scopeIterator != declarationScopes.rend(); ++scopeIterator)
                for (auto declarationIterator = scopeIterator->rbegin(); declarationIterator != scopeIterator->rend(); ++declarationIterator)
                    if ((*declarationIterator)->name == name)
                    {
                        if ((*declarationIterator)->declarationKind != Declaration::Kind::Callable) return nullptr;

                        auto callableDeclaration = static_cast<CallableDeclaration*>(*declarationIterator);

                        if (callableDeclaration->callableDeclarationKind != CallableDeclaration::Kind::Function) return nullptr;

                        auto functionDeclaration = static_cast<FunctionDeclaration*>(callableDeclaration);

                        if (functionDeclaration->parameterDeclarations.size() == parameters.size() &&
                            std::equal(parameters.begin(), parameters.end(),
                            functionDeclaration->parameterDeclarations.begin(),
                            [](const QualifiedType& qualifiedType,
                               const ParameterDeclaration* parameterDeclaration) {
                                return &qualifiedType.type == &parameterDeclaration->qualifiedType.type;
                            }))
                            return functionDeclaration;
                    }

            return nullptr;
        }

        static const FunctionDeclaration* resolveFunctionDeclaration(const std::string& name,
                                                                     const DeclarationScopes& declarationScopes,
                                                                     const std::vector<QualifiedType>& arguments)
        {
            std::vector<const FunctionDeclaration*> candidateFunctionDeclarations;

            for (auto scopeIterator = declarationScopes.rbegin(); scopeIterator != declarationScopes.rend(); ++scopeIterator)
                for (auto declarationIterator = scopeIterator->rbegin(); declarationIterator != scopeIterator->rend(); ++declarationIterator)
                    if ((*declarationIterator)->name == name)
                    {
                        if ((*declarationIterator)->declarationKind != Declaration::Kind::Callable) return nullptr;

                        auto callableDeclaration = static_cast<const CallableDeclaration*>(*declarationIterator);

                        if (callableDeclaration->callableDeclarationKind != CallableDeclaration::Kind::Function) return nullptr;

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
                                   bool scalar = qualifiedType.type.typeKind == Type::Kind::Scalar &&
                                       qualifiedType.type.typeKind == Type::Kind::Scalar;

                                   return (scalar || &qualifiedType.type == &parameterDeclaration->qualifiedType.type);
                               }))
                    viableFunctionDeclarations.push_back(functionDeclaration);

            if (viableFunctionDeclarations.empty())
                throw ParseError(ErrorCode::NoMatchingFunction, "No matching function to call " + name + " found");
            else if (viableFunctionDeclarations.size() == 1)
                return *viableFunctionDeclarations.begin();
            else
            {
                if (arguments.empty()) // two or more functions with zero parameters
                    throw ParseError(ErrorCode::AmbiguousCall, "Ambiguous call to " + name);

                const FunctionDeclaration* result = nullptr;

                for (auto viableFunctionDeclaration : viableFunctionDeclarations)
                    if (arguments.size() == viableFunctionDeclaration->parameterDeclarations.size())
                    {
                        bool valid = true;
                        for (std::size_t i = 0; i < arguments.size(); ++i)
                        {
                            const QualifiedType& argument = arguments[i];
                            const QualifiedType& parameter = viableFunctionDeclaration->parameterDeclarations[i]->qualifiedType;

                            if (&parameter.type != &argument.type)
                            {
                                valid = false;
                                break;
                            }
                        }

                        if (valid)
                        {
                            if (result)
                                throw ParseError(ErrorCode::AmbiguousCall, "Ambiguous call to " + name);
                            else
                                result = viableFunctionDeclaration;
                        }
                    }

                return result;
            }
        }

        static const ConstructorDeclaration* findConstructorDeclaration(const StructType& structType,
                                                                        const std::vector<QualifiedType>& parameters) noexcept
        {
            for (auto declaration : structType.memberDeclarations)
                if (declaration->declarationKind == Declaration::Kind::Callable)
                {
                    auto callableDeclaration = static_cast<const CallableDeclaration*>(declaration);

                    if (callableDeclaration->callableDeclarationKind == CallableDeclaration::Kind::Constructor)
                    {
                        auto constructorDeclaration = static_cast<const ConstructorDeclaration*>(callableDeclaration);

                        if (constructorDeclaration->parameterDeclarations.size() == parameters.size() &&
                            std::equal(parameters.begin(), parameters.end(),
                                       constructorDeclaration->parameterDeclarations.begin(),
                                       [](const QualifiedType& qualifiedType,
                                          const ParameterDeclaration* parameterDeclaration) {
                                           return &qualifiedType.type == &parameterDeclaration->qualifiedType.type; // TODO: overload resolution
                                       }))
                            return constructorDeclaration;
                    }
                }

            return nullptr;
        }

        static const Declaration* findMemberDeclaration(const StructType& structType,
                                                        const std::string& name) noexcept
        {
            for (auto memberDeclaration : structType.memberDeclarations)
                if (memberDeclaration->name == name) return memberDeclaration;

            return nullptr;
        }

        const ArrayType& getArrayType(const Type& type, std::size_t count)
        {
            QualifiedType qualifiedType{type};

            auto i = arrayTypes.find(std::make_pair(qualifiedType, count));

            if (i == arrayTypes.end())
            {
                auto& result = create<ArrayType>(qualifiedType, count);
                arrayTypes[std::make_pair(qualifiedType, count)] = &result;
                return result;
            }
            else
                return *i->second;
        }

        bool isType(TokenIterator iterator, TokenIterator end,
                    DeclarationScopes& declarationScopes)
        {
            if (iterator == end)
                throw ParseError(ErrorCode::UnexpectedEndOfFile, "Unexpected end of file");

            return iterator->type == Token::Type::Void ||
                iterator->type == Token::Type::Bool ||
                iterator->type == Token::Type::Int ||
                iterator->type == Token::Type::Float ||
                iterator->type == Token::Type::Double ||
                (iterator->type == Token::Type::Identifier &&
                 findType(iterator->value, declarationScopes));
        }

        const Type& parseType(TokenIterator& iterator, TokenIterator end,
                              DeclarationScopes& declarationScopes)
        {
            auto& token = getToken(iterator, end);

            const Type* result;

            switch (token.type)
            {
                case Token::Type::Void: result = &voidType; break;
                case Token::Type::Bool: result = &boolType; break;
                case Token::Type::Int: result = &intType; break;
                case Token::Type::Float: result = &floatType; break;
                case Token::Type::Double:
                    throw ParseError(ErrorCode::UnsupportedFeature, "Double precision floating point numbers are not supported");
                case Token::Type::Identifier:
                {
                    if (!(result = findType(token.value, declarationScopes)))
                        throw ParseError(ErrorCode::InvalidType, "Invalid type \"" + token.value + "\"");
                    break;
                }
                default: throw ParseError(ErrorCode::DeclarationExpected, "Expected a type name");
            }

            while (skipToken(Token::Type::LeftBracket, iterator, end))
            {
                const int size = std::stoi(expectToken(Token::Type::IntLiteral, iterator, end).value);

                if (size <= 0)
                    throw ParseError(ErrorCode::InvalidIndex, "Array size must be positive");

                result = &getArrayType(*result, static_cast<std::size_t>(size));

                expectToken(Token::Type::RightBracket, iterator, end);
            }

            return *result;
        }

        Attribute& parseAttribute(TokenIterator& iterator, TokenIterator end)
        {
            if (iterator == end)
                throw ParseError(ErrorCode::UnexpectedEndOfFile, "Unexpected end of file");

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
                throw ParseError(ErrorCode::InvalidAttribute, "Invalid attribute");
        }

        static bool isDeclaration(TokenIterator iterator, TokenIterator end)
        {
            return isToken({Token::Type::Function,
                Token::Type::Fragment,
                Token::Type::Vertex,
                Token::Type::Const,
                Token::Type::Extern,
                Token::Type::Var}, iterator, end);
        }

        Declaration& parseTopLevelDeclaration(TokenIterator& iterator, TokenIterator end,
                                              DeclarationScopes& declarationScopes)
        {
            auto& declaration = parseDeclaration(iterator, end, declarationScopes);

            if (declaration.declarationKind == Declaration::Kind::Callable)
            {
                auto& callableDeclaration = static_cast<const CallableDeclaration&>(declaration);

                // semicolon is not needed after a function definition
                if (!callableDeclaration.body)
                    expectToken(Token::Type::Semicolon, iterator, end);
            }
            else if (declaration.declarationKind == Declaration::Kind::Type)
            {
                // semicolon is not needed after a struct definition
            }
            else
                expectToken(Token::Type::Semicolon, iterator, end);

            return declaration;
        }

        Declaration& parseDeclaration(TokenIterator& iterator, TokenIterator end,
                                      DeclarationScopes& declarationScopes)
        {
            if (isToken(Token::Type::Struct, iterator, end))
                return parseStructTypeDeclaration(iterator, end, declarationScopes);
            else if (isToken({Token::Type::Function, Token::Type::Fragment, Token::Type::Vertex}, iterator, end))
                return parseFunctionDeclaration(iterator, end, declarationScopes);
            else if (isToken({Token::Type::Const, Token::Type::Extern, Token::Type::Var}, iterator, end))
                return parseVariableDeclaration(iterator, end, declarationScopes);
            else
                throw ParseError(ErrorCode::DeclarationExpected, "Expected a declaration");
        }

        FunctionDeclaration& parseFunctionDeclaration(TokenIterator& iterator, TokenIterator end,
                                                      DeclarationScopes& declarationScopes)
        {
            FunctionDeclaration::Qualifier qualifier = FunctionDeclaration::Qualifier::None;

            if (skipToken(Token::Type::Function, iterator, end))
                qualifier = FunctionDeclaration::Qualifier::None;
            else if (skipToken(Token::Type::Fragment, iterator, end))
                qualifier = FunctionDeclaration::Qualifier::Fragment;
            else if (skipToken(Token::Type::Vertex, iterator, end))
                qualifier = FunctionDeclaration::Qualifier::Vertex;
            else
                throw ParseError(ErrorCode::FunctionDeclarationExpected, "Expected a function declaration");

            const auto& name = expectToken(Token::Type::Identifier, iterator, end).value;

            auto previousDeclarationInScope = findDeclaration(name, declarationScopes.back());
            if (previousDeclarationInScope &&
                previousDeclarationInScope->declarationKind != Declaration::Kind::Callable)
                throw ParseError(ErrorCode::SymbolRedeclaration, "Redeclaration of " + name);

            expectToken(Token::Type::LeftParenthesis, iterator, end);

            std::vector<QualifiedType> parameterTypes;
            std::vector<ParameterDeclaration*> parameterDeclarations;
            std::set<std::string> parameterNames;

            if (!isToken(Token::Type::RightParenthesis, iterator, end))
            {
                for (;;)
                {
                    auto& parameterDeclaration = parseParameterDeclaration(iterator, end, declarationScopes);

                    if (!parameterNames.insert(parameterDeclaration.name).second)
                        throw ParseError(ErrorCode::SymbolRedefinition, "Redefinition of parameter " + parameterDeclaration.name);

                    parameterDeclarations.push_back(&parameterDeclaration);
                    parameterTypes.push_back(parameterDeclaration.qualifiedType);

                    if (!skipToken(Token::Type::Comma, iterator, end))
                        break;
                }
            }

            expectToken(Token::Type::RightParenthesis, iterator, end);

            auto previousDeclaration = findFunctionDeclaration(name, declarationScopes, parameterTypes);

            const Type& type = skipToken(Token::Type::Colon, iterator, end) ?
                parseType(iterator, end, declarationScopes) :
                voidType;

            if (previousDeclaration &&
                &previousDeclaration->resultType.type != &type)
                throw ParseError(ErrorCode::FunctionRedeclarationWithDifferentReturnType, "Redeclaring function with different return type");

            std::vector<const Attribute*> attributes;
            if (skipToken(Token::Type::Arrow, iterator, end))
                while (isToken(Token::Type::Identifier, iterator, end))
                    attributes.push_back(&parseAttribute(iterator, end));

            auto& result = create<FunctionDeclaration>(name, QualifiedType{type}, StorageClass::Auto,
                                                       std::move(attributes), std::move(parameterDeclarations),
                                                       qualifier);

            if (previousDeclaration)
            {
                result.previousDeclaration = previousDeclaration;
                result.firstDeclaration = previousDeclaration->firstDeclaration;
                result.definition = previousDeclaration->definition;
            }
            else
                result.firstDeclaration = &result;

            declarationScopes.back().push_back(&result);

            if (isToken(Token::Type::LeftBrace, iterator, end))
            {
                // check if only one definition exists
                if (result.definition)
                    throw ParseError(ErrorCode::SymbolRedefinition, "Redefinition of " + result.name);

                declarationScopes.push_back(DeclarationScope()); // add scope for parameters

                for (auto parameterDeclaration : result.parameterDeclarations)
                    declarationScopes.back().push_back(parameterDeclaration);

                std::vector<ReturnStatement*> returnStatements;
                // parse body
                auto& body = parseCompoundStatement(iterator, end, declarationScopes, returnStatements);
                result.body = &body;

                if (!returnStatements.empty())
                    for (const auto returnStatement : returnStatements)
                    {
                        const auto& returnType = returnStatement->result ? returnStatement->result->qualifiedType.type : voidType;
                        if (&result.resultType.type != &returnType)
                            throw ParseError(ErrorCode::WrongTypeInReturn, "Wrong type in return statement");
                    }

                // set the definition pointer of all previous declarations and check the result type
                auto declaration = &result;
                while (declaration)
                {
                    declaration->definition = &result;
                    if (&declaration->resultType.type != &result.resultType.type)
                        throw ParseError(ErrorCode::SymbolRedeclaration, "Redeclaring function with a different return type");
                    declaration = static_cast<FunctionDeclaration*>(declaration->previousDeclaration);
                }

                declarationScopes.pop_back();
            }

            return result;
        }

        VariableDeclaration& parseVariableDeclaration(TokenIterator& iterator, TokenIterator end,
                                                      DeclarationScopes& declarationScopes)
        {
            Type::Qualifiers qualifiers = Type::Qualifiers::None;
            StorageClass storageClass = StorageClass::Auto;

            if (skipToken(Token::Type::Const, iterator, end))
                qualifiers = Type::Qualifiers::Const;
            else if (skipToken(Token::Type::Extern, iterator, end))
                storageClass = StorageClass::Extern;
            else if (skipToken(Token::Type::Var, iterator, end))
                qualifiers = Type::Qualifiers::None;
            else
                throw ParseError(ErrorCode::VariableDeclarationExpected, "Expected a variable declaration");

            const auto& name = expectToken(Token::Type::Identifier, iterator, end).value;

            auto previousDeclarationInScope = findDeclaration(name, declarationScopes.back());
            if (previousDeclarationInScope)
                throw ParseError(ErrorCode::SymbolRedeclaration, "Redeclaration of " + name);

            const Type* type = nullptr;
            if (skipToken(Token::Type::Colon, iterator, end))
            {
                type = &parseType(iterator, end, declarationScopes);

                if (type->typeKind == Type::Kind::Void)
                    throw ParseError(ErrorCode::IllegalVoidType, "Variable can not have the type \"void\"");
            }

            const Expression* initialization;

            if (skipToken(Token::Type::Assignment, iterator, end))
            {
                initialization = &parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);

                if (initialization->qualifiedType.type.typeKind == Type::Kind::Void)
                    throw ParseError(ErrorCode::IllegalVoidType, "Initialization with the type \"void\"");

                if (!type)
                    type = &initialization->qualifiedType.type;
                else if (type != &initialization->qualifiedType.type)
                    throw ParseError(ErrorCode::InvalidInitializerType, "Initializer type does not match the variable type");
            }

            if (!type)
                throw ParseError(ErrorCode::MissingType, "Missing type for the variable");

            auto& result = create<VariableDeclaration>(name, QualifiedType{*type, qualifiers}, storageClass, initialization);
            declarationScopes.back().push_back(&result);
            return result;
        }

        TypeDeclaration& parseStructTypeDeclaration(TokenIterator& iterator, TokenIterator end,
                                                    DeclarationScopes& declarationScopes)
        {
            expectToken(Token::Type::Struct, iterator, end);

            const auto& name = expectToken(Token::Type::Identifier, iterator, end).value;

            // TODO: check if different kind of symbol with the same name does not exist
            auto previousDeclaration = findDeclaration(name, declarationScopes);

            Declaration* firstDeclaration = nullptr;
            Declaration* definition = nullptr;

            if (previousDeclaration)
            {
                if (previousDeclaration->declarationKind != Declaration::Kind::Type)
                    throw ParseError(ErrorCode::SymbolRedeclaration, "Redeclaration of " + name);

                auto typeDeclaration = static_cast<TypeDeclaration*>(previousDeclaration);

                if (typeDeclaration->type.typeKind != Type::Kind::Struct)
                    throw ParseError(ErrorCode::SymbolRedeclaration, "Redeclaration of " + name);

                firstDeclaration = typeDeclaration->firstDeclaration;
                definition = typeDeclaration->definition;
            }

            expectToken(Token::Type::LeftBrace, iterator, end);

            std::vector<const Declaration*> memberDeclarations;
            std::set<std::string> memberNames;

            for (;;)
                if (skipToken(Token::Type::RightBrace, iterator, end))
                    break;
                else
                {
                    auto& memberDeclaration = parseMemberDeclaration(iterator, end, declarationScopes);

                    if (!memberNames.insert(memberDeclaration.name).second)
                        throw ParseError(ErrorCode::SymbolRedefinition, "Redefinition of member " + memberDeclaration.name);

                    expectToken(Token::Type::Semicolon, iterator, end);

                    memberDeclarations.push_back(&memberDeclaration);
                }

            auto& structType = create<StructType>(name, std::move(memberDeclarations));

            auto& result = create<TypeDeclaration>(name, structType);
            result.firstDeclaration = firstDeclaration ? firstDeclaration : &result;
            result.previousDeclaration = previousDeclaration;
            result.definition = definition;

            // set the definition pointer of all previous declarations
            Declaration* declaration = &result;
            while (declaration)
            {
                declaration->definition = &result;
                declaration = declaration->previousDeclaration;
            }

            declarationScopes.back().push_back(&result);

            return result;
        }

        Declaration& parseMemberDeclaration(TokenIterator& iterator, TokenIterator end,
                                            DeclarationScopes& declarationScopes)
        {
            expectToken(Token::Type::Var, iterator, end);

            const auto& name = expectToken(Token::Type::Identifier, iterator, end).value;

            expectToken(Token::Type::Colon, iterator, end);

            const auto& type = parseType(iterator, end, declarationScopes);

            if (type.typeKind == Type::Kind::Void)
                throw ParseError(ErrorCode::IllegalVoidType, "Member cannot have the type \"void\"");

            std::vector<const Attribute*> attributes;
            if (skipToken(Token::Type::Arrow, iterator, end))
                while (isToken(Token::Type::Identifier, iterator, end))
                    attributes.push_back(&parseAttribute(iterator, end));

            return create<FieldDeclaration>(name, QualifiedType{type}, std::move(attributes));
        }

        ParameterDeclaration& parseParameterDeclaration(TokenIterator& iterator, TokenIterator end,
                                                        DeclarationScopes& declarationScopes)
        {
            InputModifier inputModifier = InputModifier::In;

            switch (iterator->type)
            {
                case Token::Type::In:
                    inputModifier = InputModifier::In;
                    ++iterator;
                    break;
                case Token::Type::Out:
                    inputModifier = InputModifier::Out;
                    ++iterator;
                    break;
                case Token::Type::Inout:
                    inputModifier = InputModifier::Inout;
                    ++iterator;
                    break;
                default:
                    break;
            }

            const auto& name = expectToken(Token::Type::Identifier, iterator, end).value;

            expectToken(Token::Type::Colon, iterator, end);

            const auto& type = parseType(iterator, end, declarationScopes);

            if (type.typeKind == Type::Kind::Void)
                throw ParseError(ErrorCode::IllegalVoidType, "Parameter cannot have the type \"void\"");

            std::vector<const Attribute*> attributes;
            if (skipToken(Token::Type::Arrow, iterator, end))
                while (isToken(Token::Type::Identifier, iterator, end))
                    attributes.push_back(&parseAttribute(iterator, end));

            return create<ParameterDeclaration>(name, QualifiedType{type}, inputModifier, std::move(attributes));
        }

        Statement& parseStatement(TokenIterator& iterator, TokenIterator end,
                                  DeclarationScopes& declarationScopes,
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
                auto& result = isToken(Token::Type::Semicolon, iterator, end) ?
                    create<ReturnStatement>() :
                    create<ReturnStatement>(&parseExpression(iterator, end, declarationScopes));

                returnStatements.push_back(&result);

                expectToken(Token::Type::Semicolon, iterator, end);

                return result;
            }
            else if (skipToken(Token::Type::Semicolon, iterator, end))
                return create<Statement>(Statement::Kind::Empty);
            else if (isToken(Token::Type::Asm, iterator, end))
                throw ParseError(ErrorCode::UnsupportedFeature, "asm statements are not supported");
            else if (isToken(Token::Type::Goto, iterator, end))
                throw ParseError(ErrorCode::UnsupportedFeature, "goto statements are not supported");
            else if (isToken({Token::Type::Try, Token::Type::Catch, Token::Type::Throw}, iterator, end))
                throw ParseError(ErrorCode::UnsupportedFeature, "Exceptions are not supported");
            else if (iterator == end || isToken(Token::Type::RightBrace, iterator, end))
                throw ParseError(ErrorCode::StatementExpected, "Expected a statement");
            else if (isDeclaration(iterator, end))
            {
                auto& declaration = parseDeclaration(iterator, end, declarationScopes);

                if (declaration.declarationKind == Declaration::Kind::Variable)
                    expectToken(Token::Type::Semicolon, iterator, end);
                else if (declaration.declarationKind != Declaration::Kind::Type)
                    throw ParseError(ErrorCode::UnexpectedDeclaration, "Unexpected declaration");

                return create<DeclarationStatement>(declaration);
            }
            else
            {
                auto& expression = parseExpression(iterator, end, declarationScopes);
                expectToken(Token::Type::Semicolon, iterator, end);
                return create<ExpressionStatement>(expression);
            }
        }

        CompoundStatement& parseCompoundStatement(TokenIterator& iterator, TokenIterator end,
                                                  DeclarationScopes& declarationScopes,
                                                  std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::LeftBrace, iterator, end);

            declarationScopes.push_back(DeclarationScope());

            std::vector<const Statement*> statements;

            for (;;)
                if (skipToken(Token::Type::RightBrace, iterator, end))
                    break;
                else
                    statements.push_back(&parseStatement(iterator, end, declarationScopes, returnStatements));

            declarationScopes.pop_back();

            return create<CompoundStatement>(std::move(statements));
        }

        IfStatement& parseIfStatement(TokenIterator& iterator, TokenIterator end,
                                      DeclarationScopes& declarationScopes,
                                      std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::If, iterator, end);
            expectToken(Token::Type::LeftParenthesis, iterator, end);

            const Construct* condition = nullptr;

            if (isDeclaration(iterator, end))
            {
                auto& declaration = parseVariableDeclaration(iterator, end, declarationScopes);

                if (!isBooleanType(declaration.qualifiedType.type))
                    throw ParseError(ErrorCode::ConditionNotBoolean, "Condition is not of the type \"bool\"");

                condition = &declaration;
            }
            else
            {
                auto& expression = parseExpression(iterator, end, declarationScopes);

                if (!isBooleanType(expression.qualifiedType.type))
                    throw ParseError(ErrorCode::ConditionNotBoolean, "Condition is not of the type \"bool\"");

                condition = &expression;
            }

            expectToken(Token::Type::RightParenthesis, iterator, end);

            auto& body = parseStatement(iterator, end, declarationScopes, returnStatements);

            const Statement* elseBody = (skipToken(Token::Type::Else, iterator, end)) ?
                &parseStatement(iterator, end, declarationScopes, returnStatements) : nullptr;

            return create<IfStatement>(*condition, body, elseBody);
        }

        ForStatement& parseForStatement(TokenIterator& iterator, TokenIterator end,
                                        DeclarationScopes& declarationScopes,
                                        std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::For, iterator, end);
            expectToken(Token::Type::LeftParenthesis, iterator, end);

            const Construct* initialization = nullptr;

            if (isDeclaration(iterator, end))
            {
                auto& declaration = parseVariableDeclaration(iterator, end, declarationScopes);
                initialization = &declaration;

                expectToken(Token::Type::Semicolon, iterator, end);
            }
            else if (skipToken(Token::Type::Semicolon, iterator, end))
                initialization = nullptr;
            else
            {
                initialization = &parseExpression(iterator, end, declarationScopes);

                expectToken(Token::Type::Semicolon, iterator, end);
            }

            const Construct* condition = nullptr;

            if (isDeclaration(iterator, end))
            {
                auto& declaration = parseVariableDeclaration(iterator, end, declarationScopes);

                if (!declaration.initialization)
                    throw ParseError(ErrorCode::MissingInitializer, "Condition must have an initializer");

                if (!isBooleanType(declaration.qualifiedType.type))
                    throw ParseError(ErrorCode::ConditionNotBoolean, "Condition is not of the type \"bool\"");

                condition = &declaration;

                expectToken(Token::Type::Semicolon, iterator, end);
            }
            else if (skipToken(Token::Type::Semicolon, iterator, end))
                condition = nullptr;
            else
            {
                auto& expression = parseExpression(iterator, end, declarationScopes);

                if (!isBooleanType(expression.qualifiedType.type))
                    throw ParseError(ErrorCode::ConditionNotBoolean, "Condition is not of the type \"bool\"");

                condition = &expression;

                expectToken(Token::Type::Semicolon, iterator, end);
            }

            const Expression* increment = nullptr;

            if (skipToken(Token::Type::RightParenthesis, iterator, end))
                increment = nullptr;
            else
            {
                increment = &parseExpression(iterator, end, declarationScopes);
                expectToken(Token::Type::RightParenthesis, iterator, end);
            }

            auto& body = parseStatement(iterator, end, declarationScopes, returnStatements);

            return create<ForStatement>(initialization, condition, increment, body);
        }

        SwitchStatement& parseSwitchStatement(TokenIterator& iterator, TokenIterator end,
                                              DeclarationScopes& declarationScopes,
                                              std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::Switch, iterator, end);
            expectToken(Token::Type::LeftParenthesis, iterator, end);

            const Construct* condition = nullptr;

            if (isDeclaration(iterator, end))
            {
                auto& declaration = parseVariableDeclaration(iterator, end, declarationScopes);

                if (!isIntegerType(declaration.qualifiedType.type))
                    throw ParseError(ErrorCode::IntegerTypeExpected, "Statement requires expression of integer type");

                condition = &declaration;
            }
            else
            {
                auto& expression = parseExpression(iterator, end, declarationScopes);

                if (!isIntegerType(expression.qualifiedType.type))
                    throw ParseError(ErrorCode::IntegerTypeExpected, "Statement requires expression of integer type");

                condition = &expression;
            }

            expectToken(Token::Type::RightParenthesis, iterator, end);

            auto& body = parseStatement(iterator, end, declarationScopes, returnStatements);

            return create<SwitchStatement>(*condition, body);
        }

        CaseStatement& parseCaseStatement(TokenIterator& iterator, TokenIterator end,
                                          DeclarationScopes& declarationScopes,
                                          std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::Case, iterator, end);

            auto& condition = parseExpression(iterator, end, declarationScopes);

            if (!isIntegerType(condition.qualifiedType.type))
                throw ParseError(ErrorCode::IntegerTypeExpected, "Statement requires expression of integer type");

            if ((condition.qualifiedType.qualifiers & Type::Qualifiers::Const) != Type::Qualifiers::Const)
                throw ParseError(ErrorCode::ExpressionNotConst, "Expression must be constant");

            expectToken(Token::Type::Colon, iterator, end);

            auto& body = parseStatement(iterator, end, declarationScopes, returnStatements);

            return create<CaseStatement>(condition, body);
        }

        DefaultStatement& parseDefaultStatement(TokenIterator& iterator, TokenIterator end,
                                                DeclarationScopes& declarationScopes,
                                                std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::Default, iterator, end);
            expectToken(Token::Type::Colon, iterator, end);

            return create<DefaultStatement>(parseStatement(iterator, end, declarationScopes, returnStatements));
        }

        WhileStatement& parseWhileStatement(TokenIterator& iterator, TokenIterator end,
                                            DeclarationScopes& declarationScopes,
                                            std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::While, iterator, end);
            expectToken(Token::Type::LeftParenthesis, iterator, end);

            const Construct* condition = nullptr;

            if (isDeclaration(iterator, end))
            {
                auto& declaration = parseVariableDeclaration(iterator, end, declarationScopes);

                if (!isBooleanType(declaration.qualifiedType.type))
                    throw ParseError(ErrorCode::ConditionNotBoolean, "Condition is not of the type \"bool\"");

                condition = &declaration;
            }
            else
            {
                auto& expression = parseExpression(iterator, end, declarationScopes);

                if (!isBooleanType(expression.qualifiedType.type))
                    throw ParseError(ErrorCode::ConditionNotBoolean, "Condition is not of the type \"bool\"");

                condition = &expression;
            }

            expectToken(Token::Type::RightParenthesis, iterator, end);

            auto& body = parseStatement(iterator, end, declarationScopes, returnStatements);

            return create<WhileStatement>(*condition, body);
        }

        DoStatement& parseDoStatement(TokenIterator& iterator, TokenIterator end,
                                      DeclarationScopes& declarationScopes,
                                      std::vector<ReturnStatement*>& returnStatements)
        {
            expectToken(Token::Type::Do, iterator, end);

            auto& body = parseStatement(iterator, end, declarationScopes, returnStatements);

            expectToken(Token::Type::While, iterator, end);
            expectToken(Token::Type::LeftParenthesis, iterator, end);

            auto& condition = parseExpression(iterator, end, declarationScopes);

            if (!isBooleanType(condition.qualifiedType.type))
                throw ParseError(ErrorCode::ConditionNotBoolean, "Condition is not of the type \"bool\"");

            expectToken(Token::Type::RightParenthesis, iterator, end);
            expectToken(Token::Type::Semicolon, iterator, end);

            return create<DoStatement>(condition, body);
        }

        Expression& parsePrimaryExpression(TokenIterator& iterator, TokenIterator end,
                                           DeclarationScopes& declarationScopes)
        {
            if (isToken(Token::Type::IntLiteral, iterator, end))
            {
                auto value = strtoll(iterator->value.c_str(), nullptr, 0);
                ++iterator;

                return create<IntegerLiteralExpression>(intType, value);
            }
            else if (isToken(Token::Type::FloatLiteral, iterator, end))
            {
                auto value = strtod(iterator->value.c_str(), nullptr);
                ++iterator;

                return create<FloatingPointLiteralExpression>(floatType, value);
            }
            else if (isToken(Token::Type::DoubleLiteral, iterator, end))
                throw ParseError(ErrorCode::UnsupportedFeature, "Double precision floating point numbers are not supported");
            else if (isToken(Token::Type::StringLiteral, iterator, end))
            {
                auto& value = iterator->value;
                ++iterator;

                return create<StringLiteralExpression>(stringType, value);
            }
            else if (isToken({Token::Type::True, Token::Type::False}, iterator, end))
            {
                auto value = iterator->type == Token::Type::True;
                ++iterator;

                return create<BooleanLiteralExpression>(boolType, value);
            }
            else if (isToken({Token::Type::Bool, Token::Type::Int, Token::Type::Float, Token::Type::Double}, iterator, end))
            {
                const Type* type = nullptr;

                if (skipToken(Token::Type::Bool, iterator, end)) type = &boolType;
                else if(skipToken(Token::Type::Int, iterator, end)) type = &intType;
                else if(skipToken(Token::Type::Float, iterator, end)) type = &floatType;
                else if(skipToken(Token::Type::Double, iterator, end))
                    throw ParseError(ErrorCode::UnsupportedFeature, "Double precision floating point numbers are not supported");

                expectToken(Token::Type::LeftParenthesis, iterator, end);

                auto& expression = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);
                expectToken(Token::Type::RightParenthesis, iterator, end);
                return create<CastExpression>(CastExpression::Kind::Functional, *type, expression);
            }
            else if (skipToken(Token::Type::LeftBrace, iterator, end))
            {
                std::vector<const Expression*> expressions;
                const Type* type = nullptr;

                for (;;)
                {
                    auto& expression = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);

                    if (!type)
                        type = &expression.qualifiedType.type;
                    else if (type != &expression.qualifiedType.type)
                        throw ParseError(ErrorCode::ConflictingTypesInInitializerList, "Conflicting types in initializer list");

                    expressions.push_back(&expression);

                    if (!skipToken(Token::Type::Comma, iterator, end))
                        break;
                }

                expectToken(Token::Type::RightBrace, iterator, end);

                auto& arrayType = getArrayType(*type, expressions.size());
                return create<InitializerListExpression>(arrayType, std::move(expressions));
            }
            else if (isToken(Token::Type::Identifier, iterator, end))
            {
                std::string name = iterator->value;
                ++iterator;

                if (skipToken(Token::Type::LeftParenthesis, iterator, end))
                {
                    if (auto type = findType(name, declarationScopes))
                    {
                        std::vector<const Expression*> parameters;
                        std::vector<QualifiedType> parameterTypes;

                        if (!skipToken(Token::Type::RightParenthesis, iterator, end)) // has arguments
                        {
                            for (;;)
                            {
                                auto& parameter = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);

                                parameters.push_back(&parameter);
                                parameterTypes.push_back(parameter.qualifiedType);

                                if (!skipToken(Token::Type::Comma, iterator, end))
                                    break;
                            }

                            expectToken(Token::Type::RightParenthesis, iterator, end);
                        }

                        switch (type->typeKind)
                        {
                            case Type::Kind::Struct:
                            {
                                auto structType = static_cast<const StructType*>(type);

                                auto constructorDeclaration = findConstructorDeclaration(*structType, parameterTypes);

                                if (!constructorDeclaration)
                                    throw ParseError(ErrorCode::NoConstructorFound, "No matching constructor found");

                                return create<TemporaryObjectExpression>(*structType, *constructorDeclaration, std::move(parameters));
                            }
                            case Type::Kind::Vector:
                            {
                                if (parameters.empty())
                                    throw ParseError(ErrorCode::EmptyVectorInitializer, "Vector cannot not have an empty initializer");

                                auto vectorType = static_cast<const VectorType*>(type);

                                std::size_t componentCount = 0;

                                for (auto parameter : parameters)
                                {
                                    auto& parameterType = parameter->qualifiedType.type;
                                    if (parameterType.typeKind == Type::Kind::Scalar)
                                    {
                                        if (&parameterType != &vectorType->componentType)
                                            throw ParseError(ErrorCode::InvalidVectorInitialization, "Invalid vector initialization");

                                        ++componentCount;
                                    }
                                    else if (parameterType.typeKind == Type::Kind::Vector)
                                    {
                                        auto& vectorParameterType = static_cast<const VectorType&>(parameterType);
                                        if (&vectorParameterType.componentType != &vectorType->componentType)
                                            throw ParseError(ErrorCode::InvalidVectorInitialization, "Invalid vector initialization");

                                        componentCount += vectorParameterType.componentCount;
                                    }
                                }

                                if (componentCount != vectorType->componentCount)
                                    throw ParseError(ErrorCode::InvalidVectorInitialization, "Invalid vector initialization");

                                return create<VectorInitializeExpression>(*vectorType, std::move(parameters));
                            }
                            case Type::Kind::Matrix:
                            {
                                if (parameters.empty())
                                    throw ParseError(ErrorCode::EmptyMatrixInitializer, "Matrix cannot not have an empty initializer");

                                auto matrixType = static_cast<const MatrixType*>(type);

                                std::size_t rowCount = 0;

                                for (auto parameter : parameters)
                                {
                                    auto& parameterType = parameter->qualifiedType.type;
                                    if (parameterType.typeKind == Type::Kind::Vector)
                                    {
                                        auto& vectorParameterType = static_cast<const VectorType&>(parameterType);

                                        if (&vectorParameterType != &matrixType->rowType)
                                            throw ParseError(ErrorCode::InvalidMatrixInitialization, "Invalid matrix initialization");

                                        ++rowCount;
                                    }
                                    else if (parameterType.typeKind == Type::Kind::Matrix)
                                    {
                                        auto& matrixParameterType = static_cast<const MatrixType&>(parameterType);

                                        if (&matrixParameterType.rowType != &matrixType->rowType ||
                                            matrixParameterType.rowCount != matrixType->rowCount)
                                            throw ParseError(ErrorCode::InvalidMatrixInitialization, "Invalid matrix initialization");

                                        rowCount += matrixParameterType.rowCount;
                                    }

                                }

                                if (rowCount != matrixType->rowCount)
                                    throw ParseError(ErrorCode::InvalidMatrixInitialization, "Invalid matrix initialization");

                                return create<MatrixInitializeExpression>(*matrixType, std::move(parameters));
                            }
                            default:
                                throw ParseError(ErrorCode::StructTypeExpected, "Expected a struct type");
                        }
                    }
                    else
                    {
                        std::vector<QualifiedType> argumentTypes;
                        std::vector<const Expression*> arguments;

                        if (!skipToken(Token::Type::RightParenthesis, iterator, end)) // has arguments
                        {
                            for (;;)
                            {
                                auto& argument = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);

                                arguments.push_back(&argument);
                                argumentTypes.push_back(argument.qualifiedType);

                                if (!skipToken(Token::Type::Comma, iterator, end))
                                    break;
                            }

                            expectToken(Token::Type::RightParenthesis, iterator, end);
                        }

                        auto functionDeclaration = resolveFunctionDeclaration(name, declarationScopes, argumentTypes);
                        if (!functionDeclaration)
                            throw ParseError(ErrorCode::InvalidDeclarationReference, "Invalid function reference \"" + name + "\"");

                        auto& declRefExpression = create<DeclarationReferenceExpression>(functionDeclaration->resultType,
                                                                                         *functionDeclaration,
                                                                                         Expression::Category::Lvalue);

                        return create<CallExpression>(functionDeclaration->resultType, Expression::Category::Rvalue, declRefExpression, std::move(arguments));
                    }
                }
                else
                {
                    auto declaration = findDeclaration(name, declarationScopes);
                    if (!declaration)
                        throw ParseError(ErrorCode::InvalidDeclarationReference, "Invalid declaration reference \"" + name + "\"");

                    if (declaration->declarationKind != Declaration::Kind::Variable)
                        throw ParseError(ErrorCode::VariableDeclarationExpected, "Expected a variable declaration");

                    auto variableDeclaration = static_cast<VariableDeclaration*>(declaration);

                    return create<DeclarationReferenceExpression>(variableDeclaration->qualifiedType, *declaration, Expression::Category::Lvalue);
                }
            }
            else if (skipToken(Token::Type::LeftParenthesis, iterator, end))
            {
                if (isType(iterator, end, declarationScopes))
                {
                    const auto& type = parseType(iterator, end, declarationScopes);

                    expectToken(Token::Type::RightParenthesis, iterator, end);

                    auto& expression = parseExpression(iterator, end, declarationScopes);
                    return create<CastExpression>(CastExpression::Kind::CStyle, type, expression);
                }
                else
                {
                    auto& expression = parseExpression(iterator, end, declarationScopes);
                    expectToken(Token::Type::RightParenthesis, iterator, end);
                    return create<ParenExpression>(expression);
                }
            }
            else if (isToken(Token::Type::StaticCast, iterator, end))
            {
                CastExpression::Kind castKind;

                switch (iterator->type)
                {
                    case Token::Type::StaticCast: castKind = CastExpression::Kind::Static; break;
                    default: throw ParseError(ErrorCode::InvalidCast, "Invalid cast");
                }

                ++iterator;

                expectToken(Token::Type::LessThan, iterator, end);

                const auto& type = parseType(iterator, end, declarationScopes);

                expectToken(Token::Type::GreaterThan, iterator, end);
                expectToken(Token::Type::LeftParenthesis, iterator, end);

                auto& expression = parseExpression(iterator, end, declarationScopes);
                expectToken(Token::Type::RightParenthesis, iterator, end);
                return create<CastExpression>(castKind, type, expression);
            }
            else if (isToken(Token::Type::This, iterator, end))
                throw ParseError(ErrorCode::UnsupportedFeature, "Expression \"this\" is not supported");
            else
                throw ParseError(ErrorCode::ExpressionExpected, "Expected an expression");
        }

        Expression& parsePostfixExpression(TokenIterator& iterator, TokenIterator end,
                                           DeclarationScopes& declarationScopes)
        {
            auto result = &parsePrimaryExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::Increment, Token::Type::Decrement}, iterator, end))
            {
                if (result->category != Expression::Category::Lvalue)
                    throw ParseError(ErrorCode::ExpressionNotAssignable, "Expression is not assignable");

                if ((result->qualifiedType.qualifiers & Type::Qualifiers::Const) == Type::Qualifiers::Const)
                    throw ParseError(ErrorCode::ExpressionNotAssignable, "Cannot assign to const variable");

                if (result->qualifiedType.type.typeKind != Type::Kind::Scalar)
                    throw ParseError(ErrorCode::NumberTypeExpected, "Parameter of the postfix operator must be a number");

                const auto operatorKind = (iterator->type == Token::Type::Increment) ?
                    UnaryOperatorExpression::Kind::PostfixIncrement :
                    UnaryOperatorExpression::Kind::PostfixDecrement;

                ++iterator;

                const auto& unaryOperator = getUnaryOperator(operatorKind,
                                                             result->qualifiedType.type);

                result = &create<UnaryOperatorExpression>(operatorKind, unaryOperator.resultType, Expression::Category::Lvalue, *result);
            }

            return *result;
        }

        Expression& parseSubscriptExpression(TokenIterator& iterator, TokenIterator end,
                                             DeclarationScopes& declarationScopes)
        {
            auto result = &parsePostfixExpression(iterator, end, declarationScopes);

            while (skipToken(Token::Type::LeftBracket, iterator, end))
            {
                if (result->qualifiedType.type.typeKind == Type::Kind::Array)
                {
                    auto& subscript = parseExpression(iterator, end, declarationScopes);
                    if (!isIntegerType(subscript.qualifiedType.type))
                        throw ParseError(ErrorCode::InvalidSubscript, "Subscript is not an integer");

                    expectToken(Token::Type::RightBracket, iterator, end);

                    auto& arrayType = static_cast<const ArrayType&>(result->qualifiedType.type);
                    result = &create<ArraySubscriptExpression>(arrayType.elementType, *result, subscript);
                }
                else if (result->qualifiedType.type.typeKind == Type::Kind::Vector ||
                         result->qualifiedType.type.typeKind == Type::Kind::Matrix)
                {
                    const auto operatorKind = BinaryOperatorExpression::Kind::Subscript;

                    auto& rightExpression = parseExpression(iterator, end, declarationScopes);
                    if (!isIntegerType(rightExpression.qualifiedType.type))
                        throw ParseError(ErrorCode::InvalidSubscript, "Subscript is not an integer");

                    expectToken(Token::Type::RightBracket, iterator, end);

                    //auto expression = ;
                    const Type* type = nullptr;

                    if (result->qualifiedType.type.typeKind == Type::Kind::Vector)
                    {
                        auto& vectorType = static_cast<const VectorType&>(result->qualifiedType.type);
                        type = &vectorType.componentType;
                    }
                    else if (result->qualifiedType.type.typeKind == Type::Kind::Matrix)
                    {
                        auto& matrixType = static_cast<const MatrixType&>(result->qualifiedType.type);
                        type = &matrixType.rowType;
                    }

                    result = &create<BinaryOperatorExpression>(operatorKind, *type, result->category, *result, rightExpression);
                }
                else
                    throw ParseError(ErrorCode::ArrayTypeExpected, "Subscript value is not an array");
            }

            return *result;
        }

        static constexpr std::uint8_t charToComponent(char c)
        {
            return (c == 'x' || c == 'r') ? 0 :
                (c == 'y' || c == 'g') ? 1 :
                (c == 'z' || c == 'b') ? 2 :
                (c == 'w' || c == 'a') ? 3 :
                throw ParseError(ErrorCode::InvalidSwizzle, "Invalid component");
        }

        Expression& parseMemberExpression(TokenIterator& iterator, TokenIterator end,
                                          DeclarationScopes& declarationScopes)
        {
            auto result = &parseSubscriptExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::Dot, Token::Type::Arrow}, iterator, end))
            {
                if (isToken(Token::Type::Arrow, iterator, end))
                    throw ParseError(ErrorCode::UnsupportedFeature, "Pointer member access is not supported");

                ++iterator;

                if (result->qualifiedType.type.typeKind == Type::Kind::Void)
                    throw ParseError(ErrorCode::IllegalVoidType, "Expression has a void type");

                if (result->qualifiedType.type.typeKind == Type::Kind::Struct)
                {
                    auto& structType = static_cast<const StructType&>(result->qualifiedType.type);

                    auto& name = expectToken(Token::Type::Identifier, iterator, end).value;

                    auto memberDeclaration = findMemberDeclaration(structType, name);
                    if (!memberDeclaration)
                        throw ParseError(ErrorCode::InvalidMember, "Structure \"" + structType.name +  "\" has no member \"" + name + "\"");

                    if (memberDeclaration->declarationKind != Declaration::Kind::Field)
                        throw ParseError(ErrorCode::InvalidMember, "\"" + iterator->value + "\" is not a field");

                    result = &create<MemberExpression>(*result, *static_cast<const FieldDeclaration*>(memberDeclaration));
                }
                else if (result->qualifiedType.type.typeKind == Type::Kind::Vector)
                {
                    std::vector<std::uint8_t> components;
                    std::set<std::uint8_t> componentSet;

                    Expression::Category category = result->category;
                    Type::Qualifiers qualifiers = Type::Qualifiers::None;

                    const auto& token = expectToken(Token::Type::Identifier, iterator, end);

                    for (const auto c : token.value)
                    {
                        auto component = charToComponent(c);
                        if (!componentSet.insert(component).second) // has component repeated
                        {
                            category = Expression::Category::Rvalue;
                            qualifiers |= Type::Qualifiers::Const;
                        }

                        components.push_back(component);
                    }

                    auto& vectorType = static_cast<const VectorType&>(result->qualifiedType.type);

                    auto resultType = findVectorType(vectorType.componentType, components.size());
                    if (!resultType)
                        throw ParseError(ErrorCode::InvalidSwizzle, "Invalid swizzle");

                    for (auto component : components)
                        if (component >= vectorType.componentCount)
                            throw ParseError(ErrorCode::InvalidSwizzle, "Invalid swizzle");

                    result = &create<VectorElementExpression>(*resultType, qualifiers, category, std::move(components));
                }
                else
                    throw ParseError(ErrorCode::StructTypeExpected, "\"" + result->qualifiedType.type.name + "\" is not a structure");
            }

            return *result;
        }

        Expression& parsePrefixExpression(TokenIterator& iterator, TokenIterator end,
                                          DeclarationScopes& declarationScopes)
        {
            if (isToken({Token::Type::Increment, Token::Type::Decrement}, iterator, end))
            {
                const auto operatorKind = (iterator->type == Token::Type::Increment) ?
                    UnaryOperatorExpression::Kind::PrefixIncrement :
                    UnaryOperatorExpression::Kind::PrefixDecrement;

                ++iterator;

                auto& expression = parseMemberExpression(iterator, end, declarationScopes);

                if (expression.category != Expression::Category::Lvalue)
                    throw ParseError(ErrorCode::ExpressionNotAssignable, "Expression is not assignable");

                if ((expression.qualifiedType.qualifiers & Type::Qualifiers::Const) == Type::Qualifiers::Const)
                    throw ParseError(ErrorCode::ExpressionNotAssignable, "Cannot assign to const variable");

                const auto& unaryOperator = getUnaryOperator(operatorKind,
                                                             expression.qualifiedType.type);

                return create<UnaryOperatorExpression>(operatorKind, unaryOperator.resultType, Expression::Category::Rvalue, expression);
            }
            else
                return parseMemberExpression(iterator, end, declarationScopes);
        }

        Expression& parseSignExpression(TokenIterator& iterator, TokenIterator end,
                                        DeclarationScopes& declarationScopes)
        {
            if (isToken({Token::Type::Plus, Token::Type::Minus}, iterator, end))
            {
                const auto operatorKind = (iterator->type == Token::Type::Plus) ?
                    UnaryOperatorExpression::Kind::Positive :
                    UnaryOperatorExpression::Kind::Negative;

                ++iterator;

                auto& expression = parsePrefixExpression(iterator, end, declarationScopes);

                const auto& unaryOperator = getUnaryOperator(operatorKind,
                                                             expression.qualifiedType.type);

                return create<UnaryOperatorExpression>(operatorKind, unaryOperator.resultType, Expression::Category::Rvalue, expression);
            }
            else
                return parsePrefixExpression(iterator, end, declarationScopes);
        }

        Expression& parseNotExpression(TokenIterator& iterator, TokenIterator end,
                                       DeclarationScopes& declarationScopes)
        {
            if (skipToken(Token::Type::Not, iterator, end))
            {
                const auto operatorKind = UnaryOperatorExpression::Kind::Negation;

                auto& expression = parseExpression(iterator, end, declarationScopes);

                const auto& unaryOperator = getUnaryOperator(operatorKind,
                                                             expression.qualifiedType.type);

                return create<UnaryOperatorExpression>(operatorKind, unaryOperator.resultType, Expression::Category::Rvalue, expression);
            }
            else
                return parseSignExpression(iterator, end, declarationScopes);
        }

        Expression& parseMultiplicationExpression(TokenIterator& iterator, TokenIterator end,
                                                  DeclarationScopes& declarationScopes)
        {
            auto result = &parseNotExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::Multiply, Token::Type::Divide}, iterator, end))
            {
                const auto operatorKind = (iterator->type == Token::Type::Multiply) ?
                    BinaryOperatorExpression::Kind::Multiplication :
                    BinaryOperatorExpression::Kind::Division;

                ++iterator;

                auto& rightExpression = parseNotExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression.qualifiedType.type);

                result = &create<BinaryOperatorExpression>(operatorKind, binaryOperator.resultType, Expression::Category::Rvalue, *result, rightExpression);
            }

            return *result;
        }

        Expression& parseAdditionExpression(TokenIterator& iterator, TokenIterator end,
                                            DeclarationScopes& declarationScopes)
        {
            auto result = &parseMultiplicationExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::Plus, Token::Type::Minus}, iterator, end))
            {
                const auto operatorKind = (iterator->type == Token::Type::Plus) ?
                    BinaryOperatorExpression::Kind::Addition :
                    BinaryOperatorExpression::Kind::Subtraction;

                ++iterator;

                auto& rightExpression = parseMultiplicationExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression.qualifiedType.type);

                result = &create<BinaryOperatorExpression>(operatorKind, binaryOperator.resultType, Expression::Category::Rvalue, *result, rightExpression);
            }

            return *result;
        }

        Expression& parseLessThanExpression(TokenIterator& iterator, TokenIterator end,
                                            DeclarationScopes& declarationScopes)
        {
            auto result = &parseAdditionExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::LessThan, Token::Type::LessThanEqual}, iterator, end))
            {
                const auto operatorKind = (iterator->type == Token::Type::LessThan) ?
                    BinaryOperatorExpression::Kind::LessThan :
                    BinaryOperatorExpression::Kind::LessThanEqual;

                ++iterator;

                auto& rightExpression = parseAdditionExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression.qualifiedType.type);

                result = &create<BinaryOperatorExpression>(operatorKind, binaryOperator.resultType, Expression::Category::Rvalue, *result, rightExpression);
            }

            return *result;
        }

        Expression& parseGreaterThanExpression(TokenIterator& iterator, TokenIterator end,
                                               DeclarationScopes& declarationScopes)
        {
            auto result = &parseLessThanExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::GreaterThan, Token::Type::GreaterThanEqual}, iterator, end))
            {
                const auto operatorKind = (iterator->type == Token::Type::GreaterThan) ?
                    BinaryOperatorExpression::Kind::GreaterThan :
                    BinaryOperatorExpression::Kind::GraterThanEqual;

                ++iterator;

                auto& rightExpression = parseLessThanExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression.qualifiedType.type);

                result = &create<BinaryOperatorExpression>(operatorKind, binaryOperator.resultType, Expression::Category::Rvalue, *result, rightExpression);
            }

            return *result;
        }

        Expression& parseEqualityExpression(TokenIterator& iterator, TokenIterator end,
                                            DeclarationScopes& declarationScopes)
        {
            auto result = &parseGreaterThanExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::Equal, Token::Type::NotEq}, iterator, end))
            {
                const auto operatorKind = (iterator->type == Token::Type::Equal) ?
                    BinaryOperatorExpression::Kind::Equality :
                    BinaryOperatorExpression::Kind::Inequality;

                ++iterator;

                auto& rightExpression = parseGreaterThanExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression.qualifiedType.type);

                result = &create<BinaryOperatorExpression>(operatorKind, binaryOperator.resultType, Expression::Category::Rvalue, *result, rightExpression);
            }

            return *result;
        }

        Expression& parseLogicalAndExpression(TokenIterator& iterator, TokenIterator end,
                                              DeclarationScopes& declarationScopes)
        {
            auto result = &parseEqualityExpression(iterator, end, declarationScopes);

            while (skipToken(Token::Type::And, iterator, end))
            {
                const auto operatorKind = BinaryOperatorExpression::Kind::And;

                auto& rightExpression = parseEqualityExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression.qualifiedType.type);

                result = &create<BinaryOperatorExpression>(operatorKind, binaryOperator.resultType, Expression::Category::Rvalue, *result, rightExpression);
            }

            return *result;
        }

        Expression& parseLogicalOrExpression(TokenIterator& iterator, TokenIterator end,
                                             DeclarationScopes& declarationScopes)
        {
            auto result = &parseLogicalAndExpression(iterator, end, declarationScopes);

            while (skipToken(Token::Type::Or, iterator, end))
            {
                const auto operatorKind = BinaryOperatorExpression::Kind::Or;

                auto& rightExpression = parseLogicalAndExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression.qualifiedType.type);

                result = &create<BinaryOperatorExpression>(operatorKind, binaryOperator.resultType, Expression::Category::Rvalue, *result, rightExpression);
            }

            return *result;
        }

        Expression& parseTernaryExpression(TokenIterator& iterator, TokenIterator end,
                                           DeclarationScopes& declarationScopes)
        {
            auto result = &parseLogicalOrExpression(iterator, end, declarationScopes);

            while (skipToken(Token::Type::Conditional, iterator, end))
            {
                if (!isBooleanType(result->qualifiedType.type))
                    throw ParseError(ErrorCode::ConditionNotBoolean, "Condition is not of the type \"bool\"");

                auto& leftExpression = parseTernaryExpression(iterator, end, declarationScopes);

                expectToken(Token::Type::Colon, iterator, end);

                auto& rightExpression = parseTernaryExpression(iterator, end, declarationScopes);

                if (&leftExpression.qualifiedType.type != &rightExpression.qualifiedType.type)
                    throw ParseError(ErrorCode::IncompatibleOperands, "Incompatible operand types");

                result = &create<TernaryOperatorExpression>(*result, leftExpression, rightExpression);
            }

            return *result;
        }

        Expression& parseAssignmentExpression(TokenIterator& iterator, TokenIterator end,
                                              DeclarationScopes& declarationScopes)
        {
            auto result = &parseTernaryExpression(iterator, end, declarationScopes);

            while (skipToken(Token::Type::Assignment, iterator, end))
            {
                const auto operatorKind = BinaryOperatorExpression::Kind::Assignment;

                if (result->category != Expression::Category::Lvalue)
                    throw ParseError(ErrorCode::ExpressionNotAssignable, "Expression is not assignable");

                if ((result->qualifiedType.qualifiers & Type::Qualifiers::Const) == Type::Qualifiers::Const)
                    throw ParseError(ErrorCode::ExpressionNotAssignable, "Cannot assign to const variable");

                auto& rightExpression = parseTernaryExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression.qualifiedType.type);

                result = &create<BinaryOperatorExpression>(operatorKind, binaryOperator.resultType, Expression::Category::Rvalue, *result, rightExpression);
            }

            return *result;
        }

        Expression& parseAdditionAssignmentExpression(TokenIterator& iterator, TokenIterator end,
                                                      DeclarationScopes& declarationScopes)
        {
            auto result = &parseAssignmentExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::PlusAssignment, Token::Type::MinusAssignment}, iterator, end))
            {
                const auto operatorKind = (iterator->type == Token::Type::PlusAssignment) ?
                    BinaryOperatorExpression::Kind::AdditionAssignment :
                    BinaryOperatorExpression::Kind::SubtractAssignment;

                ++iterator;

                if (result->category != Expression::Category::Lvalue)
                    throw ParseError(ErrorCode::ExpressionNotAssignable, "Expression is not assignable");

                if ((result->qualifiedType.qualifiers & Type::Qualifiers::Const) == Type::Qualifiers::Const)
                    throw ParseError(ErrorCode::ExpressionNotAssignable, "Cannot assign to const variable");

                auto& rightExpression = parseAssignmentExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression.qualifiedType.type);

                result = &create<BinaryOperatorExpression>(operatorKind, binaryOperator.resultType, Expression::Category::Lvalue, *result, rightExpression);
            }

            return *result;
        }

        Expression& parseMultiplicationAssignmentExpression(TokenIterator& iterator, TokenIterator end,
                                                            DeclarationScopes& declarationScopes)
        {
            auto result = &parseAdditionAssignmentExpression(iterator, end, declarationScopes);

            while (isToken({Token::Type::MultiplyAssignment, Token::Type::DivideAssignment}, iterator, end))
            {
                const auto operatorKind = (iterator->type == Token::Type::MultiplyAssignment) ?
                    BinaryOperatorExpression::Kind::MultiplicationAssignment :
                    BinaryOperatorExpression::Kind::DivisionAssignment;

                ++iterator;

                if (result->category != Expression::Category::Lvalue)
                    throw ParseError(ErrorCode::ExpressionNotAssignable, "Expression is not assignable");

                if ((result->qualifiedType.qualifiers & Type::Qualifiers::Const) == Type::Qualifiers::Const)
                    throw ParseError(ErrorCode::ExpressionNotAssignable, "Cannot assign to const variable");

                auto& rightExpression = parseAdditionAssignmentExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression.qualifiedType.type);

                result = &create<BinaryOperatorExpression>(operatorKind, binaryOperator.resultType, Expression::Category::Lvalue, *result, rightExpression);
            }

            return *result;
        }

        Expression& parseCommaExpression(TokenIterator& iterator, TokenIterator end,
                                         DeclarationScopes& declarationScopes)
        {
            auto result = &parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);

            while (skipToken(Token::Type::Comma, iterator, end))
            {
                const auto operatorKind = BinaryOperatorExpression::Kind::Comma;

                auto& rightExpression = parseAdditionAssignmentExpression(iterator, end, declarationScopes);

                const auto& binaryOperator = getBinaryOperator(operatorKind,
                                                               result->qualifiedType.type,
                                                               rightExpression.qualifiedType.type);

                if (&result->qualifiedType.type != &rightExpression.qualifiedType.type)
                    throw ParseError(ErrorCode::IncompatibleOperands, "Incompatible operand types");

                result = &create<BinaryOperatorExpression>(operatorKind, binaryOperator.resultType, rightExpression.category, *result, rightExpression);
            }

            return *result;
        }

        Expression& parseExpression(TokenIterator& iterator, TokenIterator end,
                                    DeclarationScopes& declarationScopes)
        {
            return parseCommaExpression(iterator, end, declarationScopes);
        }

        ScalarType& addScalarType(const std::string& name,
                                  ScalarType::Kind kind,
                                  bool isUnsigned)
        {
            auto& scalarType = create<ScalarType>(name, kind, isUnsigned);

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

        StructType& addStructType(const std::string& name)
        {
            auto& structType = create<StructType>(name, std::vector<const Declaration*>{});

            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Equality, boolType, structType, structType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Inequality, boolType, structType, structType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Assignment, structType, structType, structType);

            return structType;
        }

        VectorType& addVectorType(const std::string& name,
                                  const ScalarType& componentType,
                                  std::size_t componentCount)
        {
            auto& vectorType = create<VectorType>(name, componentType, componentCount);

            vectorTypes[std::make_pair(&componentType, componentCount)] = &vectorType;

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

        MatrixType& addMatrixType(const std::string& name,
                                  const VectorType& rowType,
                                  std::size_t rowCount)
        {
            auto& matrixType = create<MatrixType>(name, rowType, rowCount);

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

            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Multiplication, matrixType, matrixType, rowType.componentType);
            binaryOperators.emplace_back(BinaryOperatorExpression::Kind::Division, matrixType, matrixType, rowType.componentType);
            
            return matrixType;
        }

        FunctionDeclaration& addBuiltinFunctionDeclaration(const std::string& name,
                                                           const Type& resultType,
                                                           const std::vector<const Type*>& parameters,
                                                           DeclarationScopes& declarationScopes)
        {
            std::vector<ParameterDeclaration*> parameterDeclarations;

            for (auto parameter : parameters)
            {
                auto& parameterDeclaration = create<ParameterDeclaration>(QualifiedType{*parameter}, InputModifier::In, std::vector<const Attribute*>{});
                parameterDeclarations.push_back(&parameterDeclaration);
            }

            auto& functionDeclaration = create<FunctionDeclaration>(name, QualifiedType{resultType}, StorageClass::Auto,
                                                                   std::vector<const Attribute*>{}, std::move(parameterDeclarations),
                                                                   FunctionDeclaration::Qualifier::None, true);

            declarationScopes.back().push_back(&functionDeclaration);

            return functionDeclaration;
        }

        struct UnaryOperator final
        {
            UnaryOperator(UnaryOperatorExpression::Kind initUnaryOperatorKind,
                          const Type& initResultType,
                          const Type& initParameterType) noexcept:
                unaryOperatorKind(initUnaryOperatorKind),
                resultType(initResultType),
                parameterType(initParameterType) {}

            UnaryOperatorExpression::Kind unaryOperatorKind;
            const Type& resultType;
            const Type& parameterType;
        };

        const UnaryOperator& getUnaryOperator(UnaryOperatorExpression::Kind unaryOperatorKind,
                                              const Type& parameterType) const
        {
            for (const auto& unaryOperator : unaryOperators)
                if (unaryOperator.unaryOperatorKind == unaryOperatorKind &&
                    &unaryOperator.parameterType == &parameterType)
                    return unaryOperator;

            throw ParseError(ErrorCode::NoOperator, "No unary operator defined for this type");
        }

        struct BinaryOperator final
        {
            BinaryOperator(BinaryOperatorExpression::Kind initBinaryOperatorKind,
                           const Type& initResultType,
                           const Type& initFirstParameterType,
                           const Type& initSecondParameterType) noexcept:
                binaryOperatorKind(initBinaryOperatorKind),
                resultType(initResultType),
                firstParameterType(initFirstParameterType),
                secondParameterType(initSecondParameterType) {}

            BinaryOperatorExpression::Kind binaryOperatorKind;
            const Type& resultType;
            const Type& firstParameterType;
            const Type& secondParameterType;
        };

        const BinaryOperator& getBinaryOperator(BinaryOperatorExpression::Kind binaryOperatorKind,
                                                const Type& firstParameterType,
                                                const Type& secondParameterType) const
        {
            for (const auto& binaryOperator : binaryOperators)
                if (binaryOperator.binaryOperatorKind == binaryOperatorKind &&
                    &binaryOperator.firstParameterType == &firstParameterType &&
                    &binaryOperator.secondParameterType == &secondParameterType)
                    return binaryOperator;

            throw ParseError(ErrorCode::NoOperator, "No binary operator defined for these types");
        }

        template <class T, class ...Args, typename std::enable_if<std::is_base_of<Type, T>::value>::type* = nullptr>
        T& create(Args&&... args)
        {
            T* result;
            types.push_back(std::unique_ptr<Type>(result = new T(std::forward<Args>(args)...)));
            return *result;
        }

        template <class T, class ...Args, typename std::enable_if<std::is_base_of<Construct, T>::value>::type* = nullptr>
        T& create(Args&&... args)
        {
            T* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new T(std::forward<Args>(args)...)));
            return *result;
        }

        std::vector<std::unique_ptr<Type>> types;
        std::vector<Declaration*> declarations;
        std::vector<std::unique_ptr<Construct>> constructs;

        std::vector<UnaryOperator> unaryOperators;
        std::vector<BinaryOperator> binaryOperators;

        std::map<std::pair<const Type*, std::uint8_t>, const VectorType*> vectorTypes;
        std::map<std::pair<QualifiedType, std::size_t>, const ArrayType*> arrayTypes;

        const Type& voidType;
        const ScalarType& boolType;
        const ScalarType& intType;
        const ScalarType& uintType;
        const ScalarType& floatType;
        const StructType& stringType;
    };
}

#endif // PARSER_HPP
