//
//  OSL
//

#include <algorithm>
#include "Parser.hpp"
#include "Utils.hpp"

namespace ouzel
{
    namespace
    {
        inline bool isToken(Token::Type tokenType,
                            std::vector<Token>::const_iterator iterator,
                            std::vector<Token>::const_iterator end) noexcept
        {
            return (iterator != end && iterator->type == tokenType);
        }

        inline void expectToken(Token::Type tokenType,
                                std::vector<Token>::const_iterator iterator,
                                std::vector<Token>::const_iterator end)
        {
            if (iterator == end)
                throw ParseError("Unexpected end of file");
            if (iterator->type != tokenType)
                throw ParseError("Expected " + toString(tokenType));
        }

        template <size_t N>
        inline bool isToken(const Token::Type (&tokenTypes)[N],
                            std::vector<Token>::const_iterator iterator,
                            std::vector<Token>::const_iterator end) noexcept
        {
            if (iterator == end) return false;

            for (Token::Type tokenType : tokenTypes)
                if (iterator->type == tokenType) return true;

            return false;
        }

        template <size_t N>
        inline void expectToken(const Token::Type (&tokenTypes)[N],
                                std::vector<Token>::const_iterator iterator,
                                std::vector<Token>::const_iterator end)
        {
            if (iterator == end)
                throw ParseError("Unexpected end of file");

            for (Token::Type tokenType : tokenTypes)
                if (iterator->type == tokenType) return;

            std::string str;
            for (Token::Type tokenType : tokenTypes)
            {
                if (!str.empty()) str += "or ";
                str += toString(tokenType);
            }

            throw ParseError("Expected " + str);
        }

        bool isBooleanType(const Type* type)
        {
            if (type->getTypeKind() != Type::Kind::Scalar)
                return false;

            auto scalarType = static_cast<const ScalarType*>(type);

            return scalarType->getScalarTypeKind() == ScalarType::Kind::Boolean;
        }

        bool isIntegerType(const Type* type) noexcept
        {
            if (type->getTypeKind() != Type::Kind::Scalar)
                return false;

            auto scalarType = static_cast<const ScalarType*>(type);

            return scalarType->getScalarTypeKind() == ScalarType::Kind::Boolean ||
                scalarType->getScalarTypeKind() == ScalarType::Kind::Integer;
        }
    }

    ASTContext::ASTContext(const std::vector<Token>& tokens)
    {
        std::vector<std::vector<Declaration*>> declarationScopes;
        declarationScopes.push_back(std::vector<Declaration*>());

        addBuiltinFunctionDeclaration("discard", nullptr, {}, declarationScopes);

        Type* voidTypePtr;
        types.push_back(std::unique_ptr<Type>(voidTypePtr = new Type(Type::Kind::Void)));
        voidTypePtr->name = "void";
        voidType = voidTypePtr;

        boolType = addScalarType("bool", ScalarType::Kind::Boolean, 1, false, declarationScopes);

        intType = addScalarType("int", ScalarType::Kind::Integer, 4, false, declarationScopes);
        unsignedIntType = addScalarType("unsigned int", ScalarType::Kind::Integer, 4, true, declarationScopes);
        floatType = addScalarType("float", ScalarType::Kind::FloatingPoint, 4, false, declarationScopes);

        VectorType* float2Type = addVectorType("float2", floatType, 2, declarationScopes);
        VectorType* float3Type = addVectorType("float3", floatType, 3, declarationScopes);
        VectorType* float4Type = addVectorType("float4", floatType, 4, declarationScopes);

        MatrixType* float2x2Type = addMatrixType("float2x2", floatType, 2, 2, declarationScopes);
        MatrixType* float3x3Type = addMatrixType("float3x3", floatType, 3, 3, declarationScopes);
        MatrixType* float4x4Type = addMatrixType("float4x4", floatType, 4, 4, declarationScopes);
        stringType = addStructType("string", 8, declarationScopes);
        StructType* texture2DType = addStructType("Texture2D", 0, declarationScopes);

        addBuiltinFunctionDeclaration("sample", float4Type, {texture2DType, float2Type}, declarationScopes);

        StructType* texture2DMSType = addStructType("Texture2DMS", 0, declarationScopes);

        addBuiltinFunctionDeclaration("load", float4Type, {texture2DMSType, float2Type}, declarationScopes);

        for (auto iterator = tokens.begin(); iterator != tokens.end();)
        {
            Declaration* declaration = parseTopLevelDeclaration(iterator, tokens.end(), declarationScopes);
            declarations.push_back(declaration);
        }
    }

    enum class Rank
    {
        NoRank = 0,
        Conversion = 1,
        Promotion = 2,
        ExactMatch = 3
    };

    namespace
    {
        Rank getRank(const QualifiedType& parameterType,
                     const QualifiedType& argumentType)
        {
            if (!parameterType.type)
                throw ParseError("Parameter does not have a type");

            if (!argumentType.type)
                throw ParseError("Argument does not have a type");

            if (argumentType.type->getTypeKind() == parameterType.type->getTypeKind())
            {
                if (parameterType.type == argumentType.type)
                    return Rank::ExactMatch;
                else if (argumentType.type->getTypeKind() == Type::Kind::Array)
                {
                    auto argumentArrayType = static_cast<const ArrayType*>(argumentType.type);
                    auto parameterArrayType = static_cast<const ArrayType*>(parameterType.type);

                    if (argumentArrayType->size == parameterArrayType->size)
                        return Rank::ExactMatch;
                    else
                        return Rank::NoRank;
                }
                else if (argumentType.type->getTypeKind() == Type::Kind::Scalar)
                {
                    auto argumentTypeDeclaration = static_cast<const ScalarType*>(argumentType.type);
                    auto parameterTypeDeclaration = static_cast<const ScalarType*>(parameterType.type);

                    if (argumentTypeDeclaration->getScalarTypeKind() == parameterTypeDeclaration->getScalarTypeKind() &&
                        argumentTypeDeclaration->isUnsigned == parameterTypeDeclaration->isUnsigned)
                    {
                        if (argumentTypeDeclaration->size == parameterTypeDeclaration->size)
                            return Rank::ExactMatch;
                        else if (argumentTypeDeclaration->size < parameterTypeDeclaration->size)
                            return Rank::Promotion;
                        else
                            return Rank::Conversion;
                    }
                    else
                        return Rank::Conversion;
                }
            }

            return Rank::NoRank;
        }
    }

    const CallableDeclaration* ASTContext::compareCallableDeclarations(const CallableDeclaration* callableDeclaration1,
                                                                 const CallableDeclaration* callableDeclaration2,
                                                                 const std::vector<QualifiedType>& arguments)
    {
        const CallableDeclaration* result = nullptr;

        if (!arguments.empty() && // both functions should have arguments
            arguments.size() == callableDeclaration1->parameterDeclarations.size() &&
            arguments.size() == callableDeclaration2->parameterDeclarations.size()) // they should have the same number of parameters
        {
            for (uint32_t i = 0; i < arguments.size(); ++i)
            {
                const QualifiedType& argument = arguments[i];
                const QualifiedType& parameter1 = callableDeclaration1->parameterDeclarations[i]->qualifiedType;
                const QualifiedType& parameter2 = callableDeclaration2->parameterDeclarations[i]->qualifiedType;

                const Rank rank1 = getRank(parameter1, argument);
                const Rank rank2 = getRank(parameter2, argument);

                if (rank1 == Rank::NoRank && rank2 == Rank::NoRank) // no valid rank for both
                    return nullptr;
                else if (rank1 == rank2) // equal ranks
                    continue;
                else if (rank1 > rank2)
                {
                    if (result == nullptr)
                        result = callableDeclaration1;
                    else if (result != callableDeclaration1)
                        return nullptr;
                }
                else if (rank2 > rank1)
                {
                    if (result == nullptr)
                        result = callableDeclaration2;
                    else if (result != callableDeclaration2)
                        return nullptr;
                }
            }
        }

        return result;
    }

    FunctionDeclaration* ASTContext::resolveFunctionDeclaration(const std::string& name,
                                                                const std::vector<std::vector<Declaration*>>& declarationScopes,
                                                                const std::vector<QualifiedType>& arguments)
    {
        std::vector<FunctionDeclaration*> candidateFunctionDeclarations;

        for (auto scopeIterator = declarationScopes.crbegin(); scopeIterator != declarationScopes.crend(); ++scopeIterator)
        {
            for (auto declarationIterator = scopeIterator->crbegin(); declarationIterator != scopeIterator->crend(); ++declarationIterator)
            {
                if ((*declarationIterator)->name == name)
                {
                    if ((*declarationIterator)->getDeclarationKind() != Declaration::Kind::Callable) return nullptr;

                    CallableDeclaration* callableDeclaration = static_cast<CallableDeclaration*>(*declarationIterator);

                    if (callableDeclaration->getCallableDeclarationKind() != CallableDeclaration::Kind::Function) return nullptr;

                    FunctionDeclaration* functionDeclaration = static_cast<FunctionDeclaration*>(callableDeclaration->getFirstDeclaration());

                    if (std::find(candidateFunctionDeclarations.begin(), candidateFunctionDeclarations.end(), functionDeclaration) == candidateFunctionDeclarations.end())
                        candidateFunctionDeclarations.push_back(functionDeclaration);
                }
            }
        }

        std::vector<FunctionDeclaration*> viableFunctionDeclarations;

        for (FunctionDeclaration* functionDeclaration : candidateFunctionDeclarations)
        {
            if (functionDeclaration->parameterDeclarations.size() == arguments.size())
            {
                if (std::equal(arguments.begin(), arguments.end(),
                               functionDeclaration->parameterDeclarations.begin(),
                               [](const QualifiedType& qualifiedType,
                                  const ParameterDeclaration* parameterDeclaration) {
                                   bool scalar = qualifiedType.type->getTypeKind() == Type::Kind::Scalar &&
                                   qualifiedType.type->getTypeKind() == Type::Kind::Scalar;

                                   return (scalar || qualifiedType.type == parameterDeclaration->qualifiedType.type);
                               }))
                {
                    viableFunctionDeclarations.push_back(functionDeclaration);
                }
            }
        }

        if (viableFunctionDeclarations.empty())
            throw ParseError("No matching function to call " + name + " found");
        else if (viableFunctionDeclarations.size() == 1)
            return *viableFunctionDeclarations.begin();
        else
        {
            if (arguments.empty()) // two or more functions with zero parameters
                throw ParseError("Ambiguous call to " + name);

            for (auto first = viableFunctionDeclarations.begin(); first != viableFunctionDeclarations.end(); ++first)
            {
                bool best = true;
                for (auto second = viableFunctionDeclarations.begin(); second != viableFunctionDeclarations.end(); ++second)
                {
                    if (first != second &&
                        compareCallableDeclarations(*first, *second, arguments) != *first)
                    {
                        best = false;
                        break;
                    }
                }

                if (best) return *first;
            };

            throw ParseError("Ambiguous call to " + name);
        }

        return nullptr;
    }

    const ArrayType* ASTContext::getArrayType(QualifiedType qualifiedType, uint32_t size)
    {
        auto i = arrayTypes.find(std::make_pair(qualifiedType, size));

        if (i != arrayTypes.end())
        {
            return i->second;
        }
        else
        {
            ArrayType* result;
            types.push_back(std::unique_ptr<Type>(result = new ArrayType()));
            result->elementType = qualifiedType;
            result->size = size;

            arrayTypes[std::make_pair(qualifiedType, size)] = result;
            return result;
        }
    }

    bool ASTContext::isType(std::vector<Token>::const_iterator iterator,
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

    const Type* ASTContext::parseType(std::vector<Token>::const_iterator& iterator,
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

        return result;
    }

    bool ASTContext::isDeclaration(std::vector<Token>::const_iterator iterator,
                                   std::vector<Token>::const_iterator end,
                                   std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        if (iterator == end)
            throw ParseError("Unexpected end of file");

        return iterator->type == Token::Type::Const ||
            iterator->type == Token::Type::Inline ||
            iterator->type == Token::Type::Signed ||
            iterator->type == Token::Type::Unsigned ||
            iterator->type == Token::Type::Struct ||
            iterator->type == Token::Type::In ||
            iterator->type == Token::Type::Inout ||
            iterator->type == Token::Type::Out ||
            isType(iterator, end, declarationScopes);
    }

    Declaration* ASTContext::parseTopLevelDeclaration(std::vector<Token>::const_iterator& iterator,
                                                      std::vector<Token>::const_iterator end,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(iterator, end, declarationScopes, nullptr)))
            throw ParseError("Failed to parse a declaration");

        if (declaration->getDeclarationKind() == Declaration::Kind::Callable)
        {
            const CallableDeclaration* callableDeclaration = static_cast<CallableDeclaration*>(declaration);

            // semicolon is not needed after a function definition
            if (!callableDeclaration->body)
            {
                expectToken(Token::Type::Semicolon, iterator, end);

                ++iterator;
            }
        }
        else
        {
            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;
        }

        return declaration;
    }

    ASTContext::Specifiers ASTContext::parseSpecifiers(std::vector<Token>::const_iterator& iterator,
                                                       std::vector<Token>::const_iterator end)
    {
        ASTContext::Specifiers result;
        result.isInline = false;

        for (;;)
        {
            if (isToken(Token::Type::Const, iterator, end))
            {
                ++iterator;
                result.qualifiers |= Qualifiers::Const;
            }
            else if (isToken(Token::Type::Extern, iterator, end))
            {
                ++iterator;
                result.isExtern = true;
            }
            else if (isToken(Token::Type::Inline, iterator, end))
            {
                ++iterator;
                result.isInline = true;
            }
            else if (isToken(Token::Type::Volatile, iterator, end))
            {
                ++iterator;
                result.qualifiers |= Qualifiers::Volatile;
            }
            else if (isToken(Token::Type::In, iterator, end))
            {
                ++iterator;
                result.qualifiers |= Qualifiers::In;
            }
            else if (isToken(Token::Type::Inout, iterator, end))
            {
                ++iterator;
                result.qualifiers |= Qualifiers::In | Qualifiers::Out;
            }
            else if (isToken(Token::Type::Out, iterator, end))
            {
                ++iterator;
                result.qualifiers |= Qualifiers::Out;
            }
            else if (isToken(Token::Type::ProgFragment, iterator, end))
            {
                if (result.program != Program::None &&
                    result.program != Program::Fragment)
                    throw ParseError("Single function can not have multiple program specifiers");

                ++iterator;
                result.program = Program::Fragment;
            }
            else if (isToken(Token::Type::ProgVertex, iterator, end))
            {
                if (result.program != Program::None &&
                    result.program != Program::Vertex)
                    throw ParseError("Single function can not have multiple program specifiers");

                ++iterator;
                result.program = Program::Vertex;
            }
            else break;
        }

        return result;
    }

    Declaration* ASTContext::parseDeclaration(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent)
    {
        if (isToken(Token::Type::Semicolon, iterator, end))
        {
            Declaration* declaration;
            constructs.push_back(std::unique_ptr<Construct>(declaration = new Declaration(Declaration::Kind::Empty)));
            declaration->parent = parent;
            declarationScopes.back().push_back(declaration);

            return declaration;
        }
        else if (isToken(Token::Type::Struct, iterator, end))
        {
            ++iterator;

            TypeDeclaration* typeDeclaration;

            if (!(typeDeclaration = parseStructTypeDeclaration(iterator, end, declarationScopes, parent)))
                throw ParseError("Failed to parse a structure declaration");

            return typeDeclaration;
        }
        /*else if (isToken(Token::Type::KEYWORD_TYPEDEF, iterator, end))
        {
            ++iterator;

            TypeDefinitionDeclaration* declaration;
            if (!(declaration = parseTypeDefinitionDeclaration(iterator, end, declarationScopes, parent)))
                throw ParseError("Failed to parse a type definition declaration");

            return declaration;
        }*/
        else
        {
            ASTContext::Specifiers specifiers = parseSpecifiers(iterator, end);

            QualifiedType qualifiedType;
            qualifiedType.qualifiers |= specifiers.qualifiers;

            bool isExtern = specifiers.isExtern;
            bool isInline = specifiers.isInline;
            Program program = specifiers.program;

            if (!(qualifiedType.type = parseType(iterator, end, declarationScopes)))
                return nullptr;

            if (qualifiedType.type->getTypeKind() == Type::Kind::Struct)
            {
                auto structType = static_cast<const StructType*>(qualifiedType.type);

                if (!structType->definition)
                    throw ParseError("Incomplete type " + qualifiedType.type->name);
            }

            specifiers = parseSpecifiers(iterator, end);

            qualifiedType.qualifiers |= specifiers.qualifiers;

            if (specifiers.isExtern) isExtern = true;
            if (specifiers.isInline) isInline = true;

            if (specifiers.program != Program::None)
            {
                if (program != Program::None &&
                    program != specifiers.program)
                    throw ParseError("Single function can not have multiple program specifiers");

                program = specifiers.program;
            }

            if (isToken(Token::Type::Operator, iterator, end))
                throw ParseError("Operator overloads are not supported");

            expectToken(Token::Type::Identifier, iterator, end);

            const std::string name = iterator->value;

            ++iterator;

            if (isToken(Token::Type::LeftParenthesis, iterator, end) &&
                (isToken(Token::Type::RightParenthesis, iterator + 1, end) ||
                 isToken(Token::Type::Void, iterator + 1, end) ||
                 isDeclaration(iterator + 1, end, declarationScopes)))  // function declaration
            {
                if ((qualifiedType.qualifiers & Qualifiers::In) == Qualifiers::In ||
                    (qualifiedType.qualifiers & Qualifiers::Out) == Qualifiers::Out)
                    throw ParseError("Functions can not have in or out qualifiers");

                ++iterator;

                FunctionDeclaration* result;
                constructs.push_back(std::unique_ptr<Construct>(result = new FunctionDeclaration()));
                result->parent = parent;
                result->qualifiedType = qualifiedType;
                result->isInline = isInline;
                result->program = program;
                result->name = name;

                std::vector<QualifiedType> parameters;

                if (isToken(Token::Type::Void, iterator, end))
                    ++iterator;
                else if (!isToken(Token::Type::RightParenthesis, iterator, end))
                {
                    for (;;)
                    {
                        ParameterDeclaration* parameterDeclaration;
                        if (!(parameterDeclaration = parseParameterDeclaration(iterator, end, declarationScopes, result)))
                            return nullptr;

                        result->parameterDeclarations.push_back(parameterDeclaration);
                        parameters.push_back(parameterDeclaration->qualifiedType);

                        if (!isToken(Token::Type::Comma, iterator, end))
                            break;

                        ++iterator;
                    }
                }

                expectToken(Token::Type::RightParenthesis, iterator, end);

                ++iterator;

                result->previousDeclaration = findFunctionDeclaration(name, declarationScopes, parameters);

                declarationScopes.back().push_back(result);

                // set the definition of the previous declaration
                if (result->previousDeclaration) result->definition = result->previousDeclaration->definition;

                if (isToken(Token::Type::LeftBrace, iterator, end))
                {
                    // check if only one definition exists
                    if (result->definition)
                        throw ParseError("Redefinition of " + result->name);

                    // set the definition pointer of all previous declarations
                    Declaration* previousDeclaration = result->previousDeclaration;
                    while (previousDeclaration)
                    {
                        previousDeclaration->definition = result;
                        previousDeclaration = previousDeclaration->previousDeclaration;
                    }

                    declarationScopes.push_back(std::vector<Declaration*>()); // add scope for parameters

                    for (ParameterDeclaration* parameterDeclaration : result->parameterDeclarations)
                        declarationScopes.back().push_back(parameterDeclaration);

                    // parse body
                    if (!(result->body = parseCompoundStatement(iterator, end, declarationScopes, result)))
                        throw ParseError("Failed to parse a compound statement");

                    declarationScopes.pop_back();
                }

                return result;
            }
            else // variable declaration
            {
                if (qualifiedType.type->getTypeKind() == Type::Kind::Void)
                    throw ParseError("Variable can not have a void type");

                if (isInline)
                    throw ParseError("Variables can not be inline");

                if (program != Program::None)
                    throw ParseError("Variables can not have program specifier");

                if (findDeclaration(name, declarationScopes.back()))
                    throw ParseError("Redefinition of " + name);

                VariableDeclaration* result;
                constructs.push_back(std::unique_ptr<VariableDeclaration>(result = new VariableDeclaration()));
                result->parent = parent;
                result->qualifiedType = qualifiedType;
                if (isExtern) result->storageClass = VariableDeclaration::StorageClass::Extern;
                result->name = name;

                while (isToken(Token::Type::LeftBracket, iterator, end))
                {
                    ++iterator;

                    expectToken(Token::Type::IntLiteral, iterator, end);

                    const int size = std::stoi(iterator->value);

                    ++iterator;

                    if (size <= 0)
                        throw ParseError("Array size must be greater than zero");

                    result->qualifiedType.type = getArrayType(result->qualifiedType, static_cast<uint32_t>(size));

                    expectToken(Token::Type::RightBracket, iterator, end);

                    ++iterator;
                }

                if (isToken(Token::Type::LeftParenthesis, iterator, end))
                {
                    ++iterator;

                    if (!(result->initialization = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, result)))
                        return nullptr;

                    if (result->initialization->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                        throw ParseError("Member can not have a void type");

                    // TODO: check for comma and parse multiple expressions

                    expectToken(Token::Type::RightParenthesis, iterator, end);

                    ++iterator;
                }
                else if (isToken(Token::Type::Assignment, iterator, end))
                {
                    ++iterator;

                    if (!(result->initialization = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, result)))
                        return nullptr;

                    if (result->initialization->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                        throw ParseError("Initialization with a void type");
                }

                declarationScopes.back().push_back(result);

                return result;
            }
        }

        return nullptr;
    }

    TypeDeclaration* ASTContext::parseStructTypeDeclaration(std::vector<Token>::const_iterator& iterator,
                                                            std::vector<Token>::const_iterator end,
                                                            std::vector<std::vector<Declaration*>>& declarationScopes,
                                                            Construct* parent)
    {
        TypeDeclaration* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new TypeDeclaration()));
        result->parent = parent;

        expectToken(Token::Type::Identifier, iterator, end);
        result->name = iterator->value;

        StructType* structType = findStructType(result->name, declarationScopes);
        if (!structType)
        {
            types.push_back(std::unique_ptr<Type>(structType = new StructType()));
            structType->name = iterator->value;
            structType->declaration = result;
        }

        result->type = structType;

        ++iterator;

        declarationScopes.back().push_back(result);

        if (isToken(Token::Type::LeftBrace, iterator, end))
        {
            ++iterator;

            // check if only one definition exists
            if (structType->definition)
                throw ParseError("Redefinition of " + structType->name);

            structType->definition = result;

            for (;;)
            {
                if (isToken(Token::Type::RightBrace, iterator, end))
                {
                    ++iterator;
                    break;
                }
                else
                {
                    Declaration* memberDeclaration;
                    if (!(memberDeclaration = parseMemberDeclaration(iterator, end, declarationScopes, result)))
                        return nullptr;

                    expectToken(Token::Type::Semicolon, iterator, end);

                    if (structType->findMemberDeclaration(memberDeclaration->name))
                        throw ParseError("Redefinition of member " + memberDeclaration->name);

                    ++iterator;

                    memberDeclaration->parent = result;

                    structType->memberDeclarations.push_back(memberDeclaration);
                }
            }
        }

        return result;
    }

    Declaration* ASTContext::parseMemberDeclaration(std::vector<Token>::const_iterator& iterator,
                                                    std::vector<Token>::const_iterator end,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                                    Construct* parent)
    {
        if (isToken(Token::Type::Semicolon, iterator, end))
        {
            Declaration* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new Declaration(Declaration::Kind::Empty)));
            result->parent = parent;

            return result;
        }
        else
        {
            FieldDeclaration* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new FieldDeclaration()));
            result->parent = parent;

            ASTContext::Specifiers specifiers = parseSpecifiers(iterator, end);

            result->qualifiedType.qualifiers |= specifiers.qualifiers;

            bool isInline = specifiers.isInline;

            if (!(result->qualifiedType.type = parseType(iterator, end, declarationScopes)))
                return nullptr;

            if (result->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                throw ParseError("Member can not have a void type");

            if (result->qualifiedType.type->getTypeKind() == Type::Kind::Struct)
            {
                auto structType = static_cast<const StructType*>(result->qualifiedType.type);

                if (!structType->definition)
                    throw ParseError("Incomplete type " + result->qualifiedType.type->name);
            }

            specifiers = parseSpecifiers(iterator, end);

            result->qualifiedType.qualifiers |= specifiers.qualifiers;

            if (specifiers.isInline) isInline = true;

            if (isInline)
                throw ParseError("Members can not be inline");

            if ((result->qualifiedType.qualifiers & Qualifiers::In) == Qualifiers::In ||
                (result->qualifiedType.qualifiers & Qualifiers::Out) == Qualifiers::Out)
                throw ParseError("Members can not have in or out qualifiers");

            expectToken(Token::Type::Identifier, iterator, end);

            result->name = iterator->value;

            ++iterator;

            while (isToken(Token::Type::LeftBracket, iterator, end))
            {
                ++iterator;

                expectToken(Token::Type::IntLiteral, iterator, end);

                const int size = std::stoi(iterator->value);

                ++iterator;

                if (size <= 0)
                    throw ParseError("Array size must be greater than zero");

                result->qualifiedType.type = getArrayType(result->qualifiedType, static_cast<uint32_t>(size));

                expectToken(Token::Type::RightBracket, iterator, end);

                ++iterator;
            }

            return result;
        }

        return nullptr;
    }

    ParameterDeclaration* ASTContext::parseParameterDeclaration(std::vector<Token>::const_iterator& iterator,
                                                                std::vector<Token>::const_iterator end,
                                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                                Construct* parent)
    {
        ParameterDeclaration* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new ParameterDeclaration()));
        result->parent = parent;

        ASTContext::Specifiers specifiers = parseSpecifiers(iterator, end);

        result->qualifiedType.qualifiers |= specifiers.qualifiers;

        bool isInline = specifiers.isInline;

        if (!(result->qualifiedType.type = parseType(iterator, end, declarationScopes)))
            return nullptr;

        if (result->qualifiedType.type->getTypeKind() == Type::Kind::Void)
            throw ParseError("Parameter can not have a void type");

        if (result->qualifiedType.type->getTypeKind() == Type::Kind::Struct)
        {
            auto structType = static_cast<const StructType*>(result->qualifiedType.type);

            if (!structType->definition)
                throw ParseError("Incomplete type " + result->qualifiedType.type->name);
        }

        specifiers = parseSpecifiers(iterator, end);

        result->qualifiedType.qualifiers |= specifiers.qualifiers;

        if (specifiers.isInline) isInline = true;

        if (isInline)
            throw ParseError("Parameters can not be inline");

        if (isToken(Token::Type::Identifier, iterator, end))
        {
            result->name = iterator->value;

            ++iterator;
        }

        while (isToken(Token::Type::LeftBracket, iterator, end))
        {
            ++iterator;

            expectToken(Token::Type::IntLiteral, iterator, end);

            const int size = std::stoi(iterator->value);

            ++iterator;

            if (size <= 0)
                throw ParseError("Array size must be greater than zero");

            result->qualifiedType.type = getArrayType(result->qualifiedType, static_cast<uint32_t>(size));

            expectToken(Token::Type::RightBracket, iterator, end);

            ++iterator;
        }

        return result;
    }

    /*TypeDefinitionDeclaration* ASTContext::parseTypeDefinitionDeclaration(std::vector<Token>::const_iterator& iterator,
                                                                            std::vector<Token>::const_iterator end,
                                                                            std::vector<std::vector<Declaration*>>& declarationScopes,
                                                                            Construct* parent)
    {
        throw ParseError("Typedef is not supported");
        return nullptr;

        TypeDefinitionDeclaration* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new TypeDefinitionDeclaration()));
        result->parent = parent;
        result->kind = Construct::Kind::Declaration;
        result->declarationKind = Declaration::Kind::Type;
        result->typeKind = Type::Kind::TypeDefinition;
        result->Declaration = findTypeDeclaration(iterator->value, declarationScopes);

        if (!(result->qualifiedType.type = parseType(iterator, end, declarationScopes)))
            return nullptr;

        if (result->qualifiedType.type->getTypeKind() == Type::Kind::STRUCT)
        {
            StructType* structType = static_cast<StructType*>(result->qualifiedType.type);

            if (!structType->hasDefinition)
                throw ParseError("Incomplete type " + result->qualifiedType.type->name);
        }

        expectToken(Token::Type::IDENTIFIER, iterator, end);

        result->name = iterator->value;

        ++iterator;

        expectToken(Token::Type::SEMICOLON, iterator, end);

        ++iterator;

        return result;
    }*/

    Statement* ASTContext::parseStatement(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent)
    {
        if (isToken(Token::Type::LeftBrace, iterator, end))
            return parseCompoundStatement(iterator, end, declarationScopes, parent);
        else if (isToken(Token::Type::If, iterator, end))
            return parseIfStatement(iterator, end, declarationScopes, parent);
        else if (isToken(Token::Type::For, iterator, end))
            return parseForStatement(iterator, end, declarationScopes, parent);
        else if (isToken(Token::Type::Switch, iterator, end))
            return parseSwitchStatement(iterator, end, declarationScopes, parent);
        else if (isToken(Token::Type::Case, iterator, end))
            return parseCaseStatement(iterator, end, declarationScopes, parent);
        else if (isToken(Token::Type::Default, iterator, end))
            return parseDefaultStatement(iterator, end, declarationScopes, parent);
        else if (isToken(Token::Type::While, iterator, end))
            return parseWhileStatement(iterator, end, declarationScopes, parent);
        else if (isToken(Token::Type::Do, iterator, end))
            return parseDoStatement(iterator, end, declarationScopes, parent);
        else if (isToken(Token::Type::Break, iterator, end))
        {
            ++iterator;

            BreakStatement* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new BreakStatement()));
            result->parent = parent;

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;

            return result;
        }
        else if (isToken(Token::Type::Continue, iterator, end))
        {
            ++iterator;

            ContinueStatement* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new ContinueStatement()));
            result->parent = parent;

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;

            return result;
        }
        else if (isToken(Token::Type::Return, iterator, end))
        {
            ++iterator;

            ReturnStatement* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new ReturnStatement()));
            result->parent = parent;

            if (!(result->result = parseExpression(iterator, end, declarationScopes, result)))
                throw ParseError("Expected an expression");

            Construct* currentParent = parent;
            CallableDeclaration* callableDeclaration = nullptr;

            while (currentParent)
            {
                if (currentParent->getKind() == Construct::Kind::Declaration)
                {
                    Declaration* declaration = static_cast<Declaration*>(currentParent);
                    if (declaration->getDeclarationKind() == Declaration::Kind::Callable)
                    {
                        callableDeclaration = static_cast<CallableDeclaration*>(currentParent);
                        break;
                    }
                }

                currentParent = currentParent->parent;
            }

            if (!callableDeclaration)
                throw ParseError("Return statement outside of a function");

            if (callableDeclaration->qualifiedType.type != result->result->qualifiedType.type)
                throw ParseError("Invalid type for a return statement");

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;

            return result;
        }
        else if (isDeclaration(iterator, end, declarationScopes))
        {
            DeclarationStatement* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new DeclarationStatement()));
            result->parent = parent;

            if (!(result->declaration = parseDeclaration(iterator, end, declarationScopes, result)))
                return nullptr;

            if (result->declaration->getDeclarationKind() != Declaration::Kind::Variable)
                throw ParseError("Expected a variable declaration");

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;

            return result;
        }
        else if (isToken(Token::Type::Semicolon, iterator, end))
        {
            ++iterator;

            Statement* statement = new Statement(Statement::Kind::Empty);
            constructs.push_back(std::unique_ptr<Construct>(statement));
            statement->parent = parent;

            return statement;
        }
        else if (isToken(Token::Type::Asm, iterator, end))
            throw ParseError("asm statements are not supported");
        else if (isToken(Token::Type::Goto, iterator, end))
            throw ParseError("goto statements are not supported");
        else if (isToken({Token::Type::Try,
            Token::Type::Catch,
            Token::Type::Throw}, iterator, end))
            throw ParseError("Exceptions are not supported");
        else if (iterator == end ||
                 isToken(Token::Type::RightBrace, iterator, end))
            throw ParseError("Exceptions a statement");
        else
        {
            ExpressionStatement* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new ExpressionStatement()));
            result->parent = parent;

            if (!(result->expression = parseExpression(iterator, end, declarationScopes, result)))
                return nullptr;

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;

            return result;
        }

        return nullptr;
    }

    CompoundStatement* ASTContext::parseCompoundStatement(std::vector<Token>::const_iterator& iterator,
                                                          std::vector<Token>::const_iterator end,
                                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                                          Construct* parent)
    {
        expectToken(Token::Type::LeftBrace, iterator, end);

        ++iterator;

        declarationScopes.push_back(std::vector<Declaration*>());

        CompoundStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new CompoundStatement()));
        result->parent = parent;

        for (;;)
        {
            if (isToken(Token::Type::RightBrace, iterator, end))
            {
                ++iterator;
                break;
            }
            else
            {
                Statement* statement;
                if (!(statement = parseStatement(iterator, end, declarationScopes, result)))
                    throw ParseError("Failed to parse a statement");

                result->statements.push_back(statement);
            }
        }

        declarationScopes.pop_back();

        return result;
    }

    IfStatement* ASTContext::parseIfStatement(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent)
    {
        expectToken(Token::Type::If, iterator, end);

        ++iterator;

        IfStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new IfStatement()));
        result->parent = parent;

        expectToken(Token::Type::LeftParenthesis, iterator, end);

        ++iterator;

        if (isDeclaration(iterator, end, declarationScopes))
        {
            // TODO: add implicit cast to bool
            Declaration* declaration;
            if (!(declaration = parseDeclaration(iterator, end, declarationScopes, result)))
                return nullptr;

            if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
                declaration->getDeclarationKind() != Declaration::Kind::Parameter)
                throw ParseError("Expected a variable declaration");

            result->condition = declaration;
        }
        else
        {
            Expression* condition = parseExpression(iterator, end, declarationScopes, result);

            if (!condition)
                return nullptr;

            if (!isBooleanType(condition->qualifiedType.type))
                throw ParseError("Condition is not a boolean");

            result->condition = condition;
        }

        expectToken(Token::Type::RightParenthesis, iterator, end);

        ++iterator;

        Statement* statement;
        if (!(statement = parseStatement(iterator, end, declarationScopes, result)))
            throw ParseError("Failed to parse the statement");

        result->body = statement;

        if (isToken(Token::Type::Else, iterator, end))
        {
            ++iterator;

            if (!(statement = parseStatement(iterator, end, declarationScopes, result)))
                throw ParseError("Failed to parse the statement");

            result->elseBody = statement;
        }

        return result;
    }

    ForStatement* ASTContext::parseForStatement(std::vector<Token>::const_iterator& iterator,
                                                std::vector<Token>::const_iterator end,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                Construct* parent)
    {
        expectToken(Token::Type::For, iterator, end);

        ++iterator;

        ForStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new ForStatement()));
        result->parent = parent;

        expectToken(Token::Type::LeftParenthesis, iterator, end);

        ++iterator;

        if (isDeclaration(iterator, end, declarationScopes))
        {
            Declaration* declaration;
            if (!(declaration = parseDeclaration(iterator, end, declarationScopes, result)))
                return nullptr;

            if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
                declaration->getDeclarationKind() != Declaration::Kind::Parameter)
                throw ParseError("Expected a variable declaration");

            result->initialization = declaration;

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;
        }
        else if (isToken(Token::Type::Semicolon, iterator, end))
        {
            ++iterator;

            result->initialization = nullptr;
        }
        else
        {
            if (!(result->initialization = parseExpression(iterator, end, declarationScopes, result)))
                return nullptr;

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;
        }

        if (isDeclaration(iterator, end, declarationScopes))
        {
            // TODO: add implicit cast to bool
            Declaration* declaration;
            if (!(declaration = parseDeclaration(iterator, end, declarationScopes, result)))
                return nullptr;

            if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
                declaration->getDeclarationKind() != Declaration::Kind::Parameter)
                throw ParseError("Expected a variable declaration");

            result->condition = declaration;

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;
        }
        else if (isToken(Token::Type::Semicolon, iterator, end))
        {
            ++iterator;

            result->condition = nullptr;
        }
        else
        {
            Expression* condition = parseExpression(iterator, end, declarationScopes, result);

            if (!condition)
                return nullptr;

            if (!isBooleanType(condition->qualifiedType.type))
                throw ParseError("Condition is not a boolean");

            result->condition = condition;

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;
        }

        if (isToken(Token::Type::RightParenthesis, iterator, end))
        {
            ++iterator;

            result->increment = nullptr;
        }
        else
        {
            if (!(result->increment = parseExpression(iterator, end, declarationScopes, result)))
                return nullptr;

            expectToken(Token::Type::RightParenthesis, iterator, end);

            ++iterator;
        }

        if (!(result->body = parseStatement(iterator, end, declarationScopes, result)))
            return nullptr;

        return result;
    }

    SwitchStatement* ASTContext::parseSwitchStatement(std::vector<Token>::const_iterator& iterator,
                                                      std::vector<Token>::const_iterator end,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                                      Construct* parent)
    {
        expectToken(Token::Type::Switch, iterator, end);

        ++iterator;

        SwitchStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new SwitchStatement()));
        result->parent = parent;

        expectToken(Token::Type::LeftParenthesis, iterator, end);

        ++iterator;

        // TODO: add implicit cast to int
        if (isDeclaration(iterator, end, declarationScopes))
        {
            Declaration* declaration;
            if (!(declaration = parseDeclaration(iterator, end, declarationScopes, result)))
                return nullptr;

            if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
                declaration->getDeclarationKind() != Declaration::Kind::Parameter)
                throw ParseError("Expected a variable declaration");

            result->condition = declaration;
        }
        else
        {
            Expression* condition = parseExpression(iterator, end, declarationScopes, result);

            if (!condition)
                throw ParseError("Expected an expression");

            if (!isIntegerType(condition->qualifiedType.type))
                throw ParseError("Statement requires expression of integer type");

            result->condition = condition;
        }

        expectToken(Token::Type::RightParenthesis, iterator, end);

        ++iterator;

        if (!(result->body = parseStatement(iterator, end, declarationScopes, result)))
            return nullptr;

        return result;
    }

    CaseStatement* ASTContext::parseCaseStatement(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  Construct* parent)
    {
        expectToken(Token::Type::Case, iterator, end);

        ++iterator;

        CaseStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new CaseStatement()));
        result->parent = parent;

        Expression* condition = parseExpression(iterator, end, declarationScopes, result);

        if (!condition)
            throw ParseError("Expected an expression");

        if (!isIntegerType(condition->qualifiedType.type))
            throw ParseError("Statement requires expression of integer type");

        if ((condition->qualifiedType.qualifiers & Qualifiers::Const) != Qualifiers::Const)
            throw ParseError("Expression must be constant");

        result->condition = condition;

        expectToken(Token::Type::Colon, iterator, end);

        ++iterator;

        if (!(result->body = parseStatement(iterator, end, declarationScopes, result)))
            return nullptr;

        return result;
    }

    DefaultStatement* ASTContext::parseDefaultStatement(std::vector<Token>::const_iterator& iterator,
                                                        std::vector<Token>::const_iterator end,
                                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                                        Construct* parent)
    {
        expectToken(Token::Type::Default, iterator, end);

        ++iterator;

        DefaultStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new DefaultStatement()));
        result->parent = parent;

        expectToken(Token::Type::Colon, iterator, end);

        ++iterator;

        if (!(result->body = parseStatement(iterator, end, declarationScopes, result)))
            return nullptr;

        return result;
    }

    WhileStatement* ASTContext::parseWhileStatement(std::vector<Token>::const_iterator& iterator,
                                                    std::vector<Token>::const_iterator end,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                                    Construct* parent)
    {
        expectToken(Token::Type::While, iterator, end);

        ++iterator;

        WhileStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new WhileStatement()));
        result->parent = parent;

        expectToken(Token::Type::LeftParenthesis, iterator, end);

        ++iterator;

        if (isDeclaration(iterator, end, declarationScopes))
        {
            // TODO: add implicit cast to bool
            Declaration* declaration;
            if (!(declaration = parseDeclaration(iterator, end, declarationScopes, result)))
                return nullptr;

            if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
                declaration->getDeclarationKind() != Declaration::Kind::Parameter)
                throw ParseError("Expected a variable declaration");

            result->condition = declaration;
        }
        else
        {
            Expression* condition = parseExpression(iterator, end, declarationScopes, result);

            if (!condition)
                return nullptr;

            if (!isBooleanType(condition->qualifiedType.type))
                throw ParseError("Condition is not a boolean");

            result->condition = condition;
        }

        expectToken(Token::Type::RightParenthesis, iterator, end);

        ++iterator;

        if (!(result->body = parseStatement(iterator, end, declarationScopes, result)))
            return nullptr;

        return result;
    }

    DoStatement* ASTContext::parseDoStatement(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent)
    {
        expectToken(Token::Type::Do, iterator, end);

        ++iterator;

        DoStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new DoStatement()));
        result->parent = parent;

        if (!(result->body = parseStatement(iterator, end, declarationScopes, result)))
            return nullptr;

        expectToken(Token::Type::While, iterator, end);

        ++iterator;

        expectToken(Token::Type::LeftParenthesis, iterator, end);

        ++iterator;

        Expression* condition = parseExpression(iterator, end, declarationScopes, result);

        if (!condition)
            return nullptr;

        if (!isBooleanType(condition->qualifiedType.type))
            throw ParseError("Condition is not a boolean");

        result->condition = condition;

        expectToken(Token::Type::RightParenthesis, iterator, end);

        ++iterator;

        expectToken(Token::Type::Semicolon, iterator, end);

        ++iterator;

        return result;
    }

    Expression* ASTContext::parsePrimaryExpression(std::vector<Token>::const_iterator& iterator,
                                                   std::vector<Token>::const_iterator end,
                                                   std::vector<std::vector<Declaration*>>& declarationScopes,
                                                   Construct* parent)
    {
        if (isToken(Token::Type::IntLiteral, iterator, end))
        {
            IntegerLiteralExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new IntegerLiteralExpression()));
            result->parent = parent;
            result->qualifiedType.type = intType;
            result->qualifiedType.qualifiers = Qualifiers::Const;
            result->category = Expression::Category::Rvalue;
            result->value = strtoll(iterator->value.c_str(), nullptr, 0);

            ++iterator;

            return result;
        }
        else if (isToken(Token::Type::FloatLiteral, iterator, end))
        {
            FloatingPointLiteralExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new FloatingPointLiteralExpression()));
            result->parent = parent;
            result->qualifiedType.type = floatType;
            result->qualifiedType.qualifiers = Qualifiers::Const;
            result->category = Expression::Category::Rvalue;
            result->value = strtod(iterator->value.c_str(), nullptr);

            ++iterator;

            return result;
        }
        else if (isToken({Token::Type::DoubleLiteral, Token::Type::Double}, iterator, end))
        {
            throw ParseError("Double precision floating point numbers are not supported");
        }
        else if (isToken(Token::Type::StringLiteral, iterator, end))
        {
            StringLiteralExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new StringLiteralExpression()));
            result->parent = parent;
            result->qualifiedType.type = stringType;
            result->qualifiedType.qualifiers = Qualifiers::Const;
            result->category = Expression::Category::Rvalue;
            result->value = iterator->value;

            ++iterator;

            return result;
        }
        else if (isToken({Token::Type::True, Token::Type::False}, iterator, end))
        {
            BooleanLiteralExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new BooleanLiteralExpression()));
            result->parent = parent;
            result->qualifiedType.type = boolType;
            result->qualifiedType.qualifiers = Qualifiers::Const;
            result->category = Expression::Category::Rvalue;
            result->value = (iterator->type == Token::Type::True);

            ++iterator;

            return result;
        }
        else if (isToken({Token::Type::Bool, Token::Type::Int, Token::Type::Float}, iterator, end))
        {
            // TODO: parse type and fix precedence
            CastExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new CastExpression(CastExpression::Kind::Functional)));

            if (isToken(Token::Type::Bool, iterator, end)) result->qualifiedType.type = boolType;
            else if(isToken(Token::Type::Int, iterator, end)) result->qualifiedType.type = intType;
            else if(isToken(Token::Type::Float, iterator, end)) result->qualifiedType.type = floatType;

            ++iterator;

            expectToken(Token::Type::LeftParenthesis, iterator, end);

            ++iterator;

            result->expression = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, result);

            expectToken(Token::Type::RightParenthesis, iterator, end);

            ++iterator;

            return result;
        }
        else if (isToken(Token::Type::LeftBrace, iterator, end))
        {
            ++iterator;

            InitializerListExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new InitializerListExpression()));

            QualifiedType qualifiedType;

            for (;;)
            {
                Expression* expression;
                if (!(expression = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, result)))
                    return nullptr;

                if (!qualifiedType.type)
                    qualifiedType.type = expression->qualifiedType.type;
                else
                {
                    if (qualifiedType.type != expression->qualifiedType.type)
                    {
                        // TODO: implement type narrowing
                        throw ParseError("Expression type does not match previous expressions in initializer list");
                    }
                }

                result->expressions.push_back(expression);

                if (!isToken(Token::Type::Comma, iterator, end))
                    break;

                ++iterator;
            }

            expectToken(Token::Type::RightBrace, iterator, end);

            result->qualifiedType.type = getArrayType(qualifiedType, static_cast<uint32_t>(result->expressions.size()));

            ++iterator;

            return result;
        }
        else if (isToken(Token::Type::Identifier, iterator, end))
        {
            std::string name = iterator->value;

            ++iterator;

            if (isToken(Token::Type::LeftParenthesis, iterator, end))
            {
                ++iterator;

                Type* type;

                if ((type = findType(name, declarationScopes)))
                {
                    TemporaryObjectExpression* result;
                    constructs.push_back(std::unique_ptr<Construct>(result = new TemporaryObjectExpression()));
                    result->parent = parent;
                    result->qualifiedType.type = type;
                    result->qualifiedType.qualifiers = Qualifiers::Const;
                    result->category = Expression::Category::Rvalue;

                    std::vector<QualifiedType> parameters;

                    if (isToken(Token::Type::RightParenthesis, iterator, end)) // no arguments
                        ++iterator;
                    else
                    {
                        for (;;)
                        {
                            Expression* parameter;

                            if (!(parameter = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, result)))
                                return nullptr;

                            result->parameters.push_back(parameter);
                            parameters.push_back(parameter->qualifiedType);

                            if (isToken(Token::Type::Comma, iterator, end))
                                ++iterator;
                            else
                                break;
                        }

                        expectToken(Token::Type::RightParenthesis, iterator, end);

                        ++iterator;
                    }

                    if (type->getTypeKind() != Type::Kind::Struct)
                        throw ParseError("Expected a struct type");

                    StructType* structType = static_cast<StructType*>(type);

                    if (!(result->constructorDeclaration = structType->findConstructorDeclaration(parameters)))
                        throw ParseError("No matching constructor found");

                    return result;
                }
                else
                {
                    CallExpression* result;
                    constructs.push_back(std::unique_ptr<Construct>(result = new CallExpression()));
                    result->parent = parent;

                    std::vector<QualifiedType> arguments;

                    if (isToken(Token::Type::RightParenthesis, iterator, end)) // no arguments
                        ++iterator;
                    else
                    {
                        for (;;)
                        {
                            Expression* argument;

                            if (!(argument = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, result)))
                                return nullptr;

                            result->arguments.push_back(argument);
                            arguments.push_back(argument->qualifiedType);

                            if (isToken(Token::Type::Comma, iterator, end))
                                ++iterator;
                            else
                                break;
                        }

                        expectToken(Token::Type::RightParenthesis, iterator, end);

                        ++iterator;
                    }

                    DeclarationReferenceExpression* declRefExpression;
                    constructs.push_back(std::unique_ptr<Construct>(declRefExpression = new DeclarationReferenceExpression()));
                    declRefExpression->parent = result;

                    FunctionDeclaration* functionDeclaration = resolveFunctionDeclaration(name, declarationScopes, arguments);

                    if (!functionDeclaration)
                        throw ParseError("Invalid function reference: " + name);

                    declRefExpression->declaration = functionDeclaration;
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
                DeclarationReferenceExpression* result;
                constructs.push_back(std::unique_ptr<Construct>(result = new DeclarationReferenceExpression()));
                result->parent = parent;
                result->declaration = findDeclaration(name, declarationScopes);

                if (!result->declaration)
                    throw ParseError("Invalid declaration reference: " + name);

                switch (result->declaration->getDeclarationKind())
                {
                    case Declaration::Kind::Type:
                    {
                        TypeDeclaration* typeDeclaration = static_cast<TypeDeclaration*>(result->declaration);
                        result->qualifiedType.type = typeDeclaration->type;
                        result->category = Expression::Category::Rvalue;
                        break;
                    }
                    case Declaration::Kind::Variable:
                    {
                        VariableDeclaration* variableDeclaration = static_cast<VariableDeclaration*>(result->declaration);
                        result->qualifiedType = variableDeclaration->qualifiedType;
                        result->category = Expression::Category::Lvalue;
                        break;
                    }
                    case Declaration::Kind::Parameter:
                    {
                        ParameterDeclaration* parameterDeclaration = static_cast<ParameterDeclaration*>(result->declaration);
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
        else if (isToken(Token::Type::LeftParenthesis, iterator, end))
        {
            ++iterator;

            if (isType(iterator, end, declarationScopes))
            {
                CastExpression* result = new CastExpression(CastExpression::Kind::CStyle);
                constructs.push_back(std::unique_ptr<Construct>(result));
                result->parent = parent;

                // TODO: parse qualifiers
                result->qualifiedType.type = parseType(iterator, end, declarationScopes);

                expectToken(Token::Type::RightParenthesis, iterator, end);
                ++iterator;

                result->expression = parseExpression(iterator, end, declarationScopes, result);
                result->category = Expression::Category::Rvalue;

                return result;
            }
            else
            {
                ParenExpression* result;
                constructs.push_back(std::unique_ptr<Construct>(result = new ParenExpression()));
                result->parent = parent;

                if (!(result->expression = parseExpression(iterator, end, declarationScopes, result)))
                    return nullptr;

                expectToken(Token::Type::RightParenthesis, iterator, end);

                ++iterator;

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

            CastExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new CastExpression(castKind)));
            result->parent = parent;

            ++iterator;

            expectToken(Token::Type::LessThan, iterator, end);
            ++iterator;

            // TODO: parse qualifiers
            result->qualifiedType.type = parseType(iterator, end, declarationScopes);

            expectToken(Token::Type::GreaterThan, iterator, end);
            ++iterator;

            expectToken(Token::Type::LeftParenthesis, iterator, end);
            ++iterator;

            if (!(result->expression = parseExpression(iterator, end, declarationScopes, result)))
                return nullptr;

            expectToken(Token::Type::RightParenthesis, iterator, end);
            ++iterator;

            result->category = Expression::Category::Rvalue;

            return result;
        }
        else if (isToken(Token::Type::This, iterator, end))
        {
            // TODO: implement
            throw ParseError("Expression \"this\" is not supported");
        }
        else
            throw ParseError("Expected an expression");

        return nullptr;
    }

    Expression* ASTContext::parsePostfixExpression(std::vector<Token>::const_iterator& iterator,
                                                   std::vector<Token>::const_iterator end,
                                                   std::vector<std::vector<Declaration*>>& declarationScopes,
                                                   Construct* parent)
    {
        Expression* result;
        if (!(result = parsePrimaryExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

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

            UnaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new UnaryOperatorExpression(operatorKind)));
            expression->parent = parent;
            expression->expression = result;

            expression->qualifiedType.type = result->qualifiedType.type;
            expression->category = Expression::Category::Lvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseSubscriptExpression(std::vector<Token>::const_iterator& iterator,
                                                     std::vector<Token>::const_iterator end,
                                                     std::vector<std::vector<Declaration*>>& declarationScopes,
                                                     Construct* parent)
    {
        Expression* result;
        if (!(result = parsePostfixExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken(Token::Type::LeftBracket, iterator, end))
        {
            ++iterator;

            if (result->qualifiedType.type->getTypeKind() == Type::Kind::Array)
            {
                ArraySubscriptExpression* expression;
                constructs.push_back(std::unique_ptr<Construct>(expression = new ArraySubscriptExpression()));
                expression->parent = parent;
                expression->expression = result;

                if (!(expression->subscript = parseExpression(iterator, end, declarationScopes, expression)))
                    return nullptr;

                if (!isIntegerType(expression->subscript->qualifiedType.type))
                    throw ParseError("Subscript is not an integer");

                expectToken(Token::Type::RightBracket, iterator, end);

                ++iterator;

                auto arrayType = static_cast<const ArrayType*>(result->qualifiedType.type);

                expression->qualifiedType = arrayType->elementType;
                expression->category = result->category;

                result->parent = expression;
                result = expression;
            }
            else if (result->qualifiedType.type->getTypeKind() == Type::Kind::Vector ||
                     result->qualifiedType.type->getTypeKind() == Type::Kind::Matrix)
            {
                const auto operatorKind = BinaryOperatorExpression::Kind::Subscript;

                BinaryOperatorExpression* expression;
                constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
                expression->parent = parent;
                expression->leftExpression = result;

                if (!(expression->rightExpression = parseExpression(iterator, end, declarationScopes, expression)))
                    return nullptr;

                if (!isIntegerType(expression->rightExpression->qualifiedType.type))
                    throw ParseError("Subscript is not an integer");

                expectToken(Token::Type::RightBracket, iterator, end);

                ++iterator;

                if (result->qualifiedType.type->getTypeKind() == Type::Kind::Vector)
                {
                    auto vectorType = static_cast<const VectorType*>(result->qualifiedType.type);
                    expression->qualifiedType.type = vectorType->componentType;
                }
                else if (result->qualifiedType.type->getTypeKind() == Type::Kind::Matrix)
                {
                    auto matrixType = static_cast<const MatrixType*>(result->qualifiedType.type);

                    auto vectorTypeIterator = vectorTypes.find(std::make_pair(matrixType->componentType, static_cast<uint8_t>(matrixType->columnCount)));

                    if (vectorTypeIterator == vectorTypes.end())
                        throw ParseError("Invalid vector type");

                    expression->qualifiedType.type = vectorTypeIterator->second;
                }

                expression->category = expression->rightExpression->category;

                result->parent = expression;
                result = expression;
            }
            else
                throw ParseError("Subscript value is not an array");
        }

        return result;
    }

    namespace
    {
        constexpr uint8_t charToComponent(char c)
        {
            return (c == 'x' || c == 'r') ? 0 :
                (c == 'y' || c == 'g') ? 1 :
                (c == 'z' || c == 'b') ? 2 :
                (c == 'w' || c == 'a') ? 3 :
                throw ParseError("Invalid component");
        }
    }

    Expression* ASTContext::parseMemberExpression(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  Construct* parent)
    {
        Expression* result;
        if (!(result = parseSubscriptExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken({Token::Type::Dot, Token::Type::Arrow}, iterator, end))
        {
            if (isToken(Token::Type::Arrow, iterator, end))
                throw ParseError("Pointer member access is not supported");

            ++iterator;

            if (result->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                throw ParseError("Expression has a void type");

            if (result->qualifiedType.type->getTypeKind() == Type::Kind::Struct)
            {
                MemberExpression* expression;
                constructs.push_back(std::unique_ptr<Construct>(expression = new MemberExpression()));
                expression->parent = parent;
                expression->expression = result;

                auto structType = static_cast<const StructType*>(result->qualifiedType.type);

                expectToken(Token::Type::Identifier, iterator, end);

                Declaration* memberDeclaration = structType->findMemberDeclaration(iterator->value);

                if (!memberDeclaration)
                {
                    throw ParseError("Structure " + structType->name +  " has no member " + iterator->value);
                    return nullptr;
                }

                if (memberDeclaration->getDeclarationKind() != Declaration::Kind::Field)
                    throw ParseError(iterator->value + " is not a field");

                expression->fieldDeclaration = static_cast<FieldDeclaration*>(memberDeclaration);

                ++iterator;

                expression->qualifiedType = expression->fieldDeclaration->qualifiedType;
                if ((result->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                    expression->qualifiedType.qualifiers |= Qualifiers::Const;
                expression->category = result->category;

                result->parent = expression;
                result = expression;
            }
            else if (result->qualifiedType.type->getTypeKind() == Type::Kind::Vector)
            {
                VectorElementExpression* expression;
                constructs.push_back(std::unique_ptr<Construct>(expression = new VectorElementExpression()));
                expression->parent = parent;

                std::vector<uint8_t> components;
                std::set<uint8_t> componentSet;

                expression->category = result->category;

                for (const char c : iterator->value)
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

                ++iterator;

                auto vectorType = static_cast<const VectorType*>(result->qualifiedType.type);

                auto resultTypeIterator = vectorTypes.find(std::make_pair(vectorType->componentType,
                                                                          static_cast<uint8_t>(components.size())));

                if (resultTypeIterator == vectorTypes.end())
                    throw ParseError("Invalid swizzle");

                expression->qualifiedType.type = resultTypeIterator->second;

                for (uint8_t component : components)
                    if (component >= vectorType->componentCount)
                        throw ParseError("Invalid swizzle");

                result->parent = expression;
                result = expression;
            }
            else
                throw ParseError(result->qualifiedType.type->name + " is not a structure");
        }

        return result;
    }

    Expression* ASTContext::parsePrefixExpression(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  Construct* parent)
    {
        if (isToken({Token::Type::Increment, Token::Type::Decrement}, iterator, end))
        {
            const auto operatorKind =
                (iterator->type == Token::Type::Increment) ? UnaryOperatorExpression::Kind::PrefixIncrement :
                (iterator->type == Token::Type::Decrement) ? UnaryOperatorExpression::Kind::PrefixDecrement :
                throw ParseError("Invalid operator");

            ++iterator;

            UnaryOperatorExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new UnaryOperatorExpression(operatorKind)));
            result->parent = parent;

            if (!(result->expression = parseMemberExpression(iterator, end, declarationScopes, result)))
                return nullptr;

            if (result->expression->category != Expression::Category::Lvalue)
                throw ParseError("Expression is not assignable");

            if ((result->expression->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                throw ParseError("Cannot assign to const variable");

            if (result->expression->qualifiedType.type->getTypeKind() != Type::Kind::Scalar)
                throw ParseError("Parameter of the prefix operator must be a number");

            result->qualifiedType.type = result->expression->qualifiedType.type;
            result->category = Expression::Category::Rvalue;

            return result;
        }
        else
        {
            Expression* result;
            if (!(result = parseMemberExpression(iterator, end, declarationScopes, parent)))
                return nullptr;

            return result;
        }
    }

    Expression* ASTContext::parseSignExpression(std::vector<Token>::const_iterator& iterator,
                                                std::vector<Token>::const_iterator end,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                Construct* parent)
    {
        if (isToken({Token::Type::Plus, Token::Type::Minus}, iterator, end))
        {
            const auto operatorKind =
                (iterator->type == Token::Type::Plus) ? UnaryOperatorExpression::Kind::Positive :
                (iterator->type == Token::Type::Minus) ? UnaryOperatorExpression::Kind::Negative :
                throw ParseError("Invalid operator");

            UnaryOperatorExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new UnaryOperatorExpression(operatorKind)));
            result->parent = parent;

            ++iterator;

            if (!(result->expression = parsePrefixExpression(iterator, end, declarationScopes, result)))
                return nullptr;

            if (result->expression->qualifiedType.type->getTypeKind() != Type::Kind::Scalar)
                throw ParseError("Parameter of the sign operator must be a number");

            result->qualifiedType.type = result->expression->qualifiedType.type;
            result->category = Expression::Category::Rvalue;

            return result;
        }
        else
        {
            Expression* result;
            if (!(result = parsePrefixExpression(iterator, end, declarationScopes, parent)))
                return nullptr;

            return result;
        }
    }

    Expression* ASTContext::parseNotExpression(std::vector<Token>::const_iterator& iterator,
                                               std::vector<Token>::const_iterator end,
                                               std::vector<std::vector<Declaration*>>& declarationScopes,
                                               Construct* parent)
    {
        if (isToken(Token::Type::Not, iterator, end))
        {
            const auto operatorKind = UnaryOperatorExpression::Kind::Negation;

            UnaryOperatorExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new UnaryOperatorExpression(operatorKind)));
            result->parent = parent;

            ++iterator;

            if (!(result->expression = parseExpression(iterator, end, declarationScopes, result)))
                return nullptr;

            if (!isBooleanType(result->expression->qualifiedType.type))
                throw ParseError("Expression is not a boolean");

            result->qualifiedType.type = boolType;
            result->category = Expression::Category::Rvalue;

            return result;
        }
        else
        {
            Expression* result;
            if (!(result = parseSignExpression(iterator, end, declarationScopes, parent)))
                return nullptr;
            return result;
        }
    }

    Expression* ASTContext::parseSizeofExpression(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  Construct* parent)
    {
        if (isToken(Token::Type::Sizeof, iterator, end))
        {
            SizeofExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new SizeofExpression()));
            result->parent = parent;

            ++iterator;

            expectToken(Token::Type::LeftParenthesis, iterator, end);
            ++iterator;

            if (iterator->type == Token::Type::Void)
                throw ParseError("Parameter of sizeof can not be void");

            if (isType(iterator, end, declarationScopes))
            {
                if (!(result->type = parseType(iterator, end, declarationScopes)))
                    return nullptr;
            }
            else
            {
                if (!(result->expression = parseExpression(iterator, end, declarationScopes, result)))
                    return nullptr;
            }

            expectToken(Token::Type::RightParenthesis, iterator, end);
            ++iterator;

            result->qualifiedType.type = unsignedIntType;
            result->category = Expression::Category::Rvalue;

            return result;
        }
        else
        {
            Expression* result;
            if (!(result = parseNotExpression(iterator, end, declarationScopes, parent)))
                return nullptr;
            return result;
        }
    }

    Expression* ASTContext::parseMultiplicationExpression(std::vector<Token>::const_iterator& iterator,
                                                          std::vector<Token>::const_iterator end,
                                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                                          Construct* parent)
    {
        Expression* result;
        if (!(result = parseSizeofExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken({Token::Type::Multiply, Token::Type::Divide}, iterator, end))
        {
            const auto operatorKind =
                (iterator->type == Token::Type::Multiply) ? BinaryOperatorExpression::Kind::Multiplication :
                (iterator->type == Token::Type::Divide) ? BinaryOperatorExpression::Kind::Division :
                throw ParseError("Invalid operator");

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->parent = parent;


            expression->leftExpression = result;

            ++iterator;

            if (!(expression->rightExpression = parseSizeofExpression(iterator, end, declarationScopes, expression)))
                return nullptr;

            expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseAdditionExpression(std::vector<Token>::const_iterator& iterator,
                                                    std::vector<Token>::const_iterator end,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                                    Construct* parent)
    {
        Expression* result;
        if (!(result = parseMultiplicationExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken({Token::Type::Plus, Token::Type::Minus}, iterator, end))
        {
            const auto operatorKind =
                (iterator->type == Token::Type::Plus) ? BinaryOperatorExpression::Kind::Addition :
                (iterator->type == Token::Type::Minus) ? BinaryOperatorExpression::Kind::Subtraction :
                throw ParseError("Invalid operator");

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->parent = parent;

            expression->leftExpression = result;

            ++iterator;

            if (!(expression->rightExpression = parseMultiplicationExpression(iterator, end, declarationScopes, expression)))
                return nullptr;

            expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseLessThanExpression(std::vector<Token>::const_iterator& iterator,
                                                    std::vector<Token>::const_iterator end,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                                    Construct* parent)
    {
        Expression* result;
        if (!(result = parseAdditionExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken({Token::Type::LessThan, Token::Type::LessThanEqual}, iterator, end))
        {
            const auto operatorKind =
                (iterator->type == Token::Type::LessThan) ? BinaryOperatorExpression::Kind::LessThan :
                (iterator->type == Token::Type::LessThanEqual) ? BinaryOperatorExpression::Kind::LessThanEqual :
                throw ParseError("Invalid operator");

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->parent = parent;

            ++iterator;

            expression->leftExpression = result;

            if (!(expression->rightExpression = parseAdditionExpression(iterator, end, declarationScopes, expression)))
                return nullptr;

            expression->qualifiedType.type = boolType;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseGreaterThanExpression(std::vector<Token>::const_iterator& iterator,
                                                       std::vector<Token>::const_iterator end,
                                                       std::vector<std::vector<Declaration*>>& declarationScopes,
                                                       Construct* parent)
    {
        Expression* result;
        if (!(result = parseLessThanExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken({Token::Type::GreaterThan, Token::Type::GreaterThanEqual}, iterator, end))
        {
            const auto operatorKind =
                (iterator->type == Token::Type::GreaterThan) ? BinaryOperatorExpression::Kind::GreaterThan :
                (iterator->type == Token::Type::GreaterThanEqual) ? BinaryOperatorExpression::Kind::GraterThanEqual :
                throw ParseError("Invalid operator");

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->parent = parent;

            ++iterator;

            expression->leftExpression = result;

            if (!(expression->rightExpression = parseLessThanExpression(iterator, end, declarationScopes, expression)))
                return nullptr;

            expression->qualifiedType.type = boolType;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseEqualityExpression(std::vector<Token>::const_iterator& iterator,
                                                    std::vector<Token>::const_iterator end,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                                    Construct* parent)
    {
        Expression* result;
        if (!(result = parseGreaterThanExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken({Token::Type::Equal, Token::Type::NotEq}, iterator, end))
        {
            const auto operatorKind =
                (iterator->type == Token::Type::Equal) ? BinaryOperatorExpression::Kind::Equality :
                (iterator->type == Token::Type::NotEq) ? BinaryOperatorExpression::Kind::Inequality :
                throw ParseError("Invalid operator");

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->parent = parent;

            ++iterator;

            expression->leftExpression = result;

            if (!(expression->rightExpression = parseGreaterThanExpression(iterator, end, declarationScopes, expression)))
                return nullptr;

            expression->qualifiedType.type = boolType;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseLogicalAndExpression(std::vector<Token>::const_iterator& iterator,
                                                      std::vector<Token>::const_iterator end,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                                      Construct* parent)
    {
        Expression* result;
        if (!(result = parseEqualityExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken(Token::Type::And, iterator, end))
        {
            ++iterator;

            const auto operatorKind = BinaryOperatorExpression::Kind::And;

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->parent = parent;

            expression->leftExpression = result;

            if (!(expression->rightExpression = parseEqualityExpression(iterator, end, declarationScopes, expression)))
                return nullptr;

            // TODO: check if both sides ar scalar
            expression->qualifiedType.type = boolType;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseLogicalOrExpression(std::vector<Token>::const_iterator& iterator,
                                                     std::vector<Token>::const_iterator end,
                                                     std::vector<std::vector<Declaration*>>& declarationScopes,
                                                     Construct* parent)
    {
        Expression* result;
        if (!(result = parseLogicalAndExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken(Token::Type::Or, iterator, end))
        {
            ++iterator;

            const auto operatorKind = BinaryOperatorExpression::Kind::Or;

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->parent = parent;

            expression->leftExpression = result;

            if (!(expression->rightExpression = parseLogicalAndExpression(iterator, end, declarationScopes, expression)))
                return nullptr;

            // TODO: check if both sides ar scalar
            expression->qualifiedType.type = boolType;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseTernaryExpression(std::vector<Token>::const_iterator& iterator,
                                                   std::vector<Token>::const_iterator end,
                                                   std::vector<std::vector<Declaration*>>& declarationScopes,
                                                   Construct* parent)
    {
        Expression* result;
        if (!(result = parseLogicalOrExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken(Token::Type::Conditional, iterator, end))
        {
            ++iterator;

            TernaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new TernaryOperatorExpression()));
            expression->parent = parent;

            if (!isBooleanType(result->qualifiedType.type))
                throw ParseError("Condition is not a boolean");

            expression->condition = result;

            if (expression->condition->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                throw ParseError("Ternary expression with a void condition");

            if (!(expression->leftExpression = parseTernaryExpression(iterator, end, declarationScopes, expression)))
                return nullptr;

            expectToken(Token::Type::Colon, iterator, end);

            ++iterator;

            if (!(expression->rightExpression = parseTernaryExpression(iterator, end, declarationScopes, expression)))
                return nullptr;

            // TODO: fix this
            expression->qualifiedType = expression->leftExpression->qualifiedType;
            expression->category = (expression->leftExpression->category == Expression::Category::Lvalue &&
                                    expression->rightExpression->category == Expression::Category::Lvalue) ?
                                    Expression::Category::Lvalue : Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                                      std::vector<Token>::const_iterator end,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                                      Construct* parent)
    {
        Expression* result;
        if (!(result = parseTernaryExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken(Token::Type::Assignment, iterator, end))
        {
            ++iterator;

            const auto operatorKind = BinaryOperatorExpression::Kind::Assignment;

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->parent = parent;
            expression->leftExpression = result;

            if (expression->leftExpression->category != Expression::Category::Lvalue)
                throw ParseError("Expression is not assignable");

            if ((expression->leftExpression->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                throw ParseError("Cannot assign to const variable");

            if (!(expression->rightExpression = parseTernaryExpression(iterator, end, declarationScopes, expression)))
                return nullptr;

            expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseAdditionAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                                              std::vector<Token>::const_iterator end,
                                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                                              Construct* parent)
    {
        Expression* result;
        if (!(result = parseAssignmentExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken({Token::Type::PlusAssignment, Token::Type::MinusAssignment}, iterator, end))
        {
            const auto operatorKind =
                (iterator->type == Token::Type::PlusAssignment) ? BinaryOperatorExpression::Kind::AdditionAssignment :
                (iterator->type == Token::Type::MinusAssignment) ? BinaryOperatorExpression::Kind::SubtractAssignment :
                throw ParseError("Invalid operator");

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->parent = parent;

            ++iterator;

            expression->leftExpression = result;

            if (expression->leftExpression->category != Expression::Category::Lvalue)
                throw ParseError("Expression is not assignable");

            if ((expression->leftExpression->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                throw ParseError("Cannot assign to const variable");

            if (!(expression->rightExpression = parseAssignmentExpression(iterator, end, declarationScopes, expression)))
                return nullptr;

            expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;
            expression->category = Expression::Category::Lvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseMultiplicationAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                                                    std::vector<Token>::const_iterator end,
                                                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                                                    Construct* parent)
    {
        Expression* result;
        if (!(result = parseAdditionAssignmentExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken({Token::Type::Multiply, Token::Type::DivideAssignment}, iterator, end))
        {
            const auto operatorKind =
                (iterator->type == Token::Type::Multiply) ? BinaryOperatorExpression::Kind::MultiplicationAssignment :
                (iterator->type == Token::Type::DivideAssignment) ? BinaryOperatorExpression::Kind::DivisionAssignment :
                throw ParseError("Invalid operator");

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->parent = parent;

            ++iterator;

            expression->leftExpression = result;

            if (expression->leftExpression->category != Expression::Category::Lvalue)
                throw ParseError("Expression is not assignable");

            if ((expression->leftExpression->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                throw ParseError("Cannot assign to const variable");

            std::unique_ptr<Construct> right;
            if (!(expression->rightExpression = parseAdditionAssignmentExpression(iterator, end, declarationScopes, expression)))
                return nullptr;

            expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;
            expression->category = Expression::Category::Lvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseCommaExpression(std::vector<Token>::const_iterator& iterator,
                                                 std::vector<Token>::const_iterator end,
                                                 std::vector<std::vector<Declaration*>>& declarationScopes,
                                                 Construct* parent)
    {
        Expression* result;
        if (!(result = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        while (isToken(Token::Type::Comma, iterator, end))
        {
            const auto operatorKind = BinaryOperatorExpression::Kind::Comma;

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->parent = parent;
            expression->leftExpression = result;

            ++iterator;

            std::unique_ptr<Construct> right;
            if (!(expression->rightExpression = parseAdditionAssignmentExpression(iterator, end, declarationScopes, expression)))
                return nullptr;

            expression->qualifiedType.type = expression->rightExpression->qualifiedType.type;
            expression->category = expression->rightExpression->category;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    void ASTContext::dump() const
    {
        for (Declaration* declaration : declarations)
            dumpConstruct(declaration);
    }
}

