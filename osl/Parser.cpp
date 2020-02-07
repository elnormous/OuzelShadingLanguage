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

        for (size_t components = 2; components <= 4; ++components)
        {
            std::string name = "float";
            name.push_back('0' + components);
            addVectorType(name, floatType, components, declarationScopes);
        }

        for (size_t components = 2; components <= 4; ++components)
        {
            std::string name = "float";
            name.push_back('0' + components);
            name.push_back('x');
            name.push_back('0' + components);

            addMatrixType(name, floatType, components, components, declarationScopes);
        }

        stringType = addStructType("string", 8, declarationScopes);
        StructType* texture2DType = addStructType("Texture2D", 0, declarationScopes);

        //addBuiltinFunctionDeclaration("sample", float4Type, {texture2DType, float2Type}, declarationScopes);

        StructType* texture2DMSType = addStructType("Texture2DMS", 0, declarationScopes);

        //addBuiltinFunctionDeclaration("load", float4Type, {texture2DMSType, float2Type}, declarationScopes);

        for (auto iterator = tokens.begin(); iterator != tokens.end();)
        {
            Declaration* declaration = parseTopLevelDeclaration(iterator, tokens.end(), declarationScopes);
            declarations.push_back(declaration);
        }
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

                    auto callableDeclaration = static_cast<CallableDeclaration*>(*declarationIterator);

                    if (callableDeclaration->getCallableDeclarationKind() != CallableDeclaration::Kind::Function) return nullptr;

                    auto functionDeclaration = static_cast<FunctionDeclaration*>(callableDeclaration->firstDeclaration);

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

            FunctionDeclaration* result = nullptr;

            for (auto viableFunctionDeclaration : viableFunctionDeclarations)
            {
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
            }

            return result;
        }
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
        auto declaration = parseDeclaration(iterator, end, declarationScopes);

        if (declaration->getDeclarationKind() == Declaration::Kind::Callable)
        {
            auto callableDeclaration = static_cast<CallableDeclaration*>(declaration);

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
            else if (isToken(Token::Type::LeftBracket, iterator, end) &&
                     isToken(Token::Type::LeftBracket, iterator + 1, end))
            {
                ++iterator;
                ++iterator;
                if (iterator == end)
                    throw ParseError("Unexpected end of file");

                if (iterator->value == "fragment")
                {
                    if (result.program != Program::None)
                        throw ParseError("Single function can not have multiple program attributes");

                    result.program = Program::Fragment;
                }
                else if (iterator->value == "vertex")
                {
                    if (result.program != Program::None)
                        throw ParseError("Single function can not have multiple program attributes");

                    result.program = Program::Vertex;
                }
                else if (iterator->value == "binormal")
                {
                    if (result.semantic != Semantic::None)
                        throw ParseError("Single variable can not have multiple semantic attributes");

                    result.semantic = Semantic::Binormal;
                }
                else if (iterator->value == "blend_indices")
                {
                    if (result.semantic != Semantic::None)
                        throw ParseError("Single variable can not have multiple semantic attributes");

                    result.semantic = Semantic::BlendIndices;
                }
                else if (iterator->value == "blend_weight")
                {
                    if (result.semantic != Semantic::None)
                        throw ParseError("Single variable can not have multiple semantic attributes");

                    result.semantic = Semantic::BlendWeight;
                }
                else if (iterator->value == "color")
                {
                    if (result.semantic != Semantic::None)
                        throw ParseError("Single variable can not have multiple semantic attributes");

                    result.semantic = Semantic::Color;
                }
                else if (iterator->value == "normal")
                {
                    if (result.semantic != Semantic::None)
                        throw ParseError("Single variable can not have multiple semantic attributes");

                    result.semantic = Semantic::Normal;
                }
                else if (iterator->value == "position")
                {
                    if (result.semantic != Semantic::None)
                        throw ParseError("Single variable can not have multiple semantic attributes");

                    result.semantic = Semantic::Position;
                }
                else if (iterator->value == "position_transformed")
                {
                    if (result.semantic != Semantic::None)
                        throw ParseError("Single variable can not have multiple semantic attributes");

                    result.semantic = Semantic::PositionTransformed;
                }
                else if (iterator->value == "point_size")
                {
                    if (result.semantic != Semantic::None)
                        throw ParseError("Single variable can not have multiple semantic attributes");

                    result.semantic = Semantic::PointSize;
                }
                else if (iterator->value == "tangent")
                {
                    if (result.semantic != Semantic::None)
                        throw ParseError("Single variable can not have multiple semantic attributes");

                    result.semantic = Semantic::Tangent;
                }
                else if (iterator->value == "texture_coordinates")
                {
                    if (result.semantic != Semantic::None)
                        throw ParseError("Single variable can not have multiple semantic attributes");

                    result.semantic = Semantic::TextureCoordinates;
                }
                else
                    throw ParseError("Invalid attribute " + iterator->value);

                ++iterator;
                
                if (isToken(Token::Type::LeftParenthesis, iterator, end))
                {
                    ++iterator;

                    // TODO: check if attribute contains semantic
                    expectToken(Token::Type::IntLiteral, iterator, end);
                    result.semanticIndex = std::stoul(iterator->value);
                    ++iterator;

                    expectToken(Token::Type::RightParenthesis, iterator, end);
                    ++iterator;
                }

                expectToken(Token::Type::RightBracket, iterator, end);
                ++iterator;

                expectToken(Token::Type::RightBracket, iterator, end);
                ++iterator;
            }
            else break;
        }

        return result;
    }

    Declaration* ASTContext::parseDeclaration(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        if (isToken(Token::Type::Semicolon, iterator, end))
        {
            Declaration* declaration;
            constructs.push_back(std::unique_ptr<Construct>(declaration = new Declaration(Declaration::Kind::Empty)));
            declarationScopes.back().push_back(declaration);

            return declaration;
        }
        else if (isToken(Token::Type::Struct, iterator, end))
        {
            ++iterator;

            return parseStructTypeDeclaration(iterator, end, declarationScopes);
        }
        /*else if (isToken(Token::Type::KEYWORD_TYPEDEF, iterator, end))
        {
            ++iterator;

            return parseTypeDefinitionDeclaration(iterator, end, declarationScopes);
        }*/
        else
        {
            ASTContext::Specifiers specifiers = parseSpecifiers(iterator, end);

            QualifiedType qualifiedType;
            qualifiedType.qualifiers |= specifiers.qualifiers;

            bool isExtern = specifiers.isExtern;
            bool isInline = specifiers.isInline;
            Program program = specifiers.program;
            Semantic semantic = specifiers.semantic;
            size_t semanticIndex = specifiers.semanticIndex;

            qualifiedType.type = parseType(iterator, end, declarationScopes);

            if (qualifiedType.type->getTypeKind() == Type::Kind::Struct)
            {
                auto structType = static_cast<const StructType*>(qualifiedType.type);
                if (!structType->declaration->definition)
                    throw ParseError("Incomplete type " + qualifiedType.type->name);
            }

            specifiers = parseSpecifiers(iterator, end);

            if (specifiers.program != Program::None)
            {
                if (program != Program::None)
                    throw ParseError("Single function can not have multiple program attributes");

                program = specifiers.program;
            }

            if (specifiers.semantic != Semantic::None)
            {
                if (semantic != Semantic::None)
                    throw ParseError("Single variable can not have multiple semantic attributes");

                semantic = specifiers.semantic;
                semanticIndex = specifiers.semanticIndex;
            }

            qualifiedType.qualifiers |= specifiers.qualifiers;

            if (specifiers.isExtern) isExtern = true;
            if (specifiers.isInline) isInline = true;

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
                result->qualifiedType = qualifiedType;
                result->isInline = isInline;
                result->program = program;
                result->name = name;
                result->semantic = semantic;
                result->semanticIndex = semanticIndex;

                std::vector<QualifiedType> parameters;

                if (isToken(Token::Type::Void, iterator, end))
                    ++iterator;
                else if (!isToken(Token::Type::RightParenthesis, iterator, end))
                {
                    for (;;)
                    {
                        auto parameterDeclaration = parseParameterDeclaration(iterator, end, declarationScopes);
                        parameterDeclaration->parent = result;
                        result->parameterDeclarations.push_back(parameterDeclaration);
                        parameters.push_back(parameterDeclaration->qualifiedType);

                        if (!isToken(Token::Type::Comma, iterator, end))
                            break;

                        ++iterator;
                    }
                }

                expectToken(Token::Type::RightParenthesis, iterator, end);

                ++iterator;

                // TODO: forbid declaring a function with the same name as a declared type (not supported by GLSL)
                auto previousDeclaration = findFunctionDeclaration(name, declarationScopes, parameters);

                if (previousDeclaration)
                {
                    result->previousDeclaration = previousDeclaration;
                    result->firstDeclaration = previousDeclaration->firstDeclaration;
                    result->definition = previousDeclaration->definition;
                }
                else
                    result->firstDeclaration = result;

                declarationScopes.back().push_back(result);

                if (isToken(Token::Type::LeftBrace, iterator, end))
                {
                    // check if only one definition exists
                    if (result->definition)
                        throw ParseError("Redefinition of " + result->name);

                    // set the definition pointer of all previous declarations
                    Declaration* previousDeclaration = result;
                    while (previousDeclaration)
                    {
                        previousDeclaration->definition = result;
                        previousDeclaration = previousDeclaration->previousDeclaration;
                    }

                    declarationScopes.push_back(std::vector<Declaration*>()); // add scope for parameters

                    for (ParameterDeclaration* parameterDeclaration : result->parameterDeclarations)
                        declarationScopes.back().push_back(parameterDeclaration);

                    // parse body
                    auto body = parseCompoundStatement(iterator, end, declarationScopes);
                    body->parent = result;
                    result->body = body;

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
                result->qualifiedType = qualifiedType;
                result->storageClass = isExtern ? StorageClass::Extern : StorageClass::Auto;
                result->name = name;
                result->semantic = semantic;
                result->semanticIndex = semanticIndex;

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

                    auto initialization = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);
                    initialization->parent = result;

                    if (initialization->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                        throw ParseError("Member can not have a void type");

                    result->initialization = initialization;

                    // TODO: check for comma and parse multiple expressions

                    expectToken(Token::Type::RightParenthesis, iterator, end);

                    ++iterator;
                }
                else if (isToken(Token::Type::Assignment, iterator, end))
                {
                    ++iterator;

                    auto initialization = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);
                    initialization->parent = result;

                    if (initialization->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                        throw ParseError("Initialization with a void type");

                    result->initialization = initialization;
                }

                declarationScopes.back().push_back(result);

                return result;
            }
        }
    }

    TypeDeclaration* ASTContext::parseStructTypeDeclaration(std::vector<Token>::const_iterator& iterator,
                                                            std::vector<Token>::const_iterator end,
                                                            std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        TypeDeclaration* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new TypeDeclaration()));

        expectToken(Token::Type::Identifier, iterator, end);
        result->name = iterator->value;

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
            types.push_back(std::unique_ptr<Type>(structType = new StructType()));
            structType->name = iterator->value;
            structType->declaration = result;
            result->firstDeclaration = result;
            result->type = structType;
        }

        ++iterator;

        declarationScopes.back().push_back(result);

        if (isToken(Token::Type::LeftBrace, iterator, end))
        {
            ++iterator;

            // check if only one definition exists
            if (result->definition)
                throw ParseError("Redefinition of " + result->name);

            // set the definition pointer of all previous declarations
            Declaration* previousDeclaration = result;
            while (previousDeclaration)
            {
                previousDeclaration->definition = result;
                previousDeclaration = previousDeclaration->previousDeclaration;
            }

            for (;;)
            {
                if (isToken(Token::Type::RightBrace, iterator, end))
                {
                    ++iterator;
                    break;
                }
                else
                {
                    auto memberDeclaration = parseMemberDeclaration(iterator, end, declarationScopes);
                    memberDeclaration->parent = result;

                    expectToken(Token::Type::Semicolon, iterator, end);

                    if (structType->findMemberDeclaration(memberDeclaration->name))
                        throw ParseError("Redefinition of member " + memberDeclaration->name);

                    ++iterator;

                    structType->memberDeclarations.push_back(memberDeclaration);
                }
            }
        }

        return result;
    }

    Declaration* ASTContext::parseMemberDeclaration(std::vector<Token>::const_iterator& iterator,
                                                    std::vector<Token>::const_iterator end,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        if (isToken(Token::Type::Semicolon, iterator, end))
        {
            Declaration* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new Declaration(Declaration::Kind::Empty)));

            return result;
        }
        else
        {
            FieldDeclaration* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new FieldDeclaration()));

            ASTContext::Specifiers specifiers = parseSpecifiers(iterator, end);

            result->qualifiedType.qualifiers |= specifiers.qualifiers;

            bool isInline = specifiers.isInline;
            Semantic semantic = specifiers.semantic;
            size_t semanticIndex = specifiers.semanticIndex;

            result->qualifiedType.type = parseType(iterator, end, declarationScopes);

            if (result->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                throw ParseError("Member can not have a void type");

            if (result->qualifiedType.type->getTypeKind() == Type::Kind::Struct)
            {
                auto structType = static_cast<const StructType*>(result->qualifiedType.type);
                if (!structType->declaration->definition)
                    throw ParseError("Incomplete type " + result->qualifiedType.type->name);
            }

            specifiers = parseSpecifiers(iterator, end);

            result->qualifiedType.qualifiers |= specifiers.qualifiers;
            result->semantic = semantic;
            result->semanticIndex = semanticIndex;

            if (specifiers.isInline) isInline = true;

            if (specifiers.semantic != Semantic::None)
            {
                if (semantic != Semantic::None)
                    throw ParseError("Single variable can not have multiple semantic attributes");

                semantic = specifiers.semantic;
                semanticIndex = specifiers.semanticIndex;
            }

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
    }

    ParameterDeclaration* ASTContext::parseParameterDeclaration(std::vector<Token>::const_iterator& iterator,
                                                                std::vector<Token>::const_iterator end,
                                                                std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        ParameterDeclaration* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new ParameterDeclaration()));

        ASTContext::Specifiers specifiers = parseSpecifiers(iterator, end);

        result->qualifiedType.qualifiers |= specifiers.qualifiers;

        bool isInline = specifiers.isInline;
        Semantic semantic = specifiers.semantic;
        size_t semanticIndex = specifiers.semanticIndex;

        result->qualifiedType.type = parseType(iterator, end, declarationScopes);

        if (result->qualifiedType.type->getTypeKind() == Type::Kind::Void)
            throw ParseError("Parameter can not have a void type");

        if (result->qualifiedType.type->getTypeKind() == Type::Kind::Struct)
        {
            auto structType = static_cast<const StructType*>(result->qualifiedType.type);
            if (!structType->declaration->definition)
                throw ParseError("Incomplete type " + result->qualifiedType.type->name);
        }

        specifiers = parseSpecifiers(iterator, end);

        result->qualifiedType.qualifiers |= specifiers.qualifiers;
        result->semantic = semantic;
        result->semanticIndex = semanticIndex;

        if (specifiers.isInline) isInline = true;

        if (isInline)
            throw ParseError("Parameters can not be inline");

        if (specifiers.semantic != Semantic::None)
        {
            if (semantic != Semantic::None)
                throw ParseError("Single variable can not have multiple semantic attributes");

            semantic = specifiers.semantic;
            semanticIndex = specifiers.semanticIndex;
        }

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
                                                                            std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        throw ParseError("Typedef is not supported");

        TypeDefinitionDeclaration* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new TypeDefinitionDeclaration()));
        result->kind = Construct::Kind::Declaration;
        result->declarationKind = Declaration::Kind::Type;
        result->typeKind = Type::Kind::TypeDefinition;
        result->declaration = findTypeDeclaration(iterator->value, declarationScopes);

        result->qualifiedType.type = parseType(iterator, end, declarationScopes);

        if (result->qualifiedType.type->getTypeKind() == Type::Kind::STRUCT)
        {
            auto structType = static_cast<const StructType*>(result->qualifiedType.type);
            if (!structType->hasDefinition)
                throw ParseError("Incomplete type " + result->qualifiedType.type->name);
        }

        expectToken(Token::Type::IDENTIFIER, iterator, end);

        result->name = iterator->value;
        // TODO: forbid declaring type with an existing name

        ++iterator;

        expectToken(Token::Type::SEMICOLON, iterator, end);

        ++iterator;

        return result;
    }*/

    Statement* ASTContext::parseStatement(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        if (isToken(Token::Type::LeftBrace, iterator, end))
            return parseCompoundStatement(iterator, end, declarationScopes);
        else if (isToken(Token::Type::If, iterator, end))
            return parseIfStatement(iterator, end, declarationScopes);
        else if (isToken(Token::Type::For, iterator, end))
            return parseForStatement(iterator, end, declarationScopes);
        else if (isToken(Token::Type::Switch, iterator, end))
            return parseSwitchStatement(iterator, end, declarationScopes);
        else if (isToken(Token::Type::Case, iterator, end))
            return parseCaseStatement(iterator, end, declarationScopes);
        else if (isToken(Token::Type::Default, iterator, end))
            return parseDefaultStatement(iterator, end, declarationScopes);
        else if (isToken(Token::Type::While, iterator, end))
            return parseWhileStatement(iterator, end, declarationScopes);
        else if (isToken(Token::Type::Do, iterator, end))
            return parseDoStatement(iterator, end, declarationScopes);
        else if (isToken(Token::Type::Break, iterator, end))
        {
            ++iterator;

            BreakStatement* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new BreakStatement()));

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;

            return result;
        }
        else if (isToken(Token::Type::Continue, iterator, end))
        {
            ++iterator;

            ContinueStatement* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new ContinueStatement()));

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;

            return result;
        }
        else if (isToken(Token::Type::Return, iterator, end))
        {
            ++iterator;

            ReturnStatement* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new ReturnStatement()));

            auto resultExpression = parseExpression(iterator, end, declarationScopes);
            resultExpression->parent = result;
            result->result = resultExpression;

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;

            return result;
        }
        else if (isDeclaration(iterator, end, declarationScopes))
        {
            DeclarationStatement* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new DeclarationStatement()));

            auto declaration = parseDeclaration(iterator, end, declarationScopes);
            declaration->parent = result;

            if (declaration->getDeclarationKind() != Declaration::Kind::Variable)
                throw ParseError("Expected a variable declaration");

            result->declaration = declaration;

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;

            return result;
        }
        else if (isToken(Token::Type::Semicolon, iterator, end))
        {
            ++iterator;

            Statement* statement = new Statement(Statement::Kind::Empty);
            constructs.push_back(std::unique_ptr<Construct>(statement));

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

            auto expression = parseExpression(iterator, end, declarationScopes);
            expression->parent = result;
            result->expression = expression;

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;

            return result;
        }
    }

    CompoundStatement* ASTContext::parseCompoundStatement(std::vector<Token>::const_iterator& iterator,
                                                          std::vector<Token>::const_iterator end,
                                                          std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        expectToken(Token::Type::LeftBrace, iterator, end);

        ++iterator;

        declarationScopes.push_back(std::vector<Declaration*>());

        CompoundStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new CompoundStatement()));

        for (;;)
        {
            if (isToken(Token::Type::RightBrace, iterator, end))
            {
                ++iterator;
                break;
            }
            else
            {
                auto statement = parseStatement(iterator, end, declarationScopes);
                statement->parent = result;

                result->statements.push_back(statement);
            }
        }

        declarationScopes.pop_back();

        return result;
    }

    IfStatement* ASTContext::parseIfStatement(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        expectToken(Token::Type::If, iterator, end);

        ++iterator;

        IfStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new IfStatement()));

        expectToken(Token::Type::LeftParenthesis, iterator, end);

        ++iterator;

        if (isDeclaration(iterator, end, declarationScopes))
        {
            // TODO: add implicit cast to bool
            auto declaration = parseDeclaration(iterator, end, declarationScopes);
            declaration->parent = result;

            if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
                declaration->getDeclarationKind() != Declaration::Kind::Parameter)
                throw ParseError("Expected a variable declaration");

            result->condition = declaration;
        }
        else
        {
            auto condition = parseExpression(iterator, end, declarationScopes);
            condition->parent = result;

            if (!isBooleanType(condition->qualifiedType.type))
                throw ParseError("Condition is not a boolean");

            result->condition = condition;
        }

        expectToken(Token::Type::RightParenthesis, iterator, end);

        ++iterator;

        auto statement = parseStatement(iterator, end, declarationScopes);
        statement->parent = result;
        result->body = statement;

        if (isToken(Token::Type::Else, iterator, end))
        {
            ++iterator;

            statement = parseStatement(iterator, end, declarationScopes);
            statement->parent = result;
            result->elseBody = statement;
        }

        return result;
    }

    ForStatement* ASTContext::parseForStatement(std::vector<Token>::const_iterator& iterator,
                                                std::vector<Token>::const_iterator end,
                                                std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        expectToken(Token::Type::For, iterator, end);

        ++iterator;

        ForStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new ForStatement()));

        expectToken(Token::Type::LeftParenthesis, iterator, end);

        ++iterator;

        if (isDeclaration(iterator, end, declarationScopes))
        {
            auto declaration = parseDeclaration(iterator, end, declarationScopes);
            declaration->parent = result;

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
            auto initialization = parseExpression(iterator, end, declarationScopes);
            initialization->parent = result;
            result->initialization = initialization;

            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;
        }

        if (isDeclaration(iterator, end, declarationScopes))
        {
            auto declaration = parseDeclaration(iterator, end, declarationScopes);
            declaration->parent = result;

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
            auto condition = parseExpression(iterator, end, declarationScopes);
            condition->parent = result;

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
            auto increment = parseExpression(iterator, end, declarationScopes);
            increment->parent = result;
            result->increment = increment;

            expectToken(Token::Type::RightParenthesis, iterator, end);

            ++iterator;
        }

        auto body = parseStatement(iterator, end, declarationScopes);
        body->parent = result;
        result->body = body;

        return result;
    }

    SwitchStatement* ASTContext::parseSwitchStatement(std::vector<Token>::const_iterator& iterator,
                                                      std::vector<Token>::const_iterator end,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        expectToken(Token::Type::Switch, iterator, end);

        ++iterator;

        SwitchStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new SwitchStatement()));

        expectToken(Token::Type::LeftParenthesis, iterator, end);

        ++iterator;

        // TODO: add implicit cast to int
        if (isDeclaration(iterator, end, declarationScopes))
        {
            auto declaration = parseDeclaration(iterator, end, declarationScopes);
            declaration->parent = result;

            if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
                declaration->getDeclarationKind() != Declaration::Kind::Parameter)
                throw ParseError("Expected a variable declaration");

            result->condition = declaration;
        }
        else
        {
            auto condition = parseExpression(iterator, end, declarationScopes);
            condition->parent = result;

            if (!isIntegerType(condition->qualifiedType.type))
                throw ParseError("Statement requires expression of integer type");

            result->condition = condition;
        }

        expectToken(Token::Type::RightParenthesis, iterator, end);

        ++iterator;

        auto body = parseStatement(iterator, end, declarationScopes);
        body->parent = result;
        result->body = body;

        return result;
    }

    CaseStatement* ASTContext::parseCaseStatement(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        expectToken(Token::Type::Case, iterator, end);

        ++iterator;

        CaseStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new CaseStatement()));

        auto condition = parseExpression(iterator, end, declarationScopes);
        condition->parent = result;

        if (!isIntegerType(condition->qualifiedType.type))
            throw ParseError("Statement requires expression of integer type");

        if ((condition->qualifiedType.qualifiers & Qualifiers::Const) != Qualifiers::Const)
            throw ParseError("Expression must be constant");

        result->condition = condition;

        expectToken(Token::Type::Colon, iterator, end);

        ++iterator;

        auto body = parseStatement(iterator, end, declarationScopes);
        body->parent = result;
        result->body = body;

        return result;
    }

    DefaultStatement* ASTContext::parseDefaultStatement(std::vector<Token>::const_iterator& iterator,
                                                        std::vector<Token>::const_iterator end,
                                                        std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        expectToken(Token::Type::Default, iterator, end);

        ++iterator;

        DefaultStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new DefaultStatement()));

        expectToken(Token::Type::Colon, iterator, end);

        ++iterator;

        auto body = parseStatement(iterator, end, declarationScopes);
        body->parent = result;
        result->body = body;

        return result;
    }

    WhileStatement* ASTContext::parseWhileStatement(std::vector<Token>::const_iterator& iterator,
                                                    std::vector<Token>::const_iterator end,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        expectToken(Token::Type::While, iterator, end);

        ++iterator;

        WhileStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new WhileStatement()));

        expectToken(Token::Type::LeftParenthesis, iterator, end);

        ++iterator;

        if (isDeclaration(iterator, end, declarationScopes))
        {
            // TODO: add implicit cast to bool
            auto declaration = parseDeclaration(iterator, end, declarationScopes);
            declaration->parent = result;

            if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
                declaration->getDeclarationKind() != Declaration::Kind::Parameter)
                throw ParseError("Expected a variable declaration");

            result->condition = declaration;
        }
        else
        {
            auto condition = parseExpression(iterator, end, declarationScopes);
            condition->parent = result;

            if (!isBooleanType(condition->qualifiedType.type))
                throw ParseError("Condition is not a boolean");

            result->condition = condition;
        }

        expectToken(Token::Type::RightParenthesis, iterator, end);

        ++iterator;

        auto body = parseStatement(iterator, end, declarationScopes);
        body->parent = result;
        result->body = body;

        return result;
    }

    DoStatement* ASTContext::parseDoStatement(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        expectToken(Token::Type::Do, iterator, end);

        ++iterator;

        DoStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new DoStatement()));

        auto body = parseStatement(iterator, end, declarationScopes);
        body->parent = result;
        result->body = body;

        expectToken(Token::Type::While, iterator, end);

        ++iterator;

        expectToken(Token::Type::LeftParenthesis, iterator, end);

        ++iterator;

        auto condition = parseExpression(iterator, end, declarationScopes);
        condition->parent = result;

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
                                                   std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        if (isToken(Token::Type::IntLiteral, iterator, end))
        {
            IntegerLiteralExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new IntegerLiteralExpression()));
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
            result->qualifiedType.type = boolType;
            result->qualifiedType.qualifiers = Qualifiers::Const;
            result->category = Expression::Category::Rvalue;
            result->value = (iterator->type == Token::Type::True);

            ++iterator;

            return result;
        }
        else if (isToken({Token::Type::Bool, Token::Type::Int, Token::Type::Float}, iterator, end))
        {
            // TODO: parse unsigned
            CastExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new CastExpression(CastExpression::Kind::Functional)));

            if (isToken(Token::Type::Bool, iterator, end)) result->qualifiedType.type = boolType;
            else if(isToken(Token::Type::Int, iterator, end)) result->qualifiedType.type = intType;
            else if(isToken(Token::Type::Float, iterator, end)) result->qualifiedType.type = floatType;

            ++iterator;

            expectToken(Token::Type::LeftParenthesis, iterator, end);

            ++iterator;

            auto expression = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);
            expression->parent = result;
            result->expression = expression;

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
                auto expression = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);
                expression->parent = result;

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

                if (auto type = findType(name, declarationScopes))
                {
                    std::vector<Expression*> parameters;
                    std::vector<QualifiedType> parameterTypes;

                    if (isToken(Token::Type::RightParenthesis, iterator, end)) // no arguments
                        ++iterator;
                    else
                    {
                        for (;;)
                        {
                            auto parameter = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);

                            parameters.push_back(parameter);
                            parameterTypes.push_back(parameter->qualifiedType);

                            if (isToken(Token::Type::Comma, iterator, end))
                                ++iterator;
                            else
                                break;
                        }

                        expectToken(Token::Type::RightParenthesis, iterator, end);

                        ++iterator;
                    }

                    switch (type->getTypeKind())
                    {
                        case Type::Kind::Struct:
                        {
                            auto structType = static_cast<StructType*>(type);

                            TemporaryObjectExpression* result;
                            constructs.push_back(std::unique_ptr<Construct>(result = new TemporaryObjectExpression()));
                            result->qualifiedType.type = type;
                            result->qualifiedType.qualifiers = Qualifiers::Const;
                            result->category = Expression::Category::Rvalue;

                            for (auto parameter : parameters)
                            {
                                parameter->parent = result;
                                result->parameters.push_back(parameter);
                            }

                            if (!(result->constructorDeclaration = structType->findConstructorDeclaration(parameterTypes)))
                                throw ParseError("No matching constructor found");

                            return result;
                        }
                        case Type::Kind::Vector:
                        {
                            auto vectorType = static_cast<VectorType*>(type);

                            VectorInitializeExpression* result;
                            constructs.push_back(std::unique_ptr<Construct>(result = new VectorInitializeExpression()));
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

                                parameter->parent = result;
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
                            auto matrixType = static_cast<MatrixType*>(type);

                            MatrixInitializeExpression* result;
                            constructs.push_back(std::unique_ptr<Construct>(result = new MatrixInitializeExpression()));
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

                                parameter->parent = result;
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
                    CallExpression* result;
                    constructs.push_back(std::unique_ptr<Construct>(result = new CallExpression()));

                    std::vector<QualifiedType> arguments;

                    if (isToken(Token::Type::RightParenthesis, iterator, end)) // no arguments
                        ++iterator;
                    else
                    {
                        for (;;)
                        {
                            auto argument = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);
                            argument->parent = result;

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

                    auto functionDeclaration = resolveFunctionDeclaration(name, declarationScopes, arguments);
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
                result->declaration = findDeclaration(name, declarationScopes);
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
        else if (isToken(Token::Type::LeftParenthesis, iterator, end))
        {
            ++iterator;

            if (isType(iterator, end, declarationScopes))
            {
                CastExpression* result = new CastExpression(CastExpression::Kind::CStyle);
                constructs.push_back(std::unique_ptr<Construct>(result));

                // TODO: parse qualifiers
                result->qualifiedType.type = parseType(iterator, end, declarationScopes);

                expectToken(Token::Type::RightParenthesis, iterator, end);
                ++iterator;

                auto expression = parseExpression(iterator, end, declarationScopes);
                expression->parent = result;

                result->expression = expression;
                result->category = Expression::Category::Rvalue;

                return result;
            }
            else
            {
                ParenExpression* result;
                constructs.push_back(std::unique_ptr<Construct>(result = new ParenExpression()));

                auto expression = parseExpression(iterator, end, declarationScopes);
                expression->parent = result;

                result->expression = expression;

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

            ++iterator;

            expectToken(Token::Type::LessThan, iterator, end);
            ++iterator;

            // TODO: parse qualifiers
            result->qualifiedType.type = parseType(iterator, end, declarationScopes);

            expectToken(Token::Type::GreaterThan, iterator, end);
            ++iterator;

            expectToken(Token::Type::LeftParenthesis, iterator, end);
            ++iterator;

            auto expression = parseExpression(iterator, end, declarationScopes);
            expression->parent = result;

            result->expression = expression;

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
    }

    Expression* ASTContext::parsePostfixExpression(std::vector<Token>::const_iterator& iterator,
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

            UnaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new UnaryOperatorExpression(operatorKind)));
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
                                                     std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        auto result = parsePostfixExpression(iterator, end, declarationScopes);

        while (isToken(Token::Type::LeftBracket, iterator, end))
        {
            ++iterator;

            if (result->qualifiedType.type->getTypeKind() == Type::Kind::Array)
            {
                ArraySubscriptExpression* expression;
                constructs.push_back(std::unique_ptr<Construct>(expression = new ArraySubscriptExpression()));
                expression->expression = result;

                auto subscript = parseExpression(iterator, end, declarationScopes);
                subscript->parent = expression;
                if (!isIntegerType(subscript->qualifiedType.type))
                    throw ParseError("Subscript is not an integer");

                expression->subscript = subscript;

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
                expression->leftExpression = result;

                auto rightExpression = parseExpression(iterator, end, declarationScopes);
                rightExpression->parent = expression;
                if (!isIntegerType(rightExpression->qualifiedType.type))
                    throw ParseError("Subscript is not an integer");

                expression->rightExpression = rightExpression;

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

                    auto vectorType = findVectorType(matrixType->componentType, matrixType->columnCount);
                    if (!vectorType)
                        throw ParseError("Invalid vector type");

                    expression->qualifiedType.type = vectorType;
                }

                expression->category = expression->leftExpression->category;

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
                MemberExpression* expression;
                constructs.push_back(std::unique_ptr<Construct>(expression = new MemberExpression()));
                expression->expression = result;

                auto structType = static_cast<const StructType*>(result->qualifiedType.type);

                expectToken(Token::Type::Identifier, iterator, end);

                Declaration* memberDeclaration = structType->findMemberDeclaration(iterator->value);
                if (!memberDeclaration)
                    throw ParseError("Structure " + structType->name +  " has no member " + iterator->value);

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

                auto resultType = findVectorType(vectorType->componentType, components.size());
                if (!resultType)
                    throw ParseError("Invalid swizzle");

                expression->qualifiedType.type = resultType;

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
                                                  std::vector<std::vector<Declaration*>>& declarationScopes)
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

            auto expression = parseMemberExpression(iterator, end, declarationScopes);
            expression->parent = result;

            if (expression->category != Expression::Category::Lvalue)
                throw ParseError("Expression is not assignable");

            if ((expression->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                throw ParseError("Cannot assign to const variable");

            if (expression->qualifiedType.type->getTypeKind() != Type::Kind::Scalar)
                throw ParseError("Parameter of the prefix operator must be a number");

            result->expression = expression;
            result->qualifiedType.type = result->expression->qualifiedType.type;
            result->category = Expression::Category::Rvalue;

            return result;
        }
        else
            return parseMemberExpression(iterator, end, declarationScopes);
    }

    Expression* ASTContext::parseSignExpression(std::vector<Token>::const_iterator& iterator,
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

            UnaryOperatorExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new UnaryOperatorExpression(operatorKind)));

            auto expression = parsePrefixExpression(iterator, end, declarationScopes);
            expression->parent = result;

            if (expression->qualifiedType.type->getTypeKind() != Type::Kind::Scalar)
                throw ParseError("Parameter of the sign operator must be a number");

            result->expression = expression;
            result->qualifiedType.type = result->expression->qualifiedType.type;
            result->category = Expression::Category::Rvalue;

            return result;
        }
        else
            return parsePrefixExpression(iterator, end, declarationScopes);
    }

    Expression* ASTContext::parseNotExpression(std::vector<Token>::const_iterator& iterator,
                                               std::vector<Token>::const_iterator end,
                                               std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        if (isToken(Token::Type::Not, iterator, end))
        {
            ++iterator;

            const auto operatorKind = UnaryOperatorExpression::Kind::Negation;

            UnaryOperatorExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new UnaryOperatorExpression(operatorKind)));

            auto expression = parseExpression(iterator, end, declarationScopes);
            expression->parent = result;
            if (!isBooleanType(expression->qualifiedType.type))
                throw ParseError("Expression is not a boolean");

            result->expression = expression;
            result->qualifiedType.type = boolType;
            result->category = Expression::Category::Rvalue;

            return result;
        }
        else
            return parseSignExpression(iterator, end, declarationScopes);
    }

    Expression* ASTContext::parseSizeofExpression(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        if (isToken(Token::Type::Sizeof, iterator, end))
        {
            ++iterator;
            expectToken(Token::Type::LeftParenthesis, iterator, end);
            ++iterator;

            if (iterator == end)
                throw ParseError("Unexpected end of file");

            SizeofExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new SizeofExpression()));

            if (iterator->type == Token::Type::Void)
                throw ParseError("Parameter of sizeof can not be void");

            if (isType(iterator, end, declarationScopes))
                result->type = parseType(iterator, end, declarationScopes);
            else
            {
                auto expression = parseExpression(iterator, end, declarationScopes);
                expression->parent = result;

                result->expression = expression;
            }

            expectToken(Token::Type::RightParenthesis, iterator, end);
            ++iterator;

            result->qualifiedType.type = unsignedIntType;
            result->category = Expression::Category::Rvalue;

            return result;
        }
        else
            return parseNotExpression(iterator, end, declarationScopes);
    }

    Expression* ASTContext::parseMultiplicationExpression(std::vector<Token>::const_iterator& iterator,
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

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->leftExpression = result;

            auto rightExpression = parseSizeofExpression(iterator, end, declarationScopes);
            rightExpression->parent = expression;

            expression->rightExpression = rightExpression;
            expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseAdditionExpression(std::vector<Token>::const_iterator& iterator,
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

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->leftExpression = result;

            auto rightExpression = parseMultiplicationExpression(iterator, end, declarationScopes);
            rightExpression->parent = expression;

            expression->rightExpression = rightExpression;
            expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseLessThanExpression(std::vector<Token>::const_iterator& iterator,
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

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->leftExpression = result;

            auto rightExpression = parseAdditionExpression(iterator, end, declarationScopes);
            rightExpression->parent = expression;

            expression->rightExpression = rightExpression;
            expression->qualifiedType.type = boolType;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseGreaterThanExpression(std::vector<Token>::const_iterator& iterator,
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

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->leftExpression = result;

            auto rightExpression = parseLessThanExpression(iterator, end, declarationScopes);
            rightExpression->parent = expression;

            expression->rightExpression = rightExpression;
            expression->qualifiedType.type = boolType;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseEqualityExpression(std::vector<Token>::const_iterator& iterator,
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

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->leftExpression = result;

            auto rightExpression = parseGreaterThanExpression(iterator, end, declarationScopes);
            rightExpression->parent = expression;

            expression->rightExpression = rightExpression;
            expression->qualifiedType.type = boolType;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseLogicalAndExpression(std::vector<Token>::const_iterator& iterator,
                                                      std::vector<Token>::const_iterator end,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        auto result = parseEqualityExpression(iterator, end, declarationScopes);

        while (isToken(Token::Type::And, iterator, end))
        {
            ++iterator;

            const auto operatorKind = BinaryOperatorExpression::Kind::And;

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->leftExpression = result;

            auto rightExpression = parseEqualityExpression(iterator, end, declarationScopes);
            rightExpression->parent = expression;

            expression->rightExpression = rightExpression;
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
                                                     std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        auto result = parseLogicalAndExpression(iterator, end, declarationScopes);

        while (isToken(Token::Type::Or, iterator, end))
        {
            ++iterator;

            const auto operatorKind = BinaryOperatorExpression::Kind::Or;

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->leftExpression = result;

            auto rightExpression = parseLogicalAndExpression(iterator, end, declarationScopes);
            rightExpression->parent = expression;

            expression->rightExpression = rightExpression;
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
                                                   std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        auto result = parseLogicalOrExpression(iterator, end, declarationScopes);

        while (isToken(Token::Type::Conditional, iterator, end))
        {
            ++iterator;

            TernaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new TernaryOperatorExpression()));

            if (!isBooleanType(result->qualifiedType.type))
                throw ParseError("Condition is not a boolean");

            expression->condition = result;

            if (expression->condition->qualifiedType.type->getTypeKind() == Type::Kind::Void)
                throw ParseError("Ternary expression with a void condition");

            auto leftExpression = parseTernaryExpression(iterator, end, declarationScopes);
            leftExpression->parent = expression;

            expectToken(Token::Type::Colon, iterator, end);

            ++iterator;

            auto rightExpression = parseTernaryExpression(iterator, end, declarationScopes);
            rightExpression->parent = expression;

            expression->leftExpression = leftExpression;
            expression->rightExpression = rightExpression;
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
                                                      std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        auto result = parseTernaryExpression(iterator, end, declarationScopes);

        while (isToken(Token::Type::Assignment, iterator, end))
        {
            ++iterator;

            const auto operatorKind = BinaryOperatorExpression::Kind::Assignment;

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->leftExpression = result;

            if (expression->leftExpression->category != Expression::Category::Lvalue)
                throw ParseError("Expression is not assignable");

            if ((expression->leftExpression->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                throw ParseError("Cannot assign to const variable");

            auto rightExpression = parseTernaryExpression(iterator, end, declarationScopes);
            rightExpression->parent = expression;

            expression->rightExpression = rightExpression;
            expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;
            expression->category = Expression::Category::Rvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseAdditionAssignmentExpression(std::vector<Token>::const_iterator& iterator,
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

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->leftExpression = result;

            if (expression->leftExpression->category != Expression::Category::Lvalue)
                throw ParseError("Expression is not assignable");

            if ((expression->leftExpression->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                throw ParseError("Cannot assign to const variable");

            auto rightExpression = parseAssignmentExpression(iterator, end, declarationScopes);
            rightExpression->parent = expression;

            expression->rightExpression = rightExpression;
            expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;
            expression->category = Expression::Category::Lvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseMultiplicationAssignmentExpression(std::vector<Token>::const_iterator& iterator,
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

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->leftExpression = result;

            if (expression->leftExpression->category != Expression::Category::Lvalue)
                throw ParseError("Expression is not assignable");

            if ((expression->leftExpression->qualifiedType.qualifiers & Qualifiers::Const) == Qualifiers::Const)
                throw ParseError("Cannot assign to const variable");

            auto rightExpression = parseAdditionAssignmentExpression(iterator, end, declarationScopes);
            rightExpression->parent = expression;

            expression->rightExpression = rightExpression;
            expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;
            expression->category = Expression::Category::Lvalue;

            result->parent = expression;
            result = expression;
        }

        return result;
    }

    Expression* ASTContext::parseCommaExpression(std::vector<Token>::const_iterator& iterator,
                                                 std::vector<Token>::const_iterator end,
                                                 std::vector<std::vector<Declaration*>>& declarationScopes)
    {
        auto result = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes);

        while (isToken(Token::Type::Comma, iterator, end))
        {
            ++iterator;

            const auto operatorKind = BinaryOperatorExpression::Kind::Comma;

            BinaryOperatorExpression* expression;
            constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression(operatorKind)));
            expression->leftExpression = result;

            auto rightExpression = parseAdditionAssignmentExpression(iterator, end, declarationScopes);
            rightExpression->parent = expression;

            expression->rightExpression = rightExpression;
            expression->qualifiedType.type = expression->rightExpression->qualifiedType.type;
            expression->category = expression->rightExpression->category;

            if (expression->leftExpression->qualifiedType.type !=
                expression->rightExpression->qualifiedType.type)
                throw ParseError("Incompatible operand types");

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
