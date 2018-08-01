//
//  OSL
//

#include <algorithm>
#include <iostream>
#include "Parser.hpp"

ASTContext::ASTContext()
{
}

ASTContext::ASTContext(const std::vector<Token>& tokens)
{
    std::vector<std::vector<Declaration*>> declarationScopes;
    declarationScopes.push_back(std::vector<Declaration*>());

    boolTypeDeclaration = addScalarTypeDeclaration("bool", ScalarTypeDeclaration::Kind::BOOLEAN, 1, false, declarationScopes);
    addOperatorDeclaration(Operator::NEGATION, boolTypeDeclaration, {boolTypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::OR, boolTypeDeclaration, {boolTypeDeclaration, boolTypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::AND, boolTypeDeclaration, {boolTypeDeclaration, boolTypeDeclaration}, declarationScopes);

    intTypeDeclaration = addScalarTypeDeclaration("int", ScalarTypeDeclaration::Kind::INTEGER, 4, false, declarationScopes);
    unsignedIntTypeDeclaration = addScalarTypeDeclaration("unsigned int", ScalarTypeDeclaration::Kind::INTEGER, 4, true, declarationScopes);
    floatTypeDeclaration = addScalarTypeDeclaration("float", ScalarTypeDeclaration::Kind::FLOATING_POINT, 4, false, declarationScopes);

    StructDeclaration* float2TypeDeclaration = addStructDeclaration("float2", 8, declarationScopes);
    StructDeclaration* float3TypeDeclaration = addStructDeclaration("float3", 12, declarationScopes);
    StructDeclaration* float4TypeDeclaration = addStructDeclaration("float4", 16, declarationScopes);

    std::vector<std::pair<StructDeclaration*, std::vector<TypeDeclaration*>>> constructors = {
        {float2TypeDeclaration, {floatTypeDeclaration}},
        {float2TypeDeclaration, {floatTypeDeclaration, floatTypeDeclaration}},
        {float3TypeDeclaration, {floatTypeDeclaration}},
        {float3TypeDeclaration, {floatTypeDeclaration, floatTypeDeclaration, floatTypeDeclaration}},
        {float4TypeDeclaration, {floatTypeDeclaration}},
        {float4TypeDeclaration, {floatTypeDeclaration, floatTypeDeclaration, floatTypeDeclaration, floatTypeDeclaration}}
    };

    ConstructorDeclaration* constructorDeclaration = constructorDeclarations;
    ParameterDeclaration* parameterDeclaration = parameterDeclarations;

    for (auto& constructor : constructors)
    {
        constructorDeclaration->parent = constructor.first;
        constructorDeclaration->definition = constructorDeclaration;

        for (auto& parameter : constructor.second)
        {
            parameterDeclaration->parent = constructorDeclaration;
            parameterDeclaration->qualifiedType.typeDeclaration = parameter;
            constructorDeclaration->parameterDeclarations.push_back(parameterDeclaration);
            ++parameterDeclaration;
        }

        constructor.first->memberDeclarations.push_back(constructorDeclaration);

        ++constructorDeclaration;
    }

    std::vector<std::pair<StructDeclaration*, std::vector<char>>> types = {
        {float2TypeDeclaration, {'x', 'y'}},
        {float2TypeDeclaration, {'r', 'g'}},
        {float3TypeDeclaration, {'x', 'y', 'z'}},
        {float3TypeDeclaration, {'r', 'g', 'b'}},
        {float4TypeDeclaration, {'x', 'y', 'z', 'w'}},
        {float4TypeDeclaration, {'r', 'g', 'b', 'a'}}
    };

    for (auto& type : types)
    {
        declarationScopes.push_back(std::vector<Declaration*>());

        for (char first : type.second)
        {
            addFieldDeclaration(type.first, {first}, floatTypeDeclaration, false, declarationScopes);

            for (char second : type.second)
            {
                bool secondConst = (second == first);

                addFieldDeclaration(type.first, {first, second}, float2TypeDeclaration, secondConst, declarationScopes);

                for (char third : type.second)
                {
                    bool thirdConst = (secondConst || third == first || third == second);

                    addFieldDeclaration(type.first, {first, second, third}, float3TypeDeclaration, thirdConst, declarationScopes);

                    for (char fourth : type.second)
                    {
                        bool fourthConst = (thirdConst || fourth == first || fourth == second || fourth == third);

                        addFieldDeclaration(type.first, {first, second, third, fourth}, float4TypeDeclaration, fourthConst, declarationScopes);
                    }
                }
            }
        }

        declarationScopes.pop_back();
    }

    StructDeclaration* float2x2TypeDeclaration = addStructDeclaration("float2x2", 16, declarationScopes);
    StructDeclaration* float3x3TypeDeclaration = addStructDeclaration("float3x3", 36, declarationScopes);
    StructDeclaration* float4x4TypeDeclaration = addStructDeclaration("float4x4", 64, declarationScopes);
    stringTypeDeclaration = addStructDeclaration("string", 8, declarationScopes);
    StructDeclaration* samplerStateTypeDeclaration = addStructDeclaration("SamplerState", 0, declarationScopes);
    StructDeclaration* texture2DTypeDeclaration = addStructDeclaration("Texture2D", 0, declarationScopes);

    addFunctionDeclaration("texture2D", float4TypeDeclaration, {samplerStateTypeDeclaration, float2TypeDeclaration}, declarationScopes);

    // unused for now
    (void)float2x2TypeDeclaration;
    (void)float3x3TypeDeclaration;
    (void)samplerStateTypeDeclaration;
    (void)texture2DTypeDeclaration;

    addOperatorDeclaration(Operator::MULTIPLICATION, float4x4TypeDeclaration, {float4x4TypeDeclaration, float4x4TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::MULTIPLICATION, float4TypeDeclaration, {float4x4TypeDeclaration, float4TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::MULTIPLICATION, float4TypeDeclaration, {float4TypeDeclaration, float4x4TypeDeclaration}, declarationScopes);
    
    addOperatorDeclaration(Operator::DIVISION, float4x4TypeDeclaration, {float4x4TypeDeclaration, float4x4TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::DIVISION, float4TypeDeclaration, {float4x4TypeDeclaration, float4TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::DIVISION, float4TypeDeclaration, {float4TypeDeclaration, float4x4TypeDeclaration}, declarationScopes);

    for (auto iterator = tokens.cbegin(); iterator != tokens.end();)
    {
        Declaration* declaration = parseTopLevelDeclaration(tokens, iterator, declarationScopes);
        declarations.push_back(declaration);
    }
}

CallableDeclaration* ASTContext::compareCallableDeclarations(CallableDeclaration* callableDeclaration1,
                                                             CallableDeclaration* callableDeclaration2,
                                                             const std::vector<QualifiedType>& parameters)
{
    CallableDeclaration* result = nullptr;

    if (!parameters.empty() && // both functions should have arguments
        parameters.size() == callableDeclaration1->parameterDeclarations.size() &&
        parameters.size() == callableDeclaration2->parameterDeclarations.size()) // they should have the same number of parameters
    {
        for (uint32_t i = 0; i < parameters.size(); ++i)
        {
            const QualifiedType& parameter = parameters[i];
            const QualifiedType& parameter1 = callableDeclaration1->parameterDeclarations[i]->qualifiedType;
            const QualifiedType& parameter2 = callableDeclaration2->parameterDeclarations[i]->qualifiedType;

            if (!parameter.typeDeclaration ||
                !parameter1.typeDeclaration ||
                !parameter2.typeDeclaration) continue; // any type

            if (parameter1.typeDeclaration->getTypeKind() == parameter.typeDeclaration->getTypeKind() &&
                parameter2.typeDeclaration->getTypeKind() == parameter.typeDeclaration->getTypeKind())
            {
                if (parameter1.typeDeclaration == parameter.typeDeclaration &&
                    parameter2.typeDeclaration == parameter.typeDeclaration)
                {
                    continue;
                }
                else if (parameter1.typeDeclaration == parameter.typeDeclaration)
                {
                    if (result == callableDeclaration2) return nullptr;
                    result = callableDeclaration1;
                }
                else if (parameter2.typeDeclaration == parameter.typeDeclaration)
                {
                    if (result == callableDeclaration1) return nullptr;
                    result = callableDeclaration2;
                }
                else
                {
                    if (parameter.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::ARRAY)
                    {
                        ArrayTypeDeclaration* arrayTypeDeclaration = static_cast<ArrayTypeDeclaration*>(parameter.typeDeclaration);
                        ArrayTypeDeclaration* arrayTypeDeclaration1 = static_cast<ArrayTypeDeclaration*>(parameter1.typeDeclaration);
                        ArrayTypeDeclaration* arrayTypeDeclaration2 = static_cast<ArrayTypeDeclaration*>(parameter2.typeDeclaration);

                        if (arrayTypeDeclaration1->size == arrayTypeDeclaration->size &&
                            arrayTypeDeclaration2->size == arrayTypeDeclaration->size)
                        {
                        }
                        else if (arrayTypeDeclaration1->size == arrayTypeDeclaration->size)
                        {
                            if (result == callableDeclaration2) return nullptr;
                            result = callableDeclaration1;
                        }
                        else if (arrayTypeDeclaration2->size == arrayTypeDeclaration->size)
                        {
                            if (result == callableDeclaration1) return nullptr;
                            result = callableDeclaration2;
                        }
                    }
                }
            }
            else if (parameter1.typeDeclaration->getTypeKind() == parameter.typeDeclaration->getTypeKind())
            {
                if (result == callableDeclaration2) return nullptr;
                result = callableDeclaration1;
            }
            else if (parameter2.typeDeclaration->getTypeKind() == parameter.typeDeclaration->getTypeKind())
            {
                if (result == callableDeclaration1) return nullptr;
                result = callableDeclaration2;
            }
        }
    }

    return result;
}

FunctionDeclaration* ASTContext::resolveFunctionDeclaration(const std::string& name,
                                                            const std::vector<std::vector<Declaration*>>& declarationScopes,
                                                            const std::vector<QualifiedType>& parameters)
{
    std::vector<FunctionDeclaration*> candidateFunctionDeclarations;

    for (auto scopeIterator = declarationScopes.crbegin(); scopeIterator != declarationScopes.crend(); ++scopeIterator)
    {
        for (auto declarationIterator = scopeIterator->crbegin(); declarationIterator != scopeIterator->crend(); ++declarationIterator)
        {
            if ((*declarationIterator)->name == name)
            {
                if ((*declarationIterator)->getDeclarationKind() != Declaration::Kind::CALLABLE) return nullptr;

                CallableDeclaration* callableDeclaration = static_cast<CallableDeclaration*>(*declarationIterator);

                if (callableDeclaration->getCallableDeclarationKind() != CallableDeclaration::Kind::FUNCTION) return nullptr;

                FunctionDeclaration* functionDeclaration = static_cast<FunctionDeclaration*>(callableDeclaration->getFirstDeclaration());

                if (std::find(candidateFunctionDeclarations.begin(), candidateFunctionDeclarations.end(), functionDeclaration) == candidateFunctionDeclarations.end())
                    candidateFunctionDeclarations.push_back(functionDeclaration);
            }
        }
    }

    std::vector<FunctionDeclaration*> viableFunctionDeclarations;

    for (FunctionDeclaration* functionDeclaration : candidateFunctionDeclarations)
    {
        if (functionDeclaration->parameterDeclarations.size() == parameters.size())
        {
            if (std::equal(parameters.begin(), parameters.end(),
                           functionDeclaration->parameterDeclarations.begin(),
                           [](const QualifiedType& qualifiedType,
                              const ParameterDeclaration* parameterDeclaration) {
                               bool scalar = qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::SCALAR &&
                               qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::SCALAR;

                               return (scalar || qualifiedType.typeDeclaration->getFirstDeclaration() == parameterDeclaration->qualifiedType.typeDeclaration->getFirstDeclaration());
                           }))
            {
                viableFunctionDeclarations.push_back(functionDeclaration);
            }
        }
    }

    if (viableFunctionDeclarations.empty())
        throw std::runtime_error("No matching function to call " + name + " found");
    else if (viableFunctionDeclarations.size() == 1)
        return *viableFunctionDeclarations.begin();
    else
    {
        if (parameters.empty()) // two or more functions with zero parameters
            throw std::runtime_error("Ambiguous call to " + name);

        for (auto first = viableFunctionDeclarations.begin(); first != viableFunctionDeclarations.end(); ++first)
        {
            bool best = true;
            for (auto second = viableFunctionDeclarations.begin(); second != viableFunctionDeclarations.end(); ++second)
            {
                if (first != second &&
                    compareCallableDeclarations(*first, *second, parameters) != *first)
                {
                    best = false;
                    break;
                }
            }

            if (best) return *first;
        };

        throw std::runtime_error("Ambiguous call to " + name);
    }

    return nullptr;
}

static std::string toString(Operator op)
{
    switch (op)
    {
        case Operator::NONE: return "NONE";
        case Operator::NEGATION: return "NEGATION";
        case Operator::POSITIVE: return "POSITIVE";
        case Operator::NEGATIVE: return "NEGATIVE";
        case Operator::ADDITION: return "ADDITION";
        case Operator::SUBTRACTION: return "SUBTRACTION";
        case Operator::MULTIPLICATION: return "MULTIPLICATION";
        case Operator::DIVISION: return "DIVISION";
        case Operator::ADDITION_ASSIGNMENT: return "ADDITION_ASSIGNMENT";
        case Operator::SUBTRACTION_ASSIGNMENT: return "SUBTRACTION_ASSIGNMENT";
        case Operator::MULTIPLICATION_ASSIGNMENT: return "MULTIPLICATION_ASSIGNMENT";
        case Operator::DIVISION_ASSIGNMENT: return "DIVISION_ASSIGNMENT";
        case Operator::LESS_THAN: return "LESS_THAN";
        case Operator::LESS_THAN_EQUAL: return "LESS_THAN_EQUAL";
        case Operator::GREATER_THAN: return "GREATER_THAN";
        case Operator::GREATER_THAN_EQUAL: return "GREATER_THAN_EQUAL";
        case Operator::EQUALITY: return "EQUALITY";
        case Operator::INEQUALITY: return "INEQUALITY";
        case Operator::ASSIGNMENT: return "ASSIGNMENT";
        case Operator::OR: return "OR";
        case Operator::AND: return "AND";
        case Operator::COMMA: return "COMMA";
        case Operator::CONDITIONAL: return "CONDITIONAL";
        default: return "unknown";
    }
}

OperatorDeclaration* ASTContext::resolveOperatorDeclaration(Operator op,
                                                            const std::vector<std::vector<Declaration*>>& declarationScopes,
                                                            const std::vector<QualifiedType>& parameters)
{
    std::vector<OperatorDeclaration*> candidateOperatorDeclarations;

    for (auto scopeIterator = declarationScopes.crbegin(); scopeIterator != declarationScopes.crend(); ++scopeIterator)
    {
        for (auto declarationIterator = scopeIterator->crbegin(); declarationIterator != scopeIterator->crend(); ++declarationIterator)
        {
            if ((*declarationIterator)->getDeclarationKind() == Declaration::Kind::CALLABLE)
            {
                CallableDeclaration* callableDeclaration = static_cast<CallableDeclaration*>(*declarationIterator);

                if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::OPERATOR)
                {
                    OperatorDeclaration* operatorDeclaration = static_cast<OperatorDeclaration*>(callableDeclaration->getFirstDeclaration());

                    if (operatorDeclaration->op == op &&
                        std::find(candidateOperatorDeclarations.begin(), candidateOperatorDeclarations.end(), operatorDeclaration) == candidateOperatorDeclarations.end())
                        candidateOperatorDeclarations.push_back(operatorDeclaration);
                }
            }
        }
    }

    std::vector<OperatorDeclaration*> viableOperatorDeclarations;

    for (OperatorDeclaration* operatorDeclaration : candidateOperatorDeclarations)
    {
        if (operatorDeclaration->parameterDeclarations.size() == parameters.size())
        {
            if (std::equal(parameters.begin(), parameters.end(),
                           operatorDeclaration->parameterDeclarations.begin(),
                           [](const QualifiedType& qualifiedType,
                              const ParameterDeclaration* parameterDeclaration) {

                               if (!qualifiedType.typeDeclaration) return true; // any type

                               bool scalar = qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::SCALAR &&
                                   qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::SCALAR;

                               return (scalar || qualifiedType.typeDeclaration->getFirstDeclaration() == parameterDeclaration->qualifiedType.typeDeclaration->getFirstDeclaration());
                           }))
            {
                viableOperatorDeclarations.push_back(operatorDeclaration);
            }
        }
    }

    if (viableOperatorDeclarations.empty())
        throw std::runtime_error("No matching function operator " + toString(op) + " found");
    else if (viableOperatorDeclarations.size() == 1)
        return *viableOperatorDeclarations.begin();
    else
    {
        if (parameters.empty()) // two or more functions with zero parameters
            throw std::runtime_error("Ambiguous call to operator " + toString(op));

        for (auto first = viableOperatorDeclarations.begin(); first != viableOperatorDeclarations.end(); ++first)
        {
            bool best = true;
            for (auto second = viableOperatorDeclarations.begin(); second != viableOperatorDeclarations.end(); ++second)
            {
                if (first != second &&
                    compareCallableDeclarations(*first, *second, parameters) != *first)
                {
                    best = false;
                    break;
                }
            }

            if (best) return *first;
        };

        throw std::runtime_error("Ambiguous call to operator " + toString(op));
    }

    return nullptr;
}

ArrayTypeDeclaration* ASTContext::getArrayTypeDeclaration(QualifiedType qualifiedType, uint32_t size)
{
    auto i = arrayTypeDeclarations.find(std::make_pair(qualifiedType, size));

    if (i != arrayTypeDeclarations.end())
    {
        return i->second;
    }
    else
    {
        ArrayTypeDeclaration* result = new ArrayTypeDeclaration();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->elementType = qualifiedType;
        result->size = size;

        arrayTypeDeclarations[std::make_pair(qualifiedType, size)] = result;
        return result;
    }
}

bool ASTContext::isType(const std::vector<Token>& tokens,
                        std::vector<Token>::const_iterator iterator,
                        std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (iterator == tokens.end())
        throw std::runtime_error("Unexpected end of file");

    return iterator->type == Token::Type::KEYWORD_BOOL ||
        iterator->type == Token::Type::KEYWORD_INT ||
        iterator->type == Token::Type::KEYWORD_FLOAT ||
        iterator->type == Token::Type::KEYWORD_DOUBLE ||
        (iterator->type == Token::Type::IDENTIFIER &&
         findTypeDeclaration(iterator->value, declarationScopes));
}

TypeDeclaration* ASTContext::parseType(const std::vector<Token>& tokens,
                                       std::vector<Token>::const_iterator& iterator,
                                       std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (iterator == tokens.end())
        throw std::runtime_error("Unexpected end of file");

    TypeDeclaration* result;

    if (iterator->type == Token::Type::KEYWORD_BOOL)
        result = boolTypeDeclaration;
    else if (iterator->type == Token::Type::KEYWORD_INT)
        result = intTypeDeclaration;
    else if (iterator->type == Token::Type::KEYWORD_FLOAT)
        result = floatTypeDeclaration;
    else if (iterator->type == Token::Type::KEYWORD_DOUBLE)
        throw std::runtime_error("Double precision floating point numbers are not supported");
    else if (iterator->type == Token::Type::IDENTIFIER)
    {
        if (!(result = findTypeDeclaration(iterator->value, declarationScopes)))
            throw std::runtime_error("Invalid type: " + iterator->value);
    }
    else
        throw std::runtime_error("Expected a type name");

    ++iterator;

    return result;
}

bool ASTContext::isDeclaration(const std::vector<Token>& tokens,
                               std::vector<Token>::const_iterator iterator,
                               std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (iterator == tokens.end())
        throw std::runtime_error("Unexpected end of file");

    return iterator->type == Token::Type::KEYWORD_CONST ||
        iterator->type == Token::Type::KEYWORD_STATIC ||
        iterator->type == Token::Type::KEYWORD_VOLATILE ||
        iterator->type == Token::Type::KEYWORD_INLINE ||
        iterator->type == Token::Type::KEYWORD_SIGNED ||
        iterator->type == Token::Type::KEYWORD_UNSIGNED ||
        iterator->type == Token::Type::KEYWORD_STRUCT ||
        isType(tokens, iterator, declarationScopes);
}

Declaration* ASTContext::parseTopLevelDeclaration(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Declaration* declaration;
    if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes, nullptr)))
        throw std::runtime_error("Failed to parse a declaration");

    if (declaration->getDeclarationKind() == Declaration::Kind::CALLABLE)
    {
        CallableDeclaration* callableDeclaration = static_cast<CallableDeclaration*>(declaration);

        // semicolon is not needed after a function definition
        if (!callableDeclaration->body)
        {
            expectToken(Token::Type::SEMICOLON, tokens, iterator);

            ++iterator;
        }
    }
    else
    {
        expectToken(Token::Type::SEMICOLON, tokens, iterator);

        ++iterator;
    }

    return declaration;
}

ASTContext::Specifiers ASTContext::parseSpecifiers(const std::vector<Token>& tokens,
                                                   std::vector<Token>::const_iterator& iterator)
{
    ASTContext::Specifiers result;
    result.isConst = false;
    result.isInline = false;
    result.isStatic = false;
    result.isVolatile = false;

    for (;;)
    {
        if (isToken(Token::Type::KEYWORD_CONST, tokens, iterator))
        {
            ++iterator;
            result.isConst = true;
        }
        else if (isToken(Token::Type::KEYWORD_INLINE, tokens, iterator))
        {
            ++iterator;
            result.isInline = true;
        }
        else if (isToken(Token::Type::KEYWORD_STATIC, tokens, iterator))
        {
            ++iterator;
            result.isStatic = true;
        }
        else if (isToken(Token::Type::KEYWORD_VOLATILE, tokens, iterator))
        {
            ++iterator;
            result.isVolatile = true;
        }
        else break;
    }

    return result;
}

std::vector<std::pair<std::string, std::vector<std::string>>> ASTContext::parseAttributes(const std::vector<Token>& tokens,
                                                                                          std::vector<Token>::const_iterator& iterator)
{
    std::vector<std::pair<std::string, std::vector<std::string>>> result;

    while (isToken(Token::Type::LEFT_BRACKET, tokens, iterator) &&
           isToken(Token::Type::LEFT_BRACKET, tokens, iterator + 1))
    {
        ++iterator;
        ++iterator;

        expectToken(Token::Type::IDENTIFIER, tokens, iterator);

        std::pair<std::string, std::vector<std::string>> attribute;

        attribute.first = iterator->value;

        ++iterator;

        if (isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            ++iterator;

            for (;;)
            {
                expectToken({Token::Type::LITERAL_INT, Token::Type::LITERAL_FLOAT, Token::Type::LITERAL_DOUBLE, Token::Type::LITERAL_CHAR, Token::Type::LITERAL_STRING}, tokens, iterator);

                attribute.second.push_back(iterator->value);
                ++iterator;

                if (!isToken(Token::Type::COMMA, tokens, iterator))
                    break;
            }

            expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);

            ++iterator;
        }

        expectToken(Token::Type::RIGHT_BRACKET, tokens, iterator);

        ++iterator;

        expectToken(Token::Type::RIGHT_BRACKET, tokens, iterator);

        ++iterator;

        result.push_back(attribute);
    }

    return result;
}

Declaration* ASTContext::parseDeclaration(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent)
{
    if (isToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        Declaration* declaration = new Declaration(Declaration::Kind::EMPTY);
        constructs.push_back(std::unique_ptr<Construct>(declaration));

        return declaration;
    }
    else if (isToken(Token::Type::KEYWORD_STRUCT, tokens, iterator))
    {
        ++iterator;

        StructDeclaration* declaration;
        if (!(declaration = parseStructDeclaration(tokens, iterator, declarationScopes, parent)))
            throw std::runtime_error("Failed to parse a structure declaration");

        return declaration;
    }
    /*else if (isToken(Token::Type::KEYWORD_TYPEDEF, tokens, iterator))
    {
        ++iterator;

        TypeDefinitionDeclaration* declaration;
        if (!(declaration = parseTypeDefinitionDeclaration(tokens, iterator, declarationScopes, parent)))
            throw std::runtime_error("Failed to parse a type definition declaration");

        return declaration;
    }*/
    else
    {
        std::vector<std::pair<std::string, std::vector<std::string>>> attributes = parseAttributes(tokens, iterator);
        ASTContext::Specifiers specifiers = parseSpecifiers(tokens, iterator);

        QualifiedType qualifiedType;
        qualifiedType.isConst = specifiers.isConst;
        qualifiedType.isVolatile = specifiers.isVolatile;

        bool isStatic = specifiers.isStatic;
        bool isInline = specifiers.isInline;

        if (isToken(Token::Type::KEYWORD_VOID, tokens, iterator))
        {
            ++iterator;
        }
        else
        {
            if (!(qualifiedType.typeDeclaration = parseType(tokens, iterator, declarationScopes)))
                return nullptr;

            if (qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::STRUCT)
            {
                StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(qualifiedType.typeDeclaration);

                if (!structDeclaration->definition)
                    throw std::runtime_error("Incomplete type " + qualifiedType.typeDeclaration->name);
            }
        }

        specifiers = parseSpecifiers(tokens, iterator);

        if (specifiers.isConst) qualifiedType.isConst = true;
        if (specifiers.isVolatile) qualifiedType.isVolatile = true;

        if (specifiers.isStatic) isStatic = true;
        if (specifiers.isInline) isInline = true;

        if (isToken(Token::Type::KEYWORD_OPERATOR, tokens, iterator))
            throw std::runtime_error("Operator overloads are not supported");

        expectToken(Token::Type::IDENTIFIER, tokens, iterator);

        std::string name = iterator->value;

        ++iterator;

        if (isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator) &&
            (isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator + 1) ||
             isToken(Token::Type::KEYWORD_VOID, tokens, iterator + 1) ||
             isDeclaration(tokens, iterator + 1, declarationScopes)))  // function declaration
        {
            ++iterator;

            FunctionDeclaration* result = new FunctionDeclaration();
            constructs.push_back(std::unique_ptr<Construct>(result));
            result->parent = parent;
            result->qualifiedType = qualifiedType;
            result->isStatic = isStatic;
            result->isInline = isInline;
            result->name = name;

            std::vector<QualifiedType> parameters;

            if (isToken(Token::Type::KEYWORD_VOID, tokens, iterator))
            {
                ++iterator;
            }
            else if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
            {
                for (;;)
                {
                    ParameterDeclaration* parameterDeclaration;
                    if (!(parameterDeclaration = parseParameterDeclaration(tokens, iterator, declarationScopes, result)))
                        return nullptr;

                    result->parameterDeclarations.push_back(parameterDeclaration);
                    parameters.push_back(parameterDeclaration->qualifiedType);

                    if (!isToken(Token::Type::COMMA, tokens, iterator))
                        break;

                    ++iterator;
                }
            }

            expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);

            ++iterator;

            result->previousDeclaration = findFunctionDeclaration(name, declarationScopes, parameters);

            attributes = parseAttributes(tokens, iterator);

            for (std::pair<std::string, std::vector<std::string>>& attribute : attributes)
            {
                if (attribute.first == "program")
                {
                    if (attribute.second.size() == 1)
                    {
                        Program program = Program::NONE;

                        if (attribute.second.front() == "fragment") program = Program::FRAGMENT;
                        else if (attribute.second.front() == "vertex") program = Program::VERTEX;
                        else
                            throw std::runtime_error("Invalid program" + attribute.second.front());

                        result->program = program;
                    }
                    else
                        throw std::runtime_error("Invalid parameters for attribute " + attribute.first);
                }
                else
                    throw std::runtime_error("Invalid attribute " + attribute.first);
            }

            declarationScopes.back().push_back(result);

            // set the definition of the previous declaration
            if (result->previousDeclaration) result->definition = result->previousDeclaration->definition;

            if (isToken(Token::Type::LEFT_BRACE, tokens, iterator))
            {
                // check if only one definition exists
                if (result->definition)
                    throw std::runtime_error("Redefinition of " + result->name);

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
                if (!(result->body = parseCompoundStatement(tokens, iterator, declarationScopes, result)))
                    throw std::runtime_error("Failed to parse a compound statement");

                declarationScopes.pop_back();
            }

            return result;
        }
        else // variable declaration
        {
            if (isInline)
                throw std::runtime_error("Variables can not be inline");

            if (findDeclaration(name, declarationScopes.back()))
                throw std::runtime_error("Redefinition of " + name);

            VariableDeclaration* result = new VariableDeclaration();
            constructs.push_back(std::unique_ptr<VariableDeclaration>(result));
            result->parent = parent;
            result->qualifiedType = qualifiedType;
            result->isStatic = isStatic;
            result->name = name;

            while (isToken(Token::Type::LEFT_BRACKET, tokens, iterator))
            {
                ++iterator;

                expectToken(Token::Type::LITERAL_INT, tokens, iterator);

                int size = std::stoi(iterator->value);

                ++iterator;

                if (size <= 0)
                    throw std::runtime_error("Array size must be greater than zero");

                result->qualifiedType.typeDeclaration = getArrayTypeDeclaration(result->qualifiedType, static_cast<uint32_t>(size));

                expectToken(Token::Type::RIGHT_BRACKET, tokens, iterator);

                ++iterator;
            }

            if (isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
            {
                ++iterator;

                if (!(result->initialization = parseMultiplicationAssignmentExpression(tokens, iterator, declarationScopes, result)))
                    return nullptr;

                if (!result->initialization->qualifiedType.typeDeclaration)
                    throw std::runtime_error("Initialization with a void type");

                // TODO: check for comma and parse multiple expressions

                expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);

                ++iterator;
            }
            else if (isToken(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
            {
                ++iterator;

                if (!(result->initialization = parseMultiplicationAssignmentExpression(tokens, iterator, declarationScopes, result)))
                    return nullptr;

                if (!result->initialization->qualifiedType.typeDeclaration)
                    throw std::runtime_error("Initialization with a void type");
            }

            declarationScopes.back().push_back(result);

            return result;
        }
    }

    return nullptr;
}

StructDeclaration* ASTContext::parseStructDeclaration(const std::vector<Token>& tokens,
                                                      std::vector<Token>::const_iterator& iterator,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                                      Construct* parent)
{
    expectToken(Token::Type::IDENTIFIER, tokens, iterator);

    StructDeclaration* result = new StructDeclaration();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;
    result->name = iterator->value;
    result->previousDeclaration = findStructDeclaration(iterator->value, declarationScopes);

    ++iterator;

    // set the definition of the previous declaration
    if (result->previousDeclaration) result->definition = result->previousDeclaration->definition;

    if (isToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        ++iterator;

        // check if only one definition exists
        if (result->definition)
            throw std::runtime_error("Redefinition of " + result->name);

        result->definition = result;

        // set the definition pointer of all previous declarations
        Declaration* previousDeclaration = result->previousDeclaration;
        while (previousDeclaration)
        {
            previousDeclaration->definition = result;
            previousDeclaration = previousDeclaration->previousDeclaration;
        }

        for (;;)
        {
            if (isToken(Token::Type::RIGHT_BRACE, tokens, iterator))
            {
                ++iterator;

                declarationScopes.back().push_back(result);
                break;
            }
            else
            {
                Declaration* memberDeclaration;
                if (!(memberDeclaration = parseMemberDeclaration(tokens, iterator, declarationScopes, result)))
                    return nullptr;

                expectToken(Token::Type::SEMICOLON, tokens, iterator);

                if (result->findMemberDeclaration(memberDeclaration->name))
                    throw std::runtime_error("Redefinition of member " + memberDeclaration->name);

                ++iterator;

                memberDeclaration->parent = result;

                result->memberDeclarations.push_back(memberDeclaration);
            }
        }
    }

    declarationScopes.back().push_back(result);

    addOperatorDeclaration(Operator::ASSIGNMENT, result, {result, result}, declarationScopes);
    addOperatorDeclaration(Operator::EQUALITY, result, {result, result}, declarationScopes);
    addOperatorDeclaration(Operator::INEQUALITY, result, {result, result}, declarationScopes);

    return result;
}

Declaration* ASTContext::parseMemberDeclaration(const std::vector<Token>& tokens,
                                                std::vector<Token>::const_iterator& iterator,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                Construct* parent)
{
    if (isToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        Declaration* result = new Declaration(Declaration::Kind::EMPTY);
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;

        return result;
    }
    else
    {
        FieldDeclaration* result = new FieldDeclaration();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;

        std::vector<std::pair<std::string, std::vector<std::string>>> attributes = parseAttributes(tokens, iterator);
        ASTContext::Specifiers specifiers = parseSpecifiers(tokens, iterator);

        result->qualifiedType.isConst = specifiers.isConst;
        result->qualifiedType.isVolatile = specifiers.isVolatile;

        bool isStatic = specifiers.isStatic;
        bool isInline = specifiers.isInline;

        if (!(result->qualifiedType.typeDeclaration = parseType(tokens, iterator, declarationScopes)))
            return nullptr;

        if (result->qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::STRUCT)
        {
            StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->qualifiedType.typeDeclaration);

            if (!structDeclaration->definition)
                throw std::runtime_error("Incomplete type " + result->qualifiedType.typeDeclaration->name);
        }

        specifiers = parseSpecifiers(tokens, iterator);

        if (specifiers.isConst) result->qualifiedType.isConst = true;
        if (specifiers.isVolatile) result->qualifiedType.isVolatile = true;

        if (specifiers.isStatic) isStatic = true;
        if (specifiers.isInline) isInline = true;

        if (isStatic)
            throw std::runtime_error("Members can not be static");

        if (isInline)
            throw std::runtime_error("Members can not be inline");

        expectToken(Token::Type::IDENTIFIER, tokens, iterator);

        result->name = iterator->value;

        ++iterator;

        attributes = parseAttributes(tokens, iterator);

        while (isToken(Token::Type::LEFT_BRACKET, tokens, iterator))
        {
            ++iterator;

            expectToken(Token::Type::LITERAL_INT, tokens, iterator);

            int size = std::stoi(iterator->value);

            ++iterator;

            if (size <= 0)
                throw std::runtime_error("Array size must be greater than zero");

            result->qualifiedType.typeDeclaration = getArrayTypeDeclaration(result->qualifiedType, static_cast<uint32_t>(size));

            expectToken(Token::Type::RIGHT_BRACKET, tokens, iterator);

            ++iterator;
        }

        attributes = parseAttributes(tokens, iterator);

        for (std::pair<std::string, std::vector<std::string>>& attribute : attributes)
        {
            if (attribute.first == "semantic")
            {
                if (attribute.second.size() == 1)
                {
                    Semantic semantic = Semantic::NONE;

                    // TODO: find slot number
                    if (attribute.second.front() == "binormal") semantic = Semantic::BINORMAL;
                    else if (attribute.second.front() == "blend_indices") semantic = Semantic::BLEND_INDICES;
                    else if (attribute.second.front() == "blend_weight") semantic = Semantic::BLEND_WEIGHT;
                    else if (attribute.second.front() == "color") semantic = Semantic::COLOR;
                    else if (attribute.second.front() == "normal") semantic = Semantic::NORMAL;
                    else if (attribute.second.front() == "position") semantic = Semantic::POSITION;
                    else if (attribute.second.front() == "position_transformed") semantic = Semantic::POSITION_TRANSFORMED;
                    else if (attribute.second.front() == "point_size") semantic = Semantic::POINT_SIZE;
                    else if (attribute.second.front() == "tangent") semantic = Semantic::TANGENT;
                    else if (attribute.second.front() == "texture_coordinates") semantic = Semantic::TEXTURE_COORDINATES;
                    else
                        throw std::runtime_error("Invalid semantic " + attribute.second.front());

                    result->semantic = semantic;
                }
                else
                    throw std::runtime_error("Invalid parameters for attribute " + attribute.first);
            }
            else
                throw std::runtime_error("Invalid attribute " + attribute.first);
        }

        return result;
    }

    return nullptr;
}

ParameterDeclaration* ASTContext::parseParameterDeclaration(const std::vector<Token>& tokens,
                                                            std::vector<Token>::const_iterator& iterator,
                                                            std::vector<std::vector<Declaration*>>& declarationScopes,
                                                            Construct* parent)
{
    ParameterDeclaration* result = new ParameterDeclaration();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    ASTContext::Specifiers specifiers = parseSpecifiers(tokens, iterator);

    result->qualifiedType.isConst = specifiers.isConst;
    result->qualifiedType.isVolatile = specifiers.isVolatile;

    bool isStatic = specifiers.isStatic;
    bool isInline = specifiers.isInline;

    if (!(result->qualifiedType.typeDeclaration = parseType(tokens, iterator, declarationScopes)))
        return nullptr;

    if (result->qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::STRUCT)
    {
        StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->qualifiedType.typeDeclaration);

        if (!structDeclaration->definition)
            throw std::runtime_error("Incomplete type " + result->qualifiedType.typeDeclaration->name);
    }

    specifiers = parseSpecifiers(tokens, iterator);

    if (specifiers.isConst) result->qualifiedType.isConst = true;
    if (specifiers.isVolatile) result->qualifiedType.isVolatile = true;

    if (specifiers.isStatic) isStatic = true;
    if (specifiers.isInline) isInline = true;

    if (isStatic)
        throw std::runtime_error("Parameters can not be static");

    if (isInline)
        throw std::runtime_error("Parameters can not be inline");

    expectToken(Token::Type::IDENTIFIER, tokens, iterator);

    result->name = iterator->value;

    ++iterator;

    while (isToken(Token::Type::LEFT_BRACKET, tokens, iterator))
    {
        ++iterator;

        expectToken(Token::Type::LITERAL_INT, tokens, iterator);

        int size = std::stoi(iterator->value);

        ++iterator;

        if (size <= 0)
            throw std::runtime_error("Array size must be greater than zero");

        result->qualifiedType.typeDeclaration = getArrayTypeDeclaration(result->qualifiedType, static_cast<uint32_t>(size));

        expectToken(Token::Type::RIGHT_BRACKET, tokens, iterator);

        ++iterator;
    }

    return result;
}

/*TypeDefinitionDeclaration* ASTContext::parseTypeDefinitionDeclaration(const std::vector<Token>& tokens,
                                                                        std::vector<Token>::const_iterator& iterator,
                                                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                                                        Construct* parent)
{
    throw std::runtime_error("Typedef is not supported");
    return nullptr;

    TypeDefinitionDeclaration* result = new TypeDefinitionDeclaration();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;
    result->kind = Construct::Kind::DECLARATION;
    result->declarationKind = Declaration::Kind::TYPE;
    result->typeKind = TypeDeclaration::Kind::TYPE_DEFINITION;
    result->Declaration = findTypeDeclaration(iterator->value, declarationScopes);

    if (!(result->qualifiedType.typeDeclaration = parseType(tokens, iterator, declarationScopes)))
        return nullptr;

    if (result->qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::STRUCT)
    {
        StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->qualifiedType.typeDeclaration);

        if (!structDeclaration->hasDefinition)
            throw std::runtime_error("Incomplete type " + result->qualifiedType.typeDeclaration->name);
    }

    expectToken(Token::Type::IDENTIFIER, tokens, iterator);

    result->name = iterator->value;

    ++iterator;

    expectToken(Token::Type::SEMICOLON, tokens, iterator);

    ++iterator;

    return result;
}*/

Statement* ASTContext::parseStatement(const std::vector<Token>& tokens,
                                      std::vector<Token>::const_iterator& iterator,
                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                      Construct* parent)
{
    if (isToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        return parseCompoundStatement(tokens, iterator, declarationScopes, parent);
    }
    else if (isToken(Token::Type::KEYWORD_IF, tokens, iterator))
    {
        return parseIfStatement(tokens, iterator, declarationScopes, parent);
    }
    else if (isToken(Token::Type::KEYWORD_FOR, tokens, iterator))
    {
        return parseForStatement(tokens, iterator, declarationScopes, parent);
    }
    else if (isToken(Token::Type::KEYWORD_SWITCH, tokens, iterator))
    {
        return parseSwitchStatement(tokens, iterator, declarationScopes, parent);
    }
    else if (isToken(Token::Type::KEYWORD_CASE, tokens, iterator))
    {
        return parseCaseStatement(tokens, iterator, declarationScopes, parent);
    }
    else if (isToken(Token::Type::KEYWORD_DEFAULT, tokens, iterator))
    {
        return parseDefaultStatement(tokens, iterator, declarationScopes, parent);
    }
    else if (isToken(Token::Type::KEYWORD_WHILE, tokens, iterator))
    {
        return parseWhileStatement(tokens, iterator, declarationScopes, parent);
    }
    else if (isToken(Token::Type::KEYWORD_DO, tokens, iterator))
    {
        return parseDoStatement(tokens, iterator, declarationScopes, parent);
    }
    else if (isToken(Token::Type::KEYWORD_BREAK, tokens, iterator))
    {
        ++iterator;

        BreakStatement* result = new BreakStatement();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;

        expectToken(Token::Type::SEMICOLON, tokens, iterator);

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::KEYWORD_CONTINUE, tokens, iterator))
    {
        ++iterator;

        ContinueStatement* result = new ContinueStatement();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;

        expectToken(Token::Type::SEMICOLON, tokens, iterator);

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::KEYWORD_RETURN, tokens, iterator))
    {
        ++iterator;

        ReturnStatement* result = new ReturnStatement();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;

        if (!(result->result = parseExpression(tokens, iterator, declarationScopes, result)))
            throw std::runtime_error("Expected an expression");

        expectToken(Token::Type::SEMICOLON, tokens, iterator);

        ++iterator;

        return result;
    }
    else if (isDeclaration(tokens, iterator, declarationScopes))
    {
        DeclarationStatement* result = new DeclarationStatement();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;

        if (!(result->declaration = parseDeclaration(tokens, iterator, declarationScopes, result)))
            return nullptr;

        if (result->declaration->getDeclarationKind() != Declaration::Kind::VARIABLE)
            throw std::runtime_error("Expected a variable declaration");

        expectToken(Token::Type::SEMICOLON, tokens, iterator);

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        ++iterator;

        Statement* statement = new Statement(Statement::Kind::EMPTY);
        constructs.push_back(std::unique_ptr<Construct>(statement));
        statement->parent = parent;

        return statement;
    }
    else
    {
        ExpressionStatement* result = new ExpressionStatement();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;

        if (!(result->expression = parseExpression(tokens, iterator, declarationScopes, result)))
            return nullptr;

        expectToken(Token::Type::SEMICOLON, tokens, iterator);

        ++iterator;

        return result;
    }

    return nullptr;
}

CompoundStatement* ASTContext::parseCompoundStatement(const std::vector<Token>& tokens,
                                                      std::vector<Token>::const_iterator& iterator,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                                      Construct* parent)
{
    expectToken(Token::Type::LEFT_BRACE, tokens, iterator);

    ++iterator;

    declarationScopes.push_back(std::vector<Declaration*>());

    CompoundStatement* result = new CompoundStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    for (;;)
    {
        if (isToken(Token::Type::RIGHT_BRACE, tokens, iterator))
        {
            ++iterator;
            break;
        }
        else
        {
            Statement* statement;
            if (!(statement = parseStatement(tokens, iterator, declarationScopes, result)))
                throw std::runtime_error("Failed to parse a statement");

            result->statements.push_back(statement);
        }
    }

    declarationScopes.pop_back();

    return result;
}

IfStatement* ASTContext::parseIfStatement(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent)
{
    expectToken(Token::Type::KEYWORD_IF, tokens, iterator);

    ++iterator;

    IfStatement* result = new IfStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    expectToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator);

    ++iterator;

    // TODO: add implicit cast to bool
    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes, result)))
            return nullptr;

        if (declaration->getDeclarationKind() != Declaration::Kind::VARIABLE &&
            declaration->getDeclarationKind() != Declaration::Kind::PARAMETER)
            throw std::runtime_error("Expected a variable declaration");

        result->condition = declaration;
    }
    else
    {
        if (!(result->condition = parseExpression(tokens, iterator, declarationScopes, result)))
            return nullptr;
    }

    expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);

    ++iterator;

    Statement* statement;
    if (!(statement = parseStatement(tokens, iterator, declarationScopes, result)))
        throw std::runtime_error("Failed to parse the statement");

    result->body = statement;

    if (isToken(Token::Type::KEYWORD_ELSE, tokens, iterator))
    {
        ++iterator;

        if (!(statement = parseStatement(tokens, iterator, declarationScopes, result)))
            throw std::runtime_error("Failed to parse the statement");

        result->elseBody = statement;
    }

    return result;
}

ForStatement* ASTContext::parseForStatement(const std::vector<Token>& tokens,
                                            std::vector<Token>::const_iterator& iterator,
                                            std::vector<std::vector<Declaration*>>& declarationScopes,
                                            Construct* parent)
{
    expectToken(Token::Type::KEYWORD_FOR, tokens, iterator);

    ++iterator;

    ForStatement* result = new ForStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    expectToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator);

    ++iterator;

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes, result)))
            return nullptr;

        if (declaration->getDeclarationKind() != Declaration::Kind::VARIABLE &&
            declaration->getDeclarationKind() != Declaration::Kind::PARAMETER)
            throw std::runtime_error("Expected a variable declaration");

        result->condition = declaration;

        expectToken(Token::Type::SEMICOLON, tokens, iterator);

        ++iterator;
    }
    else if (isToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        ++iterator;

        result->initialization = nullptr;
    }
    else
    {
        if (!(result->initialization = parseExpression(tokens, iterator, declarationScopes, result)))
            return nullptr;

        expectToken(Token::Type::SEMICOLON, tokens, iterator);

        ++iterator;
    }

    // TODO: add implicit cast to bool
    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes, result)))
            return nullptr;

        if (declaration->getDeclarationKind() != Declaration::Kind::VARIABLE &&
            declaration->getDeclarationKind() != Declaration::Kind::PARAMETER)
            throw std::runtime_error("Expected a variable declaration");

        result->condition = declaration;

        expectToken(Token::Type::SEMICOLON, tokens, iterator);

        ++iterator;
    }
    else if (isToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        ++iterator;

        result->condition = nullptr;
    }
    else
    {
        if (!(result->condition = parseExpression(tokens, iterator, declarationScopes, result)))
            return nullptr;

        expectToken(Token::Type::SEMICOLON, tokens, iterator);

        ++iterator;
    }

    if (isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        ++iterator;

        result->increment = nullptr;
    }
    else
    {
        if (!(result->increment = parseExpression(tokens, iterator, declarationScopes, result)))
            return nullptr;

        expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);

        ++iterator;
    }

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes, result)))
        return nullptr;

    return result;
}

SwitchStatement* ASTContext::parseSwitchStatement(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  Construct* parent)
{
    expectToken(Token::Type::KEYWORD_SWITCH, tokens, iterator);

    ++iterator;

    SwitchStatement* result = new SwitchStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    expectToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator);

    ++iterator;

    // TODO: add implicit cast to int
    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes, result)))
            return nullptr;

        if (declaration->getDeclarationKind() != Declaration::Kind::VARIABLE &&
            declaration->getDeclarationKind() != Declaration::Kind::PARAMETER)
            throw std::runtime_error("Expected a variable declaration");

        result->condition = declaration;
    }
    else
    {
        if (!(result->condition = parseExpression(tokens, iterator, declarationScopes, result)))
            return nullptr;
    }

    expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);

    ++iterator;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes, result)))
        return nullptr;

    return result;
}

CaseStatement* ASTContext::parseCaseStatement(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent)
{
    expectToken(Token::Type::KEYWORD_CASE, tokens, iterator);

    ++iterator;

    CaseStatement* result = new CaseStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    if (!(result->condition = parseExpression(tokens, iterator, declarationScopes, result)))
        throw std::runtime_error("Expected an expression");

    expectToken(Token::Type::COLON, tokens, iterator);

    ++iterator;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes, result)))
        return nullptr;

    return result;
}

DefaultStatement* ASTContext::parseDefaultStatement(const std::vector<Token>& tokens,
                                                    std::vector<Token>::const_iterator& iterator,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                                    Construct* parent)
{
    expectToken(Token::Type::KEYWORD_DEFAULT, tokens, iterator);

    ++iterator;

    DefaultStatement* result = new DefaultStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    expectToken(Token::Type::COLON, tokens, iterator);

    ++iterator;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes, result)))
        return nullptr;

    return result;
}

WhileStatement* ASTContext::parseWhileStatement(const std::vector<Token>& tokens,
                                                std::vector<Token>::const_iterator& iterator,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                Construct* parent)
{
    expectToken(Token::Type::KEYWORD_WHILE, tokens, iterator);

    ++iterator;

    WhileStatement* result = new WhileStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    expectToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator);

    ++iterator;

    // TODO: add implicit cast to bool
    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes, result)))
            return nullptr;

        if (declaration->getDeclarationKind() != Declaration::Kind::VARIABLE &&
            declaration->getDeclarationKind() != Declaration::Kind::PARAMETER)
            throw std::runtime_error("Expected a variable declaration");

        result->condition = declaration;
    }
    else
    {
        if (!(result->condition = parseExpression(tokens, iterator, declarationScopes, result)))
            return nullptr;
    }

    expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);

    ++iterator;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes, result)))
        return nullptr;

    return result;
}

DoStatement* ASTContext::parseDoStatement(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent)
{
    expectToken(Token::Type::KEYWORD_DO, tokens, iterator);

    ++iterator;

    DoStatement* result = new DoStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes, result)))
        return nullptr;

    expectToken(Token::Type::KEYWORD_WHILE, tokens, iterator);

    ++iterator;

    expectToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator);

    ++iterator;

    // TODO: add implicit cast to bool
    // expression
    if (!(result->condition = parseExpression(tokens, iterator, declarationScopes, result)))
        return nullptr;

    expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);

    ++iterator;

    expectToken(Token::Type::SEMICOLON, tokens, iterator);

    ++iterator;

    return result;
}

CastExpression* ASTContext::addImplicitCast(Expression* expression,
                                            TypeDeclaration* typeDeclaration)
{
    CastExpression* result = new CastExpression(CastExpression::Kind::IMPLICIT);
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = expression->parent;
    result->qualifiedType.typeDeclaration = typeDeclaration;

    result->expression = expression;
    expression->parent = result;

    return result;
}

Expression* ASTContext::parsePrimaryExpression(const std::vector<Token>& tokens,
                                               std::vector<Token>::const_iterator& iterator,
                                               std::vector<std::vector<Declaration*>>& declarationScopes,
                                               Construct* parent)
{
    if (isToken(Token::Type::LITERAL_INT, tokens, iterator))
    {
        IntegerLiteralExpression* result = new IntegerLiteralExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;
        result->qualifiedType.typeDeclaration = intTypeDeclaration;
        result->isLValue = false;
        result->value = strtoll(iterator->value.c_str(), nullptr, 0);

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::LITERAL_FLOAT, tokens, iterator))
    {
        FloatingPointLiteralExpression* result = new FloatingPointLiteralExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;
        result->qualifiedType.typeDeclaration = floatTypeDeclaration;
        result->isLValue = false;
        result->value = strtod(iterator->value.c_str(), nullptr);

        ++iterator;

        return result;
    }
    else if (isToken({Token::Type::LITERAL_DOUBLE, Token::Type::KEYWORD_DOUBLE}, tokens, iterator))
    {
        throw std::runtime_error("Double precision floating point numbers are not supported");
    }
    else if (isToken(Token::Type::LITERAL_STRING, tokens, iterator))
    {
        StringLiteralExpression* result = new StringLiteralExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;
        result->qualifiedType.typeDeclaration = stringTypeDeclaration;
        result->isLValue = false;
        result->value = iterator->value;

        ++iterator;

        return result;
    }
    else if (isToken({Token::Type::KEYWORD_TRUE, Token::Type::KEYWORD_FALSE}, tokens, iterator))
    {
        BooleanLiteralExpression* result = new BooleanLiteralExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;
        result->qualifiedType.typeDeclaration = boolTypeDeclaration;
        result->isLValue = false;
        result->value = (iterator->type == Token::Type::KEYWORD_TRUE);

        ++iterator;

        return result;
    }
    else if (isToken({Token::Type::KEYWORD_BOOL, Token::Type::KEYWORD_INT, Token::Type::KEYWORD_FLOAT}, tokens, iterator))
    {
        // TODO: parse type and fix precedence
        CastExpression* result = new CastExpression(CastExpression::Kind::FUNCTIONAL);
        constructs.push_back(std::unique_ptr<Construct>(result));

        if (isToken(Token::Type::KEYWORD_BOOL, tokens, iterator)) result->qualifiedType.typeDeclaration = boolTypeDeclaration;
        else if(isToken(Token::Type::KEYWORD_INT, tokens, iterator)) result->qualifiedType.typeDeclaration = intTypeDeclaration;
        else if(isToken(Token::Type::KEYWORD_FLOAT, tokens, iterator)) result->qualifiedType.typeDeclaration = floatTypeDeclaration;

        ++iterator;

        expectToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator);

        ++iterator;

        result->expression = parseMultiplicationAssignmentExpression(tokens, iterator, declarationScopes, result);

        expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        ++iterator;

        InitializerListExpression* result = new InitializerListExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));

        QualifiedType qualifiedType;

        for (;;)
        {
            Expression* expression;
            if (!(expression = parseMultiplicationAssignmentExpression(tokens, iterator, declarationScopes, result)))
                return nullptr;

            if (!qualifiedType.typeDeclaration)
            {
                qualifiedType.typeDeclaration = expression->qualifiedType.typeDeclaration;
            }
            else
            {
                if (qualifiedType.typeDeclaration != expression->qualifiedType.typeDeclaration)
                {
                    // TODO: implement type narrowing
                    throw std::runtime_error("Expression type does not match previous expressions in initializer list");
                }
            }

            result->expressions.push_back(expression);

            if (!isToken(Token::Type::COMMA, tokens, iterator))
                break;

            ++iterator;
        }

        expectToken(Token::Type::RIGHT_BRACE, tokens, iterator);

        result->qualifiedType.typeDeclaration = getArrayTypeDeclaration(qualifiedType, static_cast<uint32_t>(result->expressions.size()));

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::string name = iterator->value;

        ++iterator;

        if (isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            ++iterator;

            TypeDeclaration* typeDeclaration;

            if ((typeDeclaration = findTypeDeclaration(name, declarationScopes)))
            {
                TemporaryObjectExpression* result = new TemporaryObjectExpression();
                constructs.push_back(std::unique_ptr<Construct>(result));
                result->parent = parent;
                result->qualifiedType.typeDeclaration = typeDeclaration;
                result->isLValue = false;

                std::vector<QualifiedType> parameters;

                if (isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator)) // no arguments
                {
                    ++iterator;
                }
                else
                {
                    for (;;)
                    {
                        Expression* parameter;

                        if (!(parameter = parseMultiplicationAssignmentExpression(tokens, iterator, declarationScopes, result)))
                            return nullptr;

                        result->parameters.push_back(parameter);
                        parameters.push_back(parameter->qualifiedType);

                        if (isToken(Token::Type::COMMA, tokens, iterator))
                            ++iterator;
                        else
                            break;
                    }

                    expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);

                    ++iterator;
                }

                if (typeDeclaration->getTypeKind() != TypeDeclaration::Kind::STRUCT)
                    throw std::runtime_error("Expected a struct type");

                StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(typeDeclaration);

                if (!(result->constructorDeclaration = structDeclaration->findConstructorDeclaration(parameters)))
                    throw std::runtime_error("No matching constructor found");

                return result;
            }
            else
            {
                CallExpression* result = new CallExpression();
                constructs.push_back(std::unique_ptr<Construct>(result));
                result->parent = parent;

                std::vector<QualifiedType> parameters;

                if (isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator)) // no arguments
                {
                    ++iterator;
                }
                else
                {
                    for (;;)
                    {
                        Expression* parameter;

                        if (!(parameter = parseMultiplicationAssignmentExpression(tokens, iterator, declarationScopes, result)))
                            return nullptr;

                        result->parameters.push_back(parameter);
                        parameters.push_back(parameter->qualifiedType);

                        if (isToken(Token::Type::COMMA, tokens, iterator))
                            ++iterator;
                        else
                            break;
                    }

                    expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);

                    ++iterator;
                }

                DeclarationReferenceExpression* declRefExpression = new DeclarationReferenceExpression();
                constructs.push_back(std::unique_ptr<Construct>(declRefExpression));
                declRefExpression->parent = result;

                FunctionDeclaration* functionDeclaration = resolveFunctionDeclaration(name, declarationScopes, parameters);

                if (!functionDeclaration)
                    throw std::runtime_error("Invalid function reference: " + name);

                declRefExpression->declaration = functionDeclaration;
                declRefExpression->qualifiedType = functionDeclaration->qualifiedType;
                declRefExpression->isLValue = true;
                result->declarationReference = declRefExpression;
                result->qualifiedType = functionDeclaration->qualifiedType;
                result->isLValue = false;

                return result;
            }
        }
        else
        {
            DeclarationReferenceExpression* result = new DeclarationReferenceExpression();
            constructs.push_back(std::unique_ptr<Construct>(result));
            result->parent = parent;
            result->declaration = findDeclaration(name, declarationScopes);

            if (!result->declaration)
                throw std::runtime_error("Invalid declaration reference: " + name);

            switch (result->declaration->getDeclarationKind())
            {
                case Declaration::Kind::TYPE:
                {
                    TypeDeclaration* typeDeclaration = static_cast<TypeDeclaration*>(result->declaration);
                    result->qualifiedType.typeDeclaration = typeDeclaration;
                    result->isLValue = false;
                    break;
                }
                case Declaration::Kind::VARIABLE:
                {
                    VariableDeclaration* variableDeclaration = static_cast<VariableDeclaration*>(result->declaration);
                    result->qualifiedType = variableDeclaration->qualifiedType;
                    result->isLValue = true;
                    break;
                }
                case Declaration::Kind::PARAMETER:
                {
                    ParameterDeclaration* parameterDeclaration = static_cast<ParameterDeclaration*>(result->declaration);
                    result->qualifiedType = parameterDeclaration->qualifiedType;
                    result->isLValue = false;
                    break;
                }
                default:
                    throw std::runtime_error("Invalid declaration reference " + name);
            }

            return result;
        }
    }
    else if (isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        ++iterator;

        if (isType(tokens, iterator, declarationScopes))
        {
            CastExpression* result = new CastExpression(CastExpression::Kind::C_STYLE);
            constructs.push_back(std::unique_ptr<Construct>(result));
            result->parent = parent;

            // TODO: parse qualifiers
            result->qualifiedType.typeDeclaration = parseType(tokens, iterator, declarationScopes);

            expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);
            ++iterator;

            result->expression = parseExpression(tokens, iterator, declarationScopes, result);
            result->isLValue = false;

            return result;
        }
        else
        {
            ParenExpression* result = new ParenExpression();
            constructs.push_back(std::unique_ptr<Construct>(result));
            result->parent = parent;

            if (!(result->expression = parseExpression(tokens, iterator, declarationScopes, result)))
                return nullptr;

            expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);

            ++iterator;

            result->qualifiedType = result->expression->qualifiedType;
            result->isLValue = result->expression->isLValue;

            return result;
        }
    }
    else if (isToken({Token::Type::KEYWORD_CONST_CAST,
        Token::Type::KEYWORD_DYNAMIC_CAST,
        Token::Type::KEYWORD_REINTERPRET_CAST,
        Token::Type::KEYWORD_STATIC_CAST}, tokens, iterator))
    {
        CastExpression::Kind castKind = CastExpression::Kind::NONE;
        
        switch (iterator->type)
        {
            case Token::Type::KEYWORD_CONST_CAST: castKind = CastExpression::Kind::CONST; break;
            case Token::Type::KEYWORD_DYNAMIC_CAST: castKind = CastExpression::Kind::DYNAMIC; break;
            case Token::Type::KEYWORD_REINTERPRET_CAST: castKind = CastExpression::Kind::REINTERPRET; break;
            case Token::Type::KEYWORD_STATIC_CAST: castKind = CastExpression::Kind::STATIC; break;
            default: break;
        }
        
        CastExpression* result = new CastExpression(castKind);
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;
        
        ++iterator;
        
        expectToken(Token::Type::OPERATOR_LESS_THAN, tokens, iterator);
        ++iterator;
        
        // TODO: parse qualifiers
        result->qualifiedType.typeDeclaration = parseType(tokens, iterator, declarationScopes);
        
        expectToken(Token::Type::OPERATOR_GREATER_THAN, tokens, iterator);
        ++iterator;
        
        expectToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator);
        ++iterator;
        
        if (!(result->expression = parseExpression(tokens, iterator, declarationScopes, result)))
            return nullptr;
        
        expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);
        ++iterator;
        
        result->isLValue = false;
        
        return result;
    }
    else
        throw std::runtime_error("Expected an expression");

    return nullptr;
}

Expression* ASTContext::parseSubscriptExpression(const std::vector<Token>& tokens,
                                                 std::vector<Token>::const_iterator& iterator,
                                                 std::vector<std::vector<Declaration*>>& declarationScopes,
                                                 Construct* parent)
{
    Expression* result;
    if (!(result = parsePrimaryExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken(Token::Type::LEFT_BRACKET, tokens, iterator))
    {
        ++iterator;

        ArraySubscriptExpression* expression = new ArraySubscriptExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;
        expression->expression = result;

        if (!result->qualifiedType.typeDeclaration)
            throw std::runtime_error("Subscript expression with a void type");

        if (result->qualifiedType.typeDeclaration->getTypeKind() != TypeDeclaration::Kind::ARRAY)
            throw std::runtime_error("Subscript value is not an array");

        if (!(expression->subscript = parseExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        if (!expression->subscript->qualifiedType.typeDeclaration ||
            expression->subscript->qualifiedType.typeDeclaration->getTypeKind() != TypeDeclaration::Kind::SCALAR)
            throw std::runtime_error("Subscript is not an integer");

        ScalarTypeDeclaration* scalarType = static_cast<ScalarTypeDeclaration*>(expression->subscript->qualifiedType.typeDeclaration);

        if (scalarType->getScalarTypeKind() != ScalarTypeDeclaration::Kind::BOOLEAN &&
            scalarType->getScalarTypeKind() != ScalarTypeDeclaration::Kind::INTEGER)
            throw std::runtime_error("Subscript is not an integer");

        if (scalarType->getScalarTypeKind() != ScalarTypeDeclaration::Kind::INTEGER)
            expression->subscript = addImplicitCast(expression->subscript, intTypeDeclaration);

        expectToken(Token::Type::RIGHT_BRACKET, tokens, iterator);

        ++iterator;

        ArrayTypeDeclaration* arrayTypeDeclaration = static_cast<ArrayTypeDeclaration*>(result->qualifiedType.typeDeclaration);

        expression->qualifiedType = arrayTypeDeclaration->elementType;
        expression->isLValue = true;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseMemberExpression(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent)
{
    Expression* result;
    if (!(result = parseSubscriptExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::OPERATOR_DOT, Token::Type::OPERATOR_ARROW}, tokens, iterator))
    {
        if (isToken(Token::Type::OPERATOR_ARROW, tokens, iterator))
            throw std::runtime_error("Pointer member access is not supported");

        ++iterator;

        MemberExpression* expression = new MemberExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;
        expression->expression = result;

        if (!result->qualifiedType.typeDeclaration)
            throw std::runtime_error("Expression has a void type");

        if (result->qualifiedType.typeDeclaration->getTypeKind() != TypeDeclaration::Kind::STRUCT)
        {
            throw std::runtime_error(result->qualifiedType.typeDeclaration->name + " is not a structure");
            return nullptr;
        }

        StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->qualifiedType.typeDeclaration);

        expectToken(Token::Type::IDENTIFIER, tokens, iterator);

        Declaration* memberDeclaration = structDeclaration->findMemberDeclaration(iterator->value);

        if (!memberDeclaration)
        {
            throw std::runtime_error("Structure " + structDeclaration->name +  " has no member " + iterator->value);
            return nullptr;
        }

        if (memberDeclaration->getDeclarationKind() != Declaration::Kind::FIELD)
            throw std::runtime_error(iterator->value + " is not a field");

        expression->fieldDeclaration = static_cast<FieldDeclaration*>(memberDeclaration);

        ++iterator;

        expression->qualifiedType = expression->fieldDeclaration->qualifiedType;
        if (result->qualifiedType.isConst) expression->qualifiedType.isConst = true;
        expression->isLValue = true;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseSignExpression(const std::vector<Token>& tokens,
                                            std::vector<Token>::const_iterator& iterator,
                                            std::vector<std::vector<Declaration*>>& declarationScopes,
                                            Construct* parent)
{
    if (isToken({Token::Type::OPERATOR_PLUS, Token::Type::OPERATOR_MINUS}, tokens, iterator))
    {
        UnaryOperatorExpression* result = new UnaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;

        Operator op = Operator::NONE;

        if (iterator->type == Token::Type::OPERATOR_PLUS)
            op = Operator::POSITIVE;
        else if (iterator->type == Token::Type::OPERATOR_MINUS)
            op = Operator::NEGATIVE;

        ++iterator;

        if (!(result->expression = parseMemberExpression(tokens, iterator, declarationScopes, result)))
            return nullptr;

        result->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes, {result->expression->qualifiedType});

        if (result->expression->qualifiedType.typeDeclaration == boolTypeDeclaration)
            result->expression = addImplicitCast(result->expression, intTypeDeclaration);

        result->qualifiedType = result->operatorDeclaration->qualifiedType;
        result->isLValue = false;

        return result;
    }
    else
    {
        Expression* result;
        if (!(result = parseMemberExpression(tokens, iterator, declarationScopes, parent)))
            return nullptr;

        return result;
    }
}

Expression* ASTContext::parseNotExpression(const std::vector<Token>& tokens,
                                           std::vector<Token>::const_iterator& iterator,
                                           std::vector<std::vector<Declaration*>>& declarationScopes,
                                           Construct* parent)
{
    if (isToken({Token::Type::OPERATOR_NOT, Token::Type::KEYWORD_NOT}, tokens, iterator))
    {
        UnaryOperatorExpression* result = new UnaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;

        Operator op = Operator::NEGATION;

        ++iterator;

        if (!(result->expression = parseExpression(tokens, iterator, declarationScopes, result)))
            return nullptr;

        result->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes, {result->expression->qualifiedType});

        if (result->expression->qualifiedType.typeDeclaration != boolTypeDeclaration)
            result->expression = addImplicitCast(result->expression, boolTypeDeclaration);

        result->qualifiedType = result->operatorDeclaration->qualifiedType;
        result->isLValue = false;

        return result;
    }
    else
    {
        Expression* result;
        if (!(result = parseSignExpression(tokens, iterator, declarationScopes, parent)))
            return nullptr;
        return result;
    }
}

Expression* ASTContext::parseSizeofExpression(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent)
{
    if (isToken(Token::Type::KEYWORD_SIZEOF, tokens, iterator))
    {
        SizeofExpression* result = new SizeofExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;
        
        ++iterator;
        
        expectToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator);
        ++iterator;
        
        if (isType(tokens, iterator, declarationScopes))
        {
            if (!(result->type = parseType(tokens, iterator, declarationScopes)))
                return nullptr;
        }
        else
        {
            if (!(result->expression = parseExpression(tokens, iterator, declarationScopes, result)))
                return nullptr;
        }
        
        expectToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator);
        ++iterator;
        
        result->qualifiedType.typeDeclaration = unsignedIntTypeDeclaration;
        result->isLValue = false;
        
        return result;
    }
    else
    {
        Expression* result;
        if (!(result = parseNotExpression(tokens, iterator, declarationScopes, parent)))
            return nullptr;
        return result;
    }
}

Expression* ASTContext::parseMultiplicationExpression(const std::vector<Token>& tokens,
                                                      std::vector<Token>::const_iterator& iterator,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                                      Construct* parent)
{
    Expression* result;
    if (!(result = parseSizeofExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::OPERATOR_MULTIPLY, Token::Type::OPERATOR_DIVIDE}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        Operator op = Operator::NONE;

        if (iterator->type == Token::Type::OPERATOR_MULTIPLY)
            op = Operator::MULTIPLICATION;
        else if (iterator->type == Token::Type::OPERATOR_DIVIDE)
            op = Operator::DIVISION;

        expression->leftExpression = result;

        ++iterator;

        if (!(expression->rightExpression = parseSizeofExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->isLValue = false;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseAdditionExpression(const std::vector<Token>& tokens,
                                                std::vector<Token>::const_iterator& iterator,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                Construct* parent)
{
    Expression* result;
    if (!(result = parseMultiplicationExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::OPERATOR_PLUS, Token::Type::OPERATOR_MINUS}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        Operator op = Operator::NONE;

        if (iterator->type == Token::Type::OPERATOR_PLUS)
            op = Operator::ADDITION;
        else if (iterator->type == Token::Type::OPERATOR_MINUS)
            op = Operator::SUBTRACTION;

        expression->leftExpression = result;

        ++iterator;

        if (!(expression->rightExpression = parseMultiplicationExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->isLValue = false;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseLessThanExpression(const std::vector<Token>& tokens,
                                                std::vector<Token>::const_iterator& iterator,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                Construct* parent)
{
    Expression* result;
    if (!(result = parseAdditionExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::OPERATOR_LESS_THAN, Token::Type::OPERATOR_LESS_THAN_EQUAL}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        Operator op = Operator::NONE;

        if (iterator->type == Token::Type::OPERATOR_LESS_THAN)
            op = Operator::LESS_THAN;
        else if (iterator->type == Token::Type::OPERATOR_LESS_THAN_EQUAL)
            op = Operator::LESS_THAN_EQUAL;

        ++iterator;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseAdditionExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->isLValue = false;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseGreaterThanExpression(const std::vector<Token>& tokens,
                                                   std::vector<Token>::const_iterator& iterator,
                                                   std::vector<std::vector<Declaration*>>& declarationScopes,
                                                   Construct* parent)
{
    Expression* result;
    if (!(result = parseLessThanExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::OPERATOR_GREATER_THAN, Token::Type::OPERATOR_GREATER_THAN_EQUAL}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        Operator op = Operator::NONE;

        if (iterator->type == Token::Type::OPERATOR_GREATER_THAN)
            op = Operator::GREATER_THAN;
        else if (iterator->type == Token::Type::OPERATOR_GREATER_THAN_EQUAL)
            op = Operator::GREATER_THAN_EQUAL;

        ++iterator;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseLessThanExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->isLValue = false;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseEqualityExpression(const std::vector<Token>& tokens,
                                                std::vector<Token>::const_iterator& iterator,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                Construct* parent)
{
    Expression* result;
    if (!(result = parseGreaterThanExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::OPERATOR_EQUAL, Token::Type::OPERATOR_NOT_EQUAL, Token::Type::KEYWORD_NOT_EQ}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        Operator op = Operator::NONE;

        if (iterator->type == Token::Type::OPERATOR_EQUAL)
            op = Operator::EQUALITY;
        else if (iterator->type == Token::Type::OPERATOR_NOT_EQUAL || iterator->type == Token::Type::KEYWORD_NOT_EQ)
            op = Operator::INEQUALITY;

        ++iterator;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseGreaterThanExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->isLValue = false;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseLogicalAndExpression(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  Construct* parent)
{
    Expression* result;
    if (!(result = parseEqualityExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::OPERATOR_AND, Token::Type::KEYWORD_AND}, tokens, iterator))
    {
        ++iterator;

        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        Operator op = Operator::AND;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseEqualityExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        // TODO: check if both sides ar scalar
        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->isLValue = false;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseLogicalOrExpression(const std::vector<Token>& tokens,
                                                 std::vector<Token>::const_iterator& iterator,
                                                 std::vector<std::vector<Declaration*>>& declarationScopes,
                                                 Construct* parent)
{
    Expression* result;
    if (!(result = parseLogicalAndExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::OPERATOR_OR, Token::Type::KEYWORD_OR}, tokens, iterator))
    {
        ++iterator;

        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        Operator op = Operator::OR;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseLogicalAndExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        // TODO: check if both sides ar scalar
        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->isLValue = false;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseTernaryExpression(const std::vector<Token>& tokens,
                                               std::vector<Token>::const_iterator& iterator,
                                               std::vector<std::vector<Declaration*>>& declarationScopes,
                                               Construct* parent)
{
    Expression* result;
    if (!(result = parseLogicalOrExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken(Token::Type::OPERATOR_CONDITIONAL, tokens, iterator))
    {
        ++iterator;

        TernaryOperatorExpression* expression = new TernaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;
        expression->condition = result;

        if (!expression->condition->qualifiedType.typeDeclaration)
            throw std::runtime_error("Ternary expression with a void condition");

        if (!(expression->leftExpression = parseTernaryExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        expectToken(Token::Type::COLON, tokens, iterator);

        ++iterator;

        if (!(expression->rightExpression = parseTernaryExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        // TODO: fix this
        expression->qualifiedType = expression->leftExpression->qualifiedType;
        expression->isLValue = expression->leftExpression->isLValue && expression->rightExpression->isLValue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseAssignmentExpression(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  Construct* parent)
{
    Expression* result;
    if (!(result = parseTernaryExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
    {
        ++iterator;

        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;
        expression->leftExpression = result;

        if (expression->leftExpression->qualifiedType.isConst)
            throw std::runtime_error("Cannot assign to const variable");

        if (!expression->leftExpression->isLValue)
            throw std::runtime_error("Expression is not assignable");

        if (!(expression->rightExpression = parseTernaryExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(Operator::ASSIGNMENT,
                                                                     declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->isLValue = true;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseAdditionAssignmentExpression(const std::vector<Token>& tokens,
                                                          std::vector<Token>::const_iterator& iterator,
                                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                                          Construct* parent)
{
    Expression* result;
    if (!(result = parseAssignmentExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::OPERATOR_PLUS_ASSIGNMENT, Token::Type::OPERATOR_MINUS_ASSIGNMENT}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        Operator op = Operator::NONE;

        if (iterator->type == Token::Type::OPERATOR_PLUS_ASSIGNMENT)
            op = Operator::ADDITION_ASSIGNMENT;
        else if (iterator->type == Token::Type::OPERATOR_MINUS_ASSIGNMENT)
            op = Operator::SUBTRACTION_ASSIGNMENT;

        ++iterator;

        expression->leftExpression = result;

        if (expression->leftExpression->qualifiedType.isConst)
            throw std::runtime_error("Cannot assign to const variable");

        if (!expression->leftExpression->isLValue)
            throw std::runtime_error("Expression is not assignable");

        if (!(expression->rightExpression = parseAssignmentExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->isLValue = true;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseMultiplicationAssignmentExpression(const std::vector<Token>& tokens,
                                                                std::vector<Token>::const_iterator& iterator,
                                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                                Construct* parent)
{
    Expression* result;
    if (!(result = parseAdditionAssignmentExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT, Token::Type::OPERATOR_DIVIDE_ASSIGNMENT}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        Operator op = Operator::NONE;

        if (iterator->type == Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT)
            op = Operator::MULTIPLICATION_ASSIGNMENT;
        else if (iterator->type == Token::Type::OPERATOR_DIVIDE_ASSIGNMENT)
            op = Operator::DIVISION_ASSIGNMENT;

        ++iterator;

        expression->leftExpression = result;

        if (expression->leftExpression->qualifiedType.isConst)
            throw std::runtime_error("Cannot assign to const variable");

        if (!expression->leftExpression->isLValue)
            throw std::runtime_error("Expression is not assignable");

        std::unique_ptr<Construct> right;
        if (!(expression->rightExpression = parseAdditionAssignmentExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->isLValue = true;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseCommaExpression(const std::vector<Token>& tokens,
                                             std::vector<Token>::const_iterator& iterator,
                                             std::vector<std::vector<Declaration*>>& declarationScopes,
                                             Construct* parent)
{
    Expression* result;
    if (!(result = parseMultiplicationAssignmentExpression(tokens, iterator, declarationScopes, parent)))
        return nullptr;

    while (isToken(Token::Type::COMMA, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;
        expression->leftExpression = result;

        ++iterator;

        std::unique_ptr<Construct> right;
        if (!(expression->rightExpression = parseAdditionAssignmentExpression(tokens, iterator, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(Operator::COMMA, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->isLValue = expression->rightExpression->isLValue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

static std::string toString(Construct::Kind kind)
{
    switch (kind)
    {
        case Construct::Kind::NONE: return "NONE";
        case Construct::Kind::DECLARATION: return "DECLARATION";
        case Construct::Kind::STATEMENT: return "STATEMENT";
        case Construct::Kind::EXPRESSION: return "EXPRESSION";
        default: return "unknown";
    }
}

static std::string toString(Statement::Kind kind)
{
    switch (kind)
    {
        case Statement::Kind::NONE: return "NONE";
        case Statement::Kind::EMPTY: return "EMPTY";
        case Statement::Kind::EXPRESSION: return "EXPRESSION";
        case Statement::Kind::DECLARATION: return "DECLARATION";
        case Statement::Kind::COMPOUND: return "COMPOUND";
        case Statement::Kind::IF: return "IF";
        case Statement::Kind::FOR: return "FOR";
        case Statement::Kind::SWITCH: return "SWITCH";
        case Statement::Kind::CASE: return "CASE";
        case Statement::Kind::DEFAULT: return "DEFAULT";
        case Statement::Kind::WHILE: return "WHILE";
        case Statement::Kind::DO: return "DO";
        case Statement::Kind::BREAK: return "BREAK";
        case Statement::Kind::CONTINUE: return "CONTINUE";
        case Statement::Kind::RETURN: return "RETURN";
        default: return "unknown";
    }
}

static std::string toString(Expression::Kind kind)
{
    switch (kind)
    {
        case Expression::Kind::NONE: return "NONE";
        case Expression::Kind::CALL: return "CALL";
        case Expression::Kind::LITERAL: return "LITERAL";
        case Expression::Kind::DECLARATION_REFERENCE: return "DECLARATION_REFERENCE";
        case Expression::Kind::PAREN: return "PAREN";
        case Expression::Kind::MEMBER: return "MEMBER";
        case Expression::Kind::ARRAY_SUBSCRIPT: return "ARRAY_SUBSCRIPT";
        case Expression::Kind::UNARY_OPERATOR: return "UNARY_OPERATOR";
        case Expression::Kind::BINARY_OPERATOR: return "BINARY_OPERATOR";
        case Expression::Kind::TERNARY_OPERATOR: return "TERNARY_OPERATOR";
        case Expression::Kind::TEMPORARY_OBJECT: return "TEMPORARY_OBJECT";
        case Expression::Kind::INITIALIZER_LIST: return "INITIALIZER_LIST";
        case Expression::Kind::CAST: return "CAST";
        case Expression::Kind::SIZEOF: return "SIZEOF";
        default: return "unknown";
    }
}

static std::string toString(Declaration::Kind kind)
{
    switch (kind)
    {
        case Declaration::Kind::NONE: return "NONE";
        case Declaration::Kind::EMPTY: return "EMPTY";
        case Declaration::Kind::TYPE: return "TYPE";
        case Declaration::Kind::FIELD: return "FIELD";
        case Declaration::Kind::CALLABLE: return "CALLABLE";
        case Declaration::Kind::VARIABLE: return "VARIABLE";
        case Declaration::Kind::PARAMETER: return "PARAMETER";
        default: return "unknown";
    }
}

static std::string toString(TypeDeclaration::Kind kind)
{
    switch (kind)
    {
        case TypeDeclaration::Kind::NONE: return "NONE";
        case TypeDeclaration::Kind::ARRAY: return "ARRAY";
        case TypeDeclaration::Kind::SCALAR: return "SCALAR";
        case TypeDeclaration::Kind::STRUCT: return "STRUCT";
        //case TypeDeclaration::Kind::TYPE_DEFINITION: return "TYPE_DEFINITION";
        default: return "unknown";
    }
}

static std::string toString(ScalarTypeDeclaration::Kind kind)
{
    switch (kind)
    {
        case ScalarTypeDeclaration::Kind::NONE: return "NONE";
        case ScalarTypeDeclaration::Kind::BOOLEAN: return "BOOLEAN";
        case ScalarTypeDeclaration::Kind::INTEGER: return "INTEGER";
        case ScalarTypeDeclaration::Kind::FLOATING_POINT: return "FLOATING_POINT";
        default: return "unknown";
    }
}

static std::string toString(CallableDeclaration::Kind kind)
{
    switch (kind)
    {
        case CallableDeclaration::Kind::NONE: return "NONE";
        case CallableDeclaration::Kind::FUNCTION: return "FUNCTION";
        case CallableDeclaration::Kind::CONSTRUCTOR: return "CONSTRUCTOR";
        case CallableDeclaration::Kind::METHOD: return "METHOD";
        case CallableDeclaration::Kind::OPERATOR: return "OPERATOR";
        default: return "unknown";
    }
}

static std::string toString(LiteralExpression::Kind kind)
{
    switch (kind)
    {
        case LiteralExpression::Kind::NONE: return "NONE";
        case LiteralExpression::Kind::BOOLEAN: return "BOOLEAN";
        case LiteralExpression::Kind::INTEGER: return "INTEGER";
        case LiteralExpression::Kind::FLOATING_POINT: return "FLOATING_POINT";
        case LiteralExpression::Kind::STRING: return "STRING";
        default: return "unknown";
    }
}

static std::string toString(CastExpression::Kind kind)
{
    switch (kind)
    {
        case CastExpression::Kind::NONE: return "NONE";
        case CastExpression::Kind::IMPLICIT: return "IMPLICIT";
        case CastExpression::Kind::C_STYLE: return "C_STYLE";
        case CastExpression::Kind::FUNCTIONAL: return "FUNCTIONAL";
        case CastExpression::Kind::STATIC: return "STATIC";
        case CastExpression::Kind::DYNAMIC: return "DYNAMIC";
        case CastExpression::Kind::REINTERPRET: return "REINTERPRET";
        default: return "unknown";
    }
}

void ASTContext::dump() const
{
    for (Declaration* declaration : declarations)
        dumpConstruct(declaration);
}

static std::string getPrintableTypeName(const QualifiedType& qualifiedType)
{
    std::string result;

    if (qualifiedType.isVolatile) result += "volatile ";
    if (qualifiedType.isConst) result += "const ";

    if (!qualifiedType.typeDeclaration)
    {
        result += "void";
    }
    else
    {
        TypeDeclaration* typeDeclaration = qualifiedType.typeDeclaration;
        while (typeDeclaration->getTypeKind() == TypeDeclaration::Kind::ARRAY)
        {
            ArrayTypeDeclaration* arrayTypeDeclaration = static_cast<ArrayTypeDeclaration*>(typeDeclaration);

            result = "[" + std::to_string(arrayTypeDeclaration->size) + "]" + result;

            typeDeclaration = arrayTypeDeclaration->elementType.typeDeclaration;
        }

        result = typeDeclaration->name + result;
    }

    return result;
}

void ASTContext::dumpDeclaration(const Declaration* declaration, std::string indent) const
{
    std::cout << " " << toString(declaration->getDeclarationKind());

    switch (declaration->getDeclarationKind())
    {
        case Declaration::Kind::NONE:
        {
            break;
        }

        case Declaration::Kind::EMPTY:
        {
            std::cout << std::endl;
            break;
        }

        case Declaration::Kind::TYPE:
        {
            const TypeDeclaration* typeDeclaration = static_cast<const TypeDeclaration*>(declaration);

            std::cout << " " << toString(typeDeclaration->getTypeKind());

            switch (typeDeclaration->getTypeKind())
            {
                case TypeDeclaration::Kind::NONE:
                {
                    break;
                }

                case TypeDeclaration::Kind::ARRAY: // array types can not be declared in code
                {
                    break;
                }

                case TypeDeclaration::Kind::STRUCT:
                {
                    const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(typeDeclaration);
                    std::cout << ", name: " << structDeclaration->name;

                    if (structDeclaration->previousDeclaration)
                        std::cout << ", previous declaration: " << structDeclaration->previousDeclaration;

                    if (structDeclaration->definition)
                        std::cout << ", definition: " << structDeclaration->definition;

                    std::cout << std::endl;

                    for (const Declaration* memberDeclaration : structDeclaration->memberDeclarations)
                        dumpConstruct(memberDeclaration, indent + "  ");

                    break;
                }

                case TypeDeclaration::Kind::SCALAR:
                {
                    const ScalarTypeDeclaration* scalarTypeDeclaration = static_cast<const ScalarTypeDeclaration*>(typeDeclaration);
                    std::cout << ", name: " << scalarTypeDeclaration->name << ", scalar type kind: " << toString(scalarTypeDeclaration->getScalarTypeKind());
                    break;
                }
            }
            break;
        }

        case Declaration::Kind::FIELD:
        {
            const FieldDeclaration* fieldDeclaration = static_cast<const FieldDeclaration*>(declaration);

            std::cout << ", name: " << fieldDeclaration->name << ", type: " << getPrintableTypeName(fieldDeclaration->qualifiedType);

            if (fieldDeclaration->semantic != Semantic::NONE)
                std::cout << ", semantic: " << toString(fieldDeclaration->semantic);

            std::cout << std::endl;
            break;
        }

        case Declaration::Kind::CALLABLE:
        {
            const CallableDeclaration* callableDeclaration = static_cast<const CallableDeclaration*>(declaration);

            std::cout << ", callable kind: " << toString(callableDeclaration->getCallableDeclarationKind()) << ", name: " << callableDeclaration->name << ", result type: " << getPrintableTypeName(callableDeclaration->qualifiedType);

            if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::FUNCTION)
            {
                const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(callableDeclaration);

                if (functionDeclaration->isStatic) std::cout << " static";
                if (functionDeclaration->isInline) std::cout << " inline";

                if (functionDeclaration->program != Program::NONE)
                    std::cout << ", program: " << toString(functionDeclaration->program);
            }

            if (callableDeclaration->previousDeclaration)
                std::cout << ", previous declaration: " << callableDeclaration->previousDeclaration;

            if (callableDeclaration->definition)
                std::cout << ", definition: " << callableDeclaration->definition;

            std::cout << std::endl;

            for (ParameterDeclaration* parameter : callableDeclaration->parameterDeclarations)
                dumpConstruct(parameter, indent + "  ");

            if (callableDeclaration->body)
                dumpConstruct(callableDeclaration->body, indent + "  ");

            break;
        }

        case Declaration::Kind::VARIABLE:
        {
            const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(declaration);
            std::cout << ", name: " << variableDeclaration->name << ", type: " << getPrintableTypeName(variableDeclaration->qualifiedType) << std::endl;

            if (variableDeclaration->initialization)
            {
                dumpConstruct(variableDeclaration->initialization, indent + "  ");
            }

            break;
        }

        case Declaration::Kind::PARAMETER:
        {
            const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(declaration);
            std::cout << ", name: " << parameterDeclaration->name << ", type: " << getPrintableTypeName(parameterDeclaration->qualifiedType) << std::endl;
            break;
        }

        default:
            break;
    }
}

void ASTContext::dumpStatement(const Statement* statement, std::string indent) const
{
    std::cout << " " << toString(statement->getStatementKind());

    switch (statement->getStatementKind())
    {
        case Statement::Kind::NONE:
        {
            break;
        }

        case Statement::Kind::EMPTY:
        {
            std::cout << std::endl;
            break;
        }

        case Statement::Kind::EXPRESSION:
        {
            const ExpressionStatement* expressionStatement = static_cast<const ExpressionStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(expressionStatement->expression, indent + "  ");
            break;
        }

        case Statement::Kind::DECLARATION:
        {
            const DeclarationStatement* declarationStatement = static_cast<const DeclarationStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(declarationStatement->declaration, indent + "  ");
            break;
        }

        case Statement::Kind::COMPOUND:
        {
            const CompoundStatement* compoundStatement = static_cast<const CompoundStatement*>(statement);

            std::cout << std::endl;

            for (Statement* subSstatement : compoundStatement->statements)
                dumpConstruct(subSstatement, indent + "  ");

            break;
        }

        case Statement::Kind::IF:
        {
            const IfStatement* ifStatement = static_cast<const IfStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(ifStatement->condition, indent + "  ");
            dumpConstruct(ifStatement->body, indent + "  ");
            if (ifStatement->elseBody) dumpConstruct(ifStatement->elseBody, indent + "  ");
            break;
        }

        case Statement::Kind::FOR:
        {
            const ForStatement* forStatement = static_cast<const ForStatement*>(statement);

            std::cout << std::endl;

            if (forStatement->initialization) dumpConstruct(forStatement->initialization, indent + "  ");
            if (forStatement->condition) dumpConstruct(forStatement->condition, indent + "  ");
            if (forStatement->increment) dumpConstruct(forStatement->increment, indent + "  ");
            dumpConstruct(forStatement->body, indent + "  ");
            break;
        }

        case Statement::Kind::SWITCH:
        {
            const SwitchStatement* switchStatement = static_cast<const SwitchStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(switchStatement->condition, indent + "  ");
            dumpConstruct(switchStatement->body, indent + "  ");
            break;
        }

        case Statement::Kind::CASE:
        {
            const CaseStatement* caseStatement = static_cast<const CaseStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(caseStatement->condition, indent + "  ");
            dumpConstruct(caseStatement->body, indent + "  ");
            break;
        }

        case Statement::Kind::DEFAULT:
        {
            const DefaultStatement* defaultStatement = static_cast<const DefaultStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(defaultStatement->body, indent + "  ");
            break;
        }

        case Statement::Kind::WHILE:
        {
            const WhileStatement* whileStatement = static_cast<const WhileStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(whileStatement->condition, indent + "  ");
            dumpConstruct(whileStatement->body, indent + "  ");
            break;
        }

        case Statement::Kind::DO:
        {
            const DoStatement* doStatement = static_cast<const DoStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(doStatement->body, indent + "  ");
            dumpConstruct(doStatement->condition, indent + "  ");
            break;
        }

        case Statement::Kind::BREAK:
        {
            std::cout << std::endl;
            break;
        }

        case Statement::Kind::CONTINUE:
        {
            std::cout << std::endl;
            break;
        }

        case Statement::Kind::RETURN:
        {
            const ReturnStatement* returnStatement = static_cast<const ReturnStatement*>(statement);

            std::cout << std::endl;

            if (returnStatement->result)
            {
                dumpConstruct(returnStatement->result, indent + "  ");
            }
            break;
        }

        default:
            break;
    }
}

void ASTContext::dumpExpression(const Expression* expression, std::string indent) const
{
    std::cout << " " << toString(expression->getExpressionKind()) << ", lvalue: " << (expression->isLValue ? "true" : "false");

    switch (expression->getExpressionKind())
    {
        case Expression::Kind::NONE:
        {
            break;
        }

        case Expression::Kind::CALL:
        {
            const CallExpression* callExpression = static_cast<const CallExpression*>(expression);

            std::cout << std::endl;

            dumpConstruct(callExpression->declarationReference, indent + "  ");

            for (Expression* parameter : callExpression->parameters)
                dumpConstruct(parameter, indent + "  ");

            break;
        }

        case Expression::Kind::LITERAL:
        {
            const LiteralExpression* literalExpression = static_cast<const LiteralExpression*>(expression);

            std::cout << ", literal kind: " << toString(literalExpression->getLiteralKind()) << ", value: ";

            switch (literalExpression->getLiteralKind())
            {
                case LiteralExpression::Kind::NONE: break;
                case LiteralExpression::Kind::BOOLEAN:
                {
                    const BooleanLiteralExpression* booleanLiteralExpression = static_cast<const BooleanLiteralExpression*>(literalExpression);
                    std::cout << (booleanLiteralExpression->value ? "true" : "false");
                    break;
                }
                case LiteralExpression::Kind::INTEGER:
                {
                    const IntegerLiteralExpression* integerLiteralExpression = static_cast<const IntegerLiteralExpression*>(literalExpression);
                    std::cout << integerLiteralExpression->value;
                    break;
                }
                case LiteralExpression::Kind::FLOATING_POINT:
                {
                    const FloatingPointLiteralExpression* floatingPointLiteralExpression = static_cast<const FloatingPointLiteralExpression*>(literalExpression);
                    std::cout << floatingPointLiteralExpression->value;
                    break;
                }
                case LiteralExpression::Kind::STRING:
                {
                    const StringLiteralExpression* stringLiteralExpression = static_cast<const StringLiteralExpression*>(literalExpression);
                    std::cout << stringLiteralExpression->value;
                    break;
                }
            }

            std::cout << std::endl;
            break;
        }

        case Expression::Kind::DECLARATION_REFERENCE:
        {
            const DeclarationReferenceExpression* declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(expression);

            std::cout << " " << declarationReferenceExpression->declaration->name << " " << declarationReferenceExpression->declaration;

            std::cout << std::endl;
            break;
        }

        case Expression::Kind::PAREN:
        {
            const ParenExpression* parenExpression = static_cast<const ParenExpression*>(expression);

            std::cout << std::endl;

            dumpConstruct(parenExpression->expression, indent + "  ");
            break;
        }

        case Expression::Kind::MEMBER:
        {
            const MemberExpression* memberExpression = static_cast<const MemberExpression*>(expression);

            std::cout << ", field: " << memberExpression->fieldDeclaration->name << std::endl;

            dumpConstruct(memberExpression->expression, indent + "  ");
            break;
        }

        case Expression::Kind::ARRAY_SUBSCRIPT:
        {
            const ArraySubscriptExpression* arraySubscriptExpression = static_cast<const ArraySubscriptExpression*>(expression);

            std::cout << std::endl;

            dumpConstruct(arraySubscriptExpression->expression, indent + "  ");
            dumpConstruct(arraySubscriptExpression->subscript, indent + "  ");
            break;
        }

        case Expression::Kind::UNARY_OPERATOR:
        {
            const UnaryOperatorExpression* unaryOperatorExpression = static_cast<const UnaryOperatorExpression*>(expression);

            std::cout <<", operator: " << toString(unaryOperatorExpression->operatorDeclaration->op) << std::endl;

            dumpConstruct(unaryOperatorExpression->expression, indent + "  ");
            break;
        }

        case Expression::Kind::BINARY_OPERATOR:
        {
            const BinaryOperatorExpression* binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(expression);

            std::cout << ", operator: " << toString(binaryOperatorExpression->operatorDeclaration->op) << std::endl;

            dumpConstruct(binaryOperatorExpression->leftExpression, indent + "  ");
            dumpConstruct(binaryOperatorExpression->rightExpression, indent + "  ");
            break;
        }

        case Expression::Kind::TERNARY_OPERATOR:
        {
            const TernaryOperatorExpression* ternaryOperatorExpression = static_cast<const TernaryOperatorExpression*>(expression);

            std::cout << std::endl;

            dumpConstruct(ternaryOperatorExpression->condition, indent + "  ");
            dumpConstruct(ternaryOperatorExpression->leftExpression, indent + "  ");
            dumpConstruct(ternaryOperatorExpression->rightExpression, indent + "  ");
            break;
        }

        case Expression::Kind::TEMPORARY_OBJECT:
        {
            const TemporaryObjectExpression* temporaryObjectExpression = static_cast<const TemporaryObjectExpression*>(expression);

            const TypeDeclaration* typeDeclaration = static_cast<const TypeDeclaration*>(temporaryObjectExpression->constructorDeclaration->parent);

            std::cout << " " << typeDeclaration->name << std::endl;

            for (Expression* parameter : temporaryObjectExpression->parameters)
                dumpConstruct(parameter, indent + "  ");

            break;
        }

        case Expression::Kind::INITIALIZER_LIST:
        {
            const InitializerListExpression* initializerListExpression = static_cast<const InitializerListExpression*>(expression);

            std::cout << std::endl;

            for (Expression* subExpression : initializerListExpression->expressions)
                dumpConstruct(subExpression, indent + "  ");

            break;
        }

        case Expression::Kind::CAST:
        {
            const CastExpression* castExpression = static_cast<const CastExpression*>(expression);

            std::cout << " " << toString(castExpression->getCastKind()) << " " <<
                castExpression->expression->qualifiedType.typeDeclaration->name << " to " <<
                castExpression->qualifiedType.typeDeclaration->name << std::endl;

            dumpConstruct(castExpression->expression, indent + "  ");

            break;
        }
        case Expression::Kind::SIZEOF:
        {
            const SizeofExpression* sizeofExpression = static_cast<const SizeofExpression*>(expression);

            std::cout << std::endl;

            if (sizeofExpression->expression)
                dumpConstruct(sizeofExpression->expression, indent + "  ");
            else if (sizeofExpression->type)
                dumpConstruct(sizeofExpression->type, indent + "  ");
            break;
        }
    }
}

void ASTContext::dumpConstruct(const Construct* construct, std::string indent) const
{
    std::cout << indent << construct;
    if (construct->parent) std::cout << ", parent: " << construct->parent;
    std::cout << " " << toString(construct->getKind());

    switch (construct->getKind())
    {
        case Construct::Kind::NONE:
        {
            std::cout << std::endl;
            break;
        }

        case Construct::Kind::DECLARATION:
        {
            const Declaration* declaration = static_cast<const Declaration*>(construct);
            dumpDeclaration(declaration, indent);
            break;
        }

        case Construct::Kind::STATEMENT:
        {
            const Statement* statement = static_cast<const Statement*>(construct);
            dumpStatement(statement, indent);
            break;
        }

        case Construct::Kind::EXPRESSION:
        {
            const Expression* expression = static_cast<const Expression*>(construct);
            dumpExpression(expression, indent);
            break;
        }
    }
}
