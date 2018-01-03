//
//  OSL
//

#include <iostream>
#include "Parser.hpp"

static const std::vector<std::string> builtinTypes = {
    "void", "bool", "int", "uint", "float", "double",
    "vec2", "vec3", "vec4", "mat3", "mat4"
};

ASTContext::ASTContext():
    boolTypeDeclaration(ScalarTypeDeclaration::Kind::BOOLEAN),
    intTypeDeclaration(ScalarTypeDeclaration::Kind::INTEGER),
    floatTypeDeclaration(ScalarTypeDeclaration::Kind::FLOATING_POINT)
{
    boolTypeDeclaration.name = "bool";
    boolTypeDeclaration.isBuiltin = true;

    intTypeDeclaration.name = "int";
    intTypeDeclaration.isBuiltin = true;

    floatTypeDeclaration.name = "float";
    floatTypeDeclaration.isBuiltin = true;

    float2TypeDeclaration.name = "float2";
    float2TypeDeclaration.isBuiltin = true;
    float2TypeDeclaration.definition = &float2TypeDeclaration;

    float3TypeDeclaration.name = "float3";
    float3TypeDeclaration.isBuiltin = true;
    float3TypeDeclaration.definition = &float3TypeDeclaration;

    float4TypeDeclaration.name = "float4";
    float4TypeDeclaration.isBuiltin = true;
    float4TypeDeclaration.definition = &float4TypeDeclaration;

    std::vector<std::pair<StructDeclaration*, std::vector<TypeDeclaration*>>> constructors = {
        {&float2TypeDeclaration, {&floatTypeDeclaration}},
        {&float2TypeDeclaration, {&floatTypeDeclaration, &floatTypeDeclaration}},
        {&float3TypeDeclaration, {&floatTypeDeclaration}},
        {&float3TypeDeclaration, {&floatTypeDeclaration, &floatTypeDeclaration, &floatTypeDeclaration}},
        {&float4TypeDeclaration, {&floatTypeDeclaration}},
        {&float4TypeDeclaration, {&floatTypeDeclaration, &floatTypeDeclaration, &floatTypeDeclaration, &floatTypeDeclaration}}
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
        {&float2TypeDeclaration, {'x', 'y'}},
        {&float2TypeDeclaration, {'r', 'g'}},
        {&float3TypeDeclaration, {'x', 'y', 'z'}},
        {&float3TypeDeclaration, {'r', 'g', 'b'}},
        {&float4TypeDeclaration, {'x', 'y', 'z', 'w'}},
        {&float4TypeDeclaration, {'r', 'g', 'b', 'a'}}
    };

    FieldDeclaration* fieldDeclaration = fieldDeclarations;

    for (auto& type : types)
    {
        for (char first : type.second)
        {
            fieldDeclaration->parent = type.first;
            fieldDeclaration->qualifiedType.typeDeclaration = &floatTypeDeclaration;
            fieldDeclaration->qualifiedType.isConst = false;
            fieldDeclaration->name.assign({first});

            type.first->memberDeclarations.push_back(fieldDeclaration);

            ++fieldDeclaration;

            for (char second : type.second)
            {
                bool secondConst = (second == first);

                fieldDeclaration->parent = type.first;
                fieldDeclaration->qualifiedType.typeDeclaration = &float2TypeDeclaration;
                fieldDeclaration->qualifiedType.isConst = secondConst;
                fieldDeclaration->name.assign({first, second});

                type.first->memberDeclarations.push_back(fieldDeclaration);

                ++fieldDeclaration;

                for (char third : type.second)
                {
                    bool thirdConst = (secondConst || third == first || third == second);

                    fieldDeclaration->parent = type.first;
                    fieldDeclaration->qualifiedType.typeDeclaration = &float3TypeDeclaration;
                    fieldDeclaration->qualifiedType.isConst = thirdConst;
                    fieldDeclaration->name.assign({first, second, third});

                    type.first->memberDeclarations.push_back(fieldDeclaration);

                    ++fieldDeclaration;

                    for (char fourth : type.second)
                    {
                        bool fourthConst = (thirdConst || fourth == first || fourth == second || fourth == third);

                        fieldDeclaration->parent = type.first;
                        fieldDeclaration->qualifiedType.typeDeclaration = &float4TypeDeclaration;
                        fieldDeclaration->qualifiedType.isConst = fourthConst;
                        fieldDeclaration->name.assign({first, second, third, fourth});

                        type.first->memberDeclarations.push_back(fieldDeclaration);

                        ++fieldDeclaration;
                    }
                }
            }
        }
    }

    float2x2TypeDeclaration.name = "float2x2";
    float2x2TypeDeclaration.isBuiltin = true;
    float2x2TypeDeclaration.definition = &float2x2TypeDeclaration;

    float3x3TypeDeclaration.name = "float3x3";
    float3x3TypeDeclaration.isBuiltin = true;
    float3x3TypeDeclaration.definition = &float3x3TypeDeclaration;

    float4x4TypeDeclaration.name = "float4x4";
    float4x4TypeDeclaration.isBuiltin = true;
    float4x4TypeDeclaration.definition = &float4x4TypeDeclaration;

    stringTypeDeclaration.name = "string";
    stringTypeDeclaration.isBuiltin = true;
    stringTypeDeclaration.definition = &stringTypeDeclaration;

    samplerStateTypeDeclaration.name = "SamplerState";
    samplerStateTypeDeclaration.isBuiltin = true;
    samplerStateTypeDeclaration.definition = &samplerStateTypeDeclaration;

    texture2DTypeDeclaration.name = "Texture2D";
    texture2DTypeDeclaration.isBuiltin = true;
    texture2DTypeDeclaration.definition = &texture2DTypeDeclaration;

    samplerParameterDeclaration.name = "sampler";
    samplerParameterDeclaration.qualifiedType.typeDeclaration = &samplerStateTypeDeclaration;

    coordParameterDeclaration.name = "coord";
    coordParameterDeclaration.qualifiedType.typeDeclaration = &float2TypeDeclaration;

    mulFunctionDeclaration.name = "texture2D";
    mulFunctionDeclaration.qualifiedType.typeDeclaration = &float4TypeDeclaration;
    mulFunctionDeclaration.parameterDeclarations.push_back(&samplerParameterDeclaration);
    mulFunctionDeclaration.parameterDeclarations.push_back(&coordParameterDeclaration);
    mulFunctionDeclaration.isBuiltin = true;
}

bool ASTContext::parse(const std::vector<Token>& tokens)
{
    constructs.clear();
    declarations.clear();

    auto iterator = tokens.cbegin();

    std::vector<std::vector<Declaration*>> declarationScopes;
    declarationScopes.push_back(std::vector<Declaration*>());

    declarationScopes.back().push_back(&boolTypeDeclaration);
    declarationScopes.back().push_back(&intTypeDeclaration);
    declarationScopes.back().push_back(&floatTypeDeclaration);
    declarationScopes.back().push_back(&float2TypeDeclaration);
    declarationScopes.back().push_back(&float3TypeDeclaration);
    declarationScopes.back().push_back(&float4TypeDeclaration);
    declarationScopes.back().push_back(&float2x2TypeDeclaration);
    declarationScopes.back().push_back(&float3x3TypeDeclaration);
    declarationScopes.back().push_back(&float4x4TypeDeclaration);
    declarationScopes.back().push_back(&stringTypeDeclaration);
    declarationScopes.back().push_back(&samplerStateTypeDeclaration);
    declarationScopes.back().push_back(&texture2DTypeDeclaration);
    declarationScopes.back().push_back(&mulFunctionDeclaration);

    while (iterator != tokens.end())
    {
        Declaration* declaration;
        if (!(declaration = parseTopLevelDeclaration(tokens, iterator, declarationScopes)))
        {
            return false;
        }

        declarations.push_back(declaration);
    }

    return true;
}

bool ASTContext::isType(const std::vector<Token>& tokens,
                        std::vector<Token>::const_iterator iterator,
                        std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (iterator == tokens.end()) return false;

    return iterator->type == Token::Type::KEYWORD_BOOL ||
        iterator->type == Token::Type::KEYWORD_INT ||
        iterator->type == Token::Type::KEYWORD_FLOAT ||
        (iterator->type == Token::Type::IDENTIFIER &&
         findTypeDeclaration(iterator->value, declarationScopes));
}

TypeDeclaration* ASTContext::parseType(const std::vector<Token>& tokens,
                                       std::vector<Token>::const_iterator& iterator,
                                       std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (iterator == tokens.end()) return nullptr;

    TypeDeclaration* result;

    if (iterator->type == Token::Type::KEYWORD_BOOL)
    {
        result = &boolTypeDeclaration;
    }
    else if (iterator->type == Token::Type::KEYWORD_INT)
    {
        result = &intTypeDeclaration;
    }
    else if (iterator->type == Token::Type::KEYWORD_FLOAT)
    {
        result = &floatTypeDeclaration;
    }
    else if (iterator->type == Token::Type::IDENTIFIER)
    {
        if (!(result = findTypeDeclaration(iterator->value, declarationScopes)))
        {
            std::cerr << "Invalid type: " << iterator->value << std::endl;
            return nullptr;
        }
    }
    else
    {
        std::cerr << "Expected a type name" << std::endl;
        return nullptr;
    }

    ++iterator;

    return result;
}

bool ASTContext::isDeclaration(const std::vector<Token>& tokens,
                               std::vector<Token>::const_iterator iterator,
                               std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (iterator == tokens.end()) return false;

    return iterator->type == Token::Type::KEYWORD_CONST ||
        iterator->type == Token::Type::KEYWORD_STATIC ||
        iterator->type == Token::Type::KEYWORD_INLINE ||
        iterator->type == Token::Type::KEYWORD_STRUCT ||
        iterator->type == Token::Type::KEYWORD_BOOL ||
        iterator->type == Token::Type::KEYWORD_INT ||
        iterator->type == Token::Type::KEYWORD_FLOAT ||
        isType(tokens, iterator, declarationScopes);
}

Declaration* ASTContext::parseTopLevelDeclaration(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Declaration* declaration;
    if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes, nullptr)))
    {
        std::cerr << "Failed to parse a declaration" << std::endl;
        return nullptr;
    }

    if (declaration->getDeclarationKind() == Declaration::Kind::FUNCTION)
    {
        FunctionDeclaration* functionDeclaration = static_cast<FunctionDeclaration*>(declaration);

        // semicolon is not needed after a function definition
        if (!functionDeclaration->body)
        {
            if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
            {
                std::cerr << "Expected a semicolon" << std::endl;
                return nullptr;
            }

            ++iterator;
        }
    }
    else
    {
        if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;
    }

    return declaration;
}

bool ASTContext::parseSpecifiers(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 Specifiers& specifiers)
{
    specifiers.isConst = false;
    specifiers.isInline = false;
    specifiers.isStatic = false;

    for (;;)
    {
        if (isToken(Token::Type::KEYWORD_CONST, tokens, iterator))
        {
            ++iterator;
            specifiers.isConst = true;
        }
        else if (isToken(Token::Type::KEYWORD_STATIC, tokens, iterator))
        {
            ++iterator;
            specifiers.isStatic = true;
        }
        else if (isToken(Token::Type::KEYWORD_INLINE, tokens, iterator))
        {
            ++iterator;
            specifiers.isInline = true;
        }
        else break;
    }

    return true;
}

bool ASTContext::parseAttributes(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::pair<std::string, std::vector<std::string>>>& attributes)
{
    while (isToken(Token::Type::LEFT_BRACKET, tokens, iterator) &&
           isToken(Token::Type::LEFT_BRACKET, tokens, iterator + 1))
    {
        ++iterator;
        ++iterator;

        if (!isToken(Token::Type::IDENTIFIER, tokens, iterator))
        {
            std::cerr << "Expected an identifier" << std::endl;
            return nullptr;
        }

        std::pair<std::string, std::vector<std::string>> attribute;

        attribute.first = iterator->value;

        ++iterator;

        if (isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            ++iterator;

            for (;;)
            {
                if (isToken(Token::Type::LITERAL_INT, tokens, iterator) ||
                    isToken(Token::Type::LITERAL_FLOAT, tokens, iterator) ||
                    isToken(Token::Type::LITERAL_CHAR, tokens, iterator) ||
                    isToken(Token::Type::LITERAL_STRING, tokens, iterator))
                {
                    attribute.second.push_back(iterator->value);
                    ++iterator;
                }
                else
                {
                    std::cerr << "Unexpected token" << std::endl;
                    return false;
                }

                if (!isToken(Token::Type::COMMA, tokens, iterator))
                {
                    break;
                }
            }

            if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
            {
                std::cerr << "Expected a right parenthesis" << std::endl;
                return false;
            }

            ++iterator;
        }

        if (!isToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
        {
            std::cerr << "Expected a right bracket" << std::endl;
            return false;
        }

        ++iterator;

        if (!isToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
        {
            std::cerr << "Expected a right bracket" << std::endl;
            return false;
        }

        ++iterator;

        attributes.push_back(attribute);
    }

    return true;
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
        {
            std::cerr << "Failed to parse a structure declaration" << std::endl;
            return nullptr;
        }

        return declaration;
    }
    /*else if (isToken(Token::Type::KEYWORD_TYPEDEF, tokens, iterator))
    {
        ++iterator;

        TypeDefinitionDeclaration* declaration;
        if (!(declaration = parseTypeDefinitionDeclaration(tokens, iterator, declarationScopes, parent)))
        {
            std::cerr << "Failed to parse a type definition declaration" << std::endl;
            return nullptr;
        }

        return declaration;
    }*/
    else
    {
        std::vector<std::pair<std::string, std::vector<std::string>>> attributes;
        if (!parseAttributes(tokens, iterator, attributes)) return nullptr;

        ASTContext::Specifiers specifiers;
        if (!parseSpecifiers(tokens, iterator, specifiers)) return nullptr;

        bool isStatic = specifiers.isStatic;
        bool isInline = specifiers.isInline;

        QualifiedType qualifiedType;
        qualifiedType.isConst = specifiers.isConst;

        if (isToken(Token::Type::KEYWORD_VOID, tokens, iterator))
        {
            ++iterator;
        }
        else
        {
            if (!(qualifiedType.typeDeclaration = parseType(tokens, iterator, declarationScopes)))
            {
                return nullptr;
            }

            if (qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::STRUCT)
            {
                StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(qualifiedType.typeDeclaration);

                if (!structDeclaration->definition)
                {
                    std::cerr << "Incomplete type " << qualifiedType.typeDeclaration->name << std::endl;
                    return nullptr;
                }
            }
        }

        if (!parseSpecifiers(tokens, iterator, specifiers)) return nullptr;

        if (specifiers.isStatic) isStatic = true;
        if (specifiers.isInline) isInline = true;

        if (!isToken(Token::Type::IDENTIFIER, tokens, iterator))
        {
            std::cerr << "Expected an identifier" << std::endl;
            return nullptr;
        }

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
                    {
                        return nullptr;
                    }

                    result->parameterDeclarations.push_back(parameterDeclaration);
                    parameters.push_back(parameterDeclaration->qualifiedType);

                    if (!isToken(Token::Type::COMMA, tokens, iterator))
                    {
                        break;
                    }

                    ++iterator;
                }
            }

            if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
            {
                std::cerr << "Expected a right parenthesis" << std::endl;
                return nullptr;
            }

            ++iterator;

            result->previousDeclaration = findFunctionDeclaration(name, declarationScopes, parameters);

            if (!parseAttributes(tokens, iterator, attributes)) return nullptr;

            for (std::pair<std::string, std::vector<std::string>>& attribute : attributes)
            {
                if (attribute.first == "program")
                {
                    if (attribute.second.size() == 1)
                    {
                        FunctionDeclaration::Program program = FunctionDeclaration::Program::NONE;

                        if (attribute.second.front() == "fragment") program = FunctionDeclaration::Program::FRAGMENT;
                        else if (attribute.second.front() == "vertex") program = FunctionDeclaration::Program::VERTEX;
                        else
                        {
                            std::cerr << "Invalid program" << attribute.second.front() << std::endl;
                        }

                        result->program = program;
                    }
                    else
                    {
                        std::cerr << "Invalid parameters for attribute " << attribute.first << std::endl;
                    }
                }
                else
                {
                    std::cerr << "Invalid attribute " << attribute.first << std::endl;
                }
            }

            declarationScopes.back().push_back(result);

            // set the definition of the previous declaration
            if (result->previousDeclaration) result->definition = result->previousDeclaration->definition;

            if (isToken(Token::Type::LEFT_BRACE, tokens, iterator))
            {
                // check if only one definition exists
                if (result->definition)
                {
                    std::cerr << "Redefinition of " << result->name << std::endl;
                    return nullptr;
                }

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
                {
                    std::cerr << "Failed to parse a compound statement" << std::endl;
                    return nullptr;
                }

                declarationScopes.pop_back();
            }

            return result;
        }
        else // variable declaration
        {
            if (isInline)
            {
                std::cerr << "Variables can not be inline" << std::endl;
                return nullptr;
            }

            if (findDeclaration(name, declarationScopes.back()))
            {
                std::cerr << "Redefinition of " << name << std::endl;
                return nullptr;
            }

            VariableDeclaration* result = new VariableDeclaration();
            constructs.push_back(std::unique_ptr<VariableDeclaration>(result));
            result->parent = parent;
            result->qualifiedType = qualifiedType;
            result->isStatic = isStatic;
            result->name = name;

            while (isToken(Token::Type::LEFT_BRACKET, tokens, iterator))
            {
                ++iterator;

                if (isToken(Token::Type::LITERAL_INT, tokens, iterator))
                {
                    int size = std::stoi(iterator->value);

                    ++iterator;

                    if (size <= 0)
                    {
                        std::cerr << "Array size must be greater than zero" << std::endl;
                        return nullptr;
                    }

                    result->qualifiedType.dimensions.push_back(static_cast<uint32_t>(size));
                }
                else
                {
                    std::cerr << "Expected an integer literal" << std::endl;
                    return nullptr;
                }

                if (!isToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
                {
                    std::cerr << "Expected a right bracket" << std::endl;
                    return nullptr;
                }

                ++iterator;
            }

            if (isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
            {
                ++iterator;

                if (!(result->initialization = parseMultiplicationAssignmentExpression(tokens, iterator, declarationScopes, result)))
                {
                    return nullptr;
                }

                if (!result->initialization->qualifiedType.typeDeclaration)
                {
                    std::cerr << "Initialization with a void type" << std::endl;
                    return nullptr;
                }

                // TODO: check for comma and parse multiple expressions

                if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
                {
                    std::cerr << "Expected a right parenthesis" << std::endl;
                    return nullptr;
                }

                ++iterator;
            }
            else if (isToken(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
            {
                ++iterator;

                if (!(result->initialization = parseMultiplicationAssignmentExpression(tokens, iterator, declarationScopes, result)))
                {
                    return nullptr;
                }

                if (!result->initialization->qualifiedType.typeDeclaration)
                {
                    std::cerr << "Initialization with a void type" << std::endl;
                    return nullptr;
                }
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
    if (!isToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected an identifier" << std::endl;
        return nullptr;
    }

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
        {
            std::cerr << "Redefinition of " << result->name << std::endl;
            return nullptr;
        }

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
                {
                    return nullptr;
                }

                if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
                {
                    std::cerr << "Expected a semicolon" << std::endl;
                    return nullptr;
                }

                if (result->findMemberDeclaration(memberDeclaration->name))
                {
                    std::cerr << "Redefinition of member " << memberDeclaration->name << std::endl;
                    return nullptr;
                }

                ++iterator;

                memberDeclaration->parent = result;

                result->memberDeclarations.push_back(memberDeclaration);
            }
        }
    }

    declarationScopes.back().push_back(result);

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

        std::vector<std::pair<std::string, std::vector<std::string>>> attributes;
        if (!parseAttributes(tokens, iterator, attributes)) return nullptr;

        ASTContext::Specifiers specifiers;
        if (!parseSpecifiers(tokens, iterator, specifiers)) return nullptr;

        bool isStatic = specifiers.isStatic;
        bool isInline = specifiers.isInline;

        if (!(result->qualifiedType.typeDeclaration = parseType(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (result->qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::STRUCT)
        {
            StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->qualifiedType.typeDeclaration);

            if (!structDeclaration->definition)
            {
                std::cerr << "Incomplete type " << result->qualifiedType.typeDeclaration->name << std::endl;
                return nullptr;
            }
        }

        if (!parseSpecifiers(tokens, iterator, specifiers)) return nullptr;

        if (specifiers.isStatic) isStatic = true;
        if (specifiers.isInline) isInline = true;

        if (isStatic)
        {
            std::cerr << "Members can not be static" << std::endl;
            return nullptr;
        }

        if (isInline)
        {
            std::cerr << "Members can not be inline" << std::endl;
            return nullptr;
        }

        if (!isToken(Token::Type::IDENTIFIER, tokens, iterator))
        {
            std::cerr << "Expected an identifier" << std::endl;
            return nullptr;
        }

        result->name = iterator->value;

        ++iterator;

        if (!parseAttributes(tokens, iterator, attributes)) return nullptr;

        while (isToken(Token::Type::LEFT_BRACKET, tokens, iterator))
        {
            ++iterator;

            if (isToken(Token::Type::LITERAL_INT, tokens, iterator))
            {
                int size = std::stoi(iterator->value);

                ++iterator;

                if (size <= 0)
                {
                    std::cerr << "Array size must be greater than zero" << std::endl;
                    return nullptr;
                }

                result->qualifiedType.dimensions.push_back(static_cast<uint32_t>(size));
            }
            else
            {
                std::cerr << "Expected an integer literal" << std::endl;
                return nullptr;
            }

            if (!isToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
            {
                std::cerr << "Expected a right bracket" << std::endl;
                return nullptr;
            }

            ++iterator;
        }

        if (!parseAttributes(tokens, iterator, attributes)) return nullptr;

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
                    {
                        std::cerr << "Invalid semantic " << attribute.second.front() << std::endl;
                    }

                    result->semantic = semantic;
                }
                else
                {
                    std::cerr << "Invalid parameters for attribute " << attribute.first << std::endl;
                }
            }
            else
            {
                std::cerr << "Invalid attribute " << attribute.first << std::endl;
            }
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

    ASTContext::Specifiers specifiers;
    if (!parseSpecifiers(tokens, iterator, specifiers)) return nullptr;

    bool isStatic = specifiers.isStatic;
    bool isInline = specifiers.isInline;

    if (!(result->qualifiedType.typeDeclaration = parseType(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    if (result->qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::STRUCT)
    {
        StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->qualifiedType.typeDeclaration);

        if (!structDeclaration->definition)
        {
            std::cerr << "Incomplete type " << result->qualifiedType.typeDeclaration->name << std::endl;
            return nullptr;
        }
    }

    if (!parseSpecifiers(tokens, iterator, specifiers)) return nullptr;

    if (specifiers.isStatic) isStatic = true;
    if (specifiers.isInline) isInline = true;

    if (isStatic)
    {
        std::cerr << "Parameters can not be static" << std::endl;
        return nullptr;
    }

    if (isInline)
    {
        std::cerr << "Parameters can not be inline" << std::endl;
        return nullptr;
    }

    if (!isToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected an identifier" << std::endl;
        return nullptr;
    }

    result->name = iterator->value;

    ++iterator;

    while (isToken(Token::Type::LEFT_BRACKET, tokens, iterator))
    {
        ++iterator;

        if (isToken(Token::Type::LITERAL_INT, tokens, iterator))
        {
            int size = std::stoi(iterator->value);

            ++iterator;

            if (size <= 0)
            {
                std::cerr << "Array size must be greater than zero" << std::endl;
                return nullptr;
            }

            result->qualifiedType.dimensions.push_back(static_cast<uint32_t>(size));
        }
        else
        {
            std::cerr << "Expected an integer literal" << std::endl;
            return nullptr;
        }

        if (!isToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
        {
            std::cerr << "Expected a right bracket" << std::endl;
            return nullptr;
        }

        ++iterator;
    }

    return result;
}

/*TypeDefinitionDeclaration* ASTContext::parseTypeDefinitionDeclaration(const std::vector<Token>& tokens,
                                                                        std::vector<Token>::const_iterator& iterator,
                                                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                                                        Construct* parent)
{
    std::cerr << "Typedef is not supported" << std::endl;
    return nullptr;

    TypeDefinitionDeclaration* result = new TypeDefinitionDeclaration();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;
    result->kind = Construct::Kind::DECLARATION;
    result->declarationKind = Declaration::Kind::TYPE;
    result->typeKind = TypeDeclaration::Kind::TYPE_DEFINITION;
    result->Declaration = findTypeDeclaration(iterator->value, declarationScopes);

    if (!(result->qualifiedType.typeDeclaration = parseType(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    if (result->qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::STRUCT)
    {
        StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->qualifiedType.typeDeclaration);

        if (!structDeclaration->hasDefinition)
        {
            std::cerr << "Incomplete type " << result->qualifiedType.typeDeclaration->name << std::endl;
            return nullptr;
        }
    }

    if (!isToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return nullptr;
    }

    result->name = iterator->value;

    ++iterator;

    if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        std::cerr << "Expected a semicolon" << std::endl;
        return nullptr;
    }

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

        if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::KEYWORD_CONTINUE, tokens, iterator))
    {
        ++iterator;

        ContinueStatement* result = new ContinueStatement();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;

        if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::KEYWORD_RETURN, tokens, iterator))
    {
        ++iterator;

        ReturnStatement* result = new ReturnStatement();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;

        if (!(result->result = parseCommaExpression(tokens, iterator, declarationScopes, result)))
        {
            std::cerr << "Expected an expression" << std::endl;
            return nullptr;
        }

        if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;

        return result;
    }
    else if (isDeclaration(tokens, iterator, declarationScopes))
    {
        DeclarationStatement* result = new DeclarationStatement();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;

        if (!(result->declaration = parseDeclaration(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }

        if (result->declaration->getDeclarationKind() != Declaration::Kind::VARIABLE)
        {
            std::cerr << "Expected a variable declaration" << std::endl;
            return nullptr;
        }

        if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

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

        if (!(result->expression = parseCommaExpression(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }

        if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

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
    if (!isToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        std::cerr << "Expected a left brace" << std::endl;
        return nullptr;
    }

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
            {
                std::cerr << "Failed to parse a statement" << std::endl;
                return nullptr;
            }

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
    if (!isToken(Token::Type::KEYWORD_IF, tokens, iterator))
    {
        std::cerr << "Expected the if keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    IfStatement* result = new IfStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    if (!isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }

        if (declaration->getDeclarationKind() != Declaration::Kind::VARIABLE &&
            declaration->getDeclarationKind() != Declaration::Kind::PARAMETER)
        {
            std::cerr << "Expected a variable declaration" << std::endl;
            return nullptr;
        }

        result->condition = declaration;
    }
    else
    {
        if (!(result->condition = parseCommaExpression(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }
    }

    if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    Statement* statement;
    if (!(statement = parseStatement(tokens, iterator, declarationScopes, result)))
    {
        std::cerr << "Failed to parse the statement" << std::endl;
        return nullptr;
    }

    result->body = statement;

    if (isToken(Token::Type::KEYWORD_ELSE, tokens, iterator))
    {
        ++iterator;

        if (!(statement = parseStatement(tokens, iterator, declarationScopes, result)))
        {
            std::cerr << "Failed to parse the statement" << std::endl;
            return nullptr;
        }

        result->elseBody = statement;
    }

    return result;
}

ForStatement* ASTContext::parseForStatement(const std::vector<Token>& tokens,
                                            std::vector<Token>::const_iterator& iterator,
                                            std::vector<std::vector<Declaration*>>& declarationScopes,
                                            Construct* parent)
{
    if (!isToken(Token::Type::KEYWORD_FOR, tokens, iterator))
    {
        std::cerr << "Expected the for keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    ForStatement* result = new ForStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    if (!isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }

        if (declaration->getDeclarationKind() != Declaration::Kind::VARIABLE &&
            declaration->getDeclarationKind() != Declaration::Kind::PARAMETER)
        {
            std::cerr << "Expected a variable declaration" << std::endl;
            return nullptr;
        }

        result->condition = declaration;

        if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;
    }
    else if (isToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        ++iterator;

        result->initialization = nullptr;
    }
    else
    {
        if (!(result->initialization = parseCommaExpression(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }

        if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;
    }

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }

        if (declaration->getDeclarationKind() != Declaration::Kind::VARIABLE &&
            declaration->getDeclarationKind() != Declaration::Kind::PARAMETER)
        {
            std::cerr << "Expected a variable declaration" << std::endl;
            return nullptr;
        }

        result->condition = declaration;

        if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;
    }
    else if (isToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        ++iterator;

        result->condition = nullptr;
    }
    else
    {
        if (!(result->condition = parseCommaExpression(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }

        if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;
    }

    if (isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        ++iterator;

        result->increment = nullptr;
    }
    else
    {
        if (!(result->increment = parseCommaExpression(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }

        if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return nullptr;
        }

        ++iterator;
    }

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes, result)))
    {
        return nullptr;
    }

    return result;
}

SwitchStatement* ASTContext::parseSwitchStatement(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  Construct* parent)
{
    if (!isToken(Token::Type::KEYWORD_SWITCH, tokens, iterator))
    {
        std::cerr << "Expected the switch keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    SwitchStatement* result = new SwitchStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    if (!isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }

        if (declaration->getDeclarationKind() != Declaration::Kind::VARIABLE &&
            declaration->getDeclarationKind() != Declaration::Kind::PARAMETER)
        {
            std::cerr << "Expected a variable declaration" << std::endl;
            return nullptr;
        }

        result->condition = declaration;
    }
    else
    {
        if (!(result->condition = parseCommaExpression(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }
    }

    if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes, result)))
    {
        return nullptr;
    }

    return result;
}

CaseStatement* ASTContext::parseCaseStatement(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent)
{
    if (!isToken(Token::Type::KEYWORD_CASE, tokens, iterator))
    {
        std::cerr << "Expected the case keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    CaseStatement* result = new CaseStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    if (!(result->condition = parseCommaExpression(tokens, iterator, declarationScopes, result)))
    {
        std::cerr << "Expected an expression" << std::endl;
        return nullptr;
    }

    if (!isToken(Token::Type::COLON, tokens, iterator))
    {
        std::cerr << "Expected a colon" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes, result)))
    {
        return nullptr;
    }

    return result;
}

DefaultStatement* ASTContext::parseDefaultStatement(const std::vector<Token>& tokens,
                                                    std::vector<Token>::const_iterator& iterator,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                                    Construct* parent)
{
    if (!isToken(Token::Type::KEYWORD_DEFAULT, tokens, iterator))
    {
        std::cerr << "Expected the default keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    DefaultStatement* result = new DefaultStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    if (!isToken(Token::Type::COLON, tokens, iterator))
    {
        std::cerr << "Expected a colon" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes, result)))
    {
        return nullptr;
    }

    return result;
}

WhileStatement* ASTContext::parseWhileStatement(const std::vector<Token>& tokens,
                                                std::vector<Token>::const_iterator& iterator,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                Construct* parent)
{
    if (!isToken(Token::Type::KEYWORD_WHILE, tokens, iterator))
    {
        std::cerr << "Expected the while keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    WhileStatement* result = new WhileStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    if (!isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }

        if (declaration->getDeclarationKind() != Declaration::Kind::VARIABLE &&
            declaration->getDeclarationKind() != Declaration::Kind::PARAMETER)
        {
            std::cerr << "Expected a variable declaration" << std::endl;
            return nullptr;
        }

        result->condition = declaration;
    }
    else
    {
        if (!(result->condition = parseCommaExpression(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }
    }

    if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes, result)))
    {
        return nullptr;
    }

    return result;
}

DoStatement* ASTContext::parseDoStatement(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent)
{
    if (!isToken(Token::Type::KEYWORD_DO, tokens, iterator))
    {
        std::cerr << "Expected the do keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    DoStatement* result = new DoStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = parent;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes, result)))
    {
        return nullptr;
    }

    if (!isToken(Token::Type::KEYWORD_WHILE, tokens, iterator))
    {
        std::cerr << "Expected a \"while\" keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    // expression
    if (!(result->condition = parseCommaExpression(tokens, iterator, declarationScopes, result)))
    {
        return nullptr;
    }

    if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!isToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        std::cerr << "Expected a semicolon" << std::endl;
        return nullptr;
    }

    ++iterator;

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
        result->qualifiedType.typeDeclaration = &intTypeDeclaration;
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
        result->qualifiedType.typeDeclaration = &floatTypeDeclaration;
        result->isLValue = false;
        result->value = strtod(iterator->value.c_str(), nullptr);

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::LITERAL_STRING, tokens, iterator))
    {
        StringLiteralExpression* result = new StringLiteralExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;
        result->qualifiedType.typeDeclaration = &stringTypeDeclaration;
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
        result->qualifiedType.typeDeclaration = &boolTypeDeclaration;
        result->isLValue = false;
        result->value = (iterator->type == Token::Type::KEYWORD_TRUE);

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::KEYWORD_BOOL, tokens, iterator) ||
             isToken(Token::Type::KEYWORD_INT, tokens, iterator) ||
             isToken(Token::Type::KEYWORD_FLOAT, tokens, iterator))
    {
        CastExpression* result = new CastExpression(CastExpression::Kind::EXPLICIT);
        constructs.push_back(std::unique_ptr<Construct>(result));

        if (isToken(Token::Type::KEYWORD_BOOL, tokens, iterator)) result->qualifiedType.typeDeclaration = &boolTypeDeclaration;
        else if(isToken(Token::Type::KEYWORD_INT, tokens, iterator)) result->qualifiedType.typeDeclaration = &intTypeDeclaration;
        else if(isToken(Token::Type::KEYWORD_FLOAT, tokens, iterator)) result->qualifiedType.typeDeclaration = &floatTypeDeclaration;

        ++iterator;

        if (!isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a left parenthesis" << std ::endl;
            return nullptr;
        }

        ++iterator;

        result->expression = parseMultiplicationAssignmentExpression(tokens, iterator, declarationScopes, result);

        if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std ::endl;
            return nullptr;
        }

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        ++iterator;

        InitializerListExpression* result = new InitializerListExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));

        if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            for (;;)
            {
                Expression* expression;
                if (!(expression = parseMultiplicationAssignmentExpression(tokens, iterator, declarationScopes, result)))
                {
                    return nullptr;
                }

                if (!result->qualifiedType.typeDeclaration)
                {
                    result->qualifiedType.typeDeclaration = expression->qualifiedType.typeDeclaration;
                    result->qualifiedType.dimensions.push_back(0);
                }
                else
                {
                    if (result->qualifiedType.typeDeclaration != expression->qualifiedType.typeDeclaration)
                    {
                        // TODO: implement type narrowing
                        std::cerr << "Expression type does not match previous expressions in initializer list" << std ::endl;
                        return nullptr;
                    }

                    ++result->qualifiedType.dimensions.back();
                }

                result->expressions.push_back(expression);

                if (!isToken(Token::Type::COMMA, tokens, iterator))
                {
                    break;
                }

                ++iterator;
            }
        }

        if (!isToken(Token::Type::RIGHT_BRACE, tokens, iterator))
        {
            std::cerr << "Expected a right brace" << std ::endl;
            return nullptr;
        }

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
                        {
                            return nullptr;
                        }

                        result->parameters.push_back(parameter);
                        parameters.push_back(parameter->qualifiedType);

                        if (isToken(Token::Type::COMMA, tokens, iterator))
                            ++iterator;
                        else
                            break;
                    }

                    if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
                    {
                        std::cerr << "Expected a right parenthesis" << std::endl;
                        return nullptr;
                    }

                    ++iterator;
                }

                if (typeDeclaration->getTypeKind() != TypeDeclaration::Kind::STRUCT)
                {
                    std::cerr << "Expected a struct type" << std::endl;
                    return nullptr;
                }

                StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(typeDeclaration);

                if (!(result->constructorDeclaration = structDeclaration->findConstructorDeclaration(parameters)))
                {
                    std::cerr << "No matching constructor found" << std::endl;
                    return nullptr;
                }

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
                        {
                            return nullptr;
                        }

                        result->parameters.push_back(parameter);
                        parameters.push_back(parameter->qualifiedType);

                        if (isToken(Token::Type::COMMA, tokens, iterator))
                            ++iterator;
                        else
                            break;
                    }

                    if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
                    {
                        std::cerr << "Expected a right parenthesis" << std::endl;
                        return nullptr;
                    }

                    ++iterator;
                }

                DeclarationReferenceExpression* declRefExpression = new DeclarationReferenceExpression();
                constructs.push_back(std::unique_ptr<Construct>(declRefExpression));
                declRefExpression->parent = result;

                FunctionDeclaration* functionDeclaration = findFunctionDeclaration(name, declarationScopes, parameters);

                if (!functionDeclaration)
                {
                    std::cerr << "Invalid function reference: " << name << std::endl;
                    return nullptr;
                }

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
            {
                std::cerr << "Invalid declaration reference: " << name << std::endl;
                return nullptr;
            }

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
                    std::cerr << "Invalid declaration reference " << name << std::endl;
                    return nullptr;
            }

            return result;
        }
    }
    else if (isToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        ++iterator;

        ParenExpression* result = new ParenExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;

        if (!(result->expression = parseCommaExpression(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }

        if (!isToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return nullptr;
        }

        ++iterator;

        result->qualifiedType = result->expression->qualifiedType;
        result->isLValue = result->expression->isLValue;

        return result;
    }
    else
    {
        std::cerr << "Expected an expression" << std::endl;
        return nullptr;
    }

    return nullptr;
}

Expression* ASTContext::parseSubscriptExpression(const std::vector<Token>& tokens,
                                                 std::vector<Token>::const_iterator& iterator,
                                                 std::vector<std::vector<Declaration*>>& declarationScopes,
                                                 Construct* parent)
{
    Expression* result;
    if (!(result = parsePrimaryExpression(tokens, iterator, declarationScopes, parent)))
    {
        return nullptr;
    }

    while (isToken(Token::Type::LEFT_BRACKET, tokens, iterator))
    {
        ++iterator;

        ArraySubscriptExpression* expression = new ArraySubscriptExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;
        expression->expression = result;

        if (!result->qualifiedType.typeDeclaration)
        {
            std::cerr << "Subscript expression with a void type" << std::endl;
            return nullptr;
        }

        if (result->qualifiedType.dimensions.empty())
        {
            std::cerr << "Subscript value is not an array" << std::endl;
            return nullptr;
        }

        if (!(expression->subscript = parseCommaExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

        if (!isToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
        {
            std::cerr << "Expected a right brace" << std::endl;
            return nullptr;
        }

        ++iterator;

        expression->qualifiedType = result->qualifiedType;
        expression->qualifiedType.dimensions.erase(expression->qualifiedType.dimensions.begin());
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
    {
        return nullptr;
    }

    while (isToken(Token::Type::OPERATOR_DOT, tokens, iterator))
    {
        ++iterator;

        MemberExpression* expression = new MemberExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;
        expression->expression = result;

        if (!result->qualifiedType.typeDeclaration)
        {
            std::cerr << "Expression has a void type" << std::endl;
            return nullptr;
        }

        if (result->qualifiedType.typeDeclaration->getTypeKind() != TypeDeclaration::Kind::STRUCT)
        {
            std::cerr << result->qualifiedType.typeDeclaration->name << " is not a structure" << std::endl;
            return nullptr;
        }

        StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->qualifiedType.typeDeclaration);

        if (!isToken(Token::Type::IDENTIFIER, tokens, iterator))
        {
            std::cerr << "Expected an identifier" << std::endl;
            return nullptr;
        }

        Declaration* memberDeclaration = structDeclaration->findMemberDeclaration(iterator->value);

        if (!memberDeclaration)
        {
            std::cerr << "Structure " << structDeclaration->name <<  " has no member " << iterator->value << std::endl;
            return nullptr;
        }

        if (memberDeclaration->getDeclarationKind() != Declaration::Kind::FIELD)
        {
            std::cerr << iterator->value << " is not a field" << std::endl;
            return nullptr;
        }

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

        if (iterator->type == Token::Type::OPERATOR_PLUS) result->operatorKind = UnaryOperatorExpression::Kind::POSITIVE;
        else if (iterator->type == Token::Type::OPERATOR_MINUS) result->operatorKind = UnaryOperatorExpression::Kind::NEGATIVE;

        ++iterator;

        if (!(result->expression = parseMemberExpression(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }

        if (!result->expression->qualifiedType.typeDeclaration)
        {
            std::cerr << "Unary expression with a void type" << std::endl;
            return nullptr;
        }

        if (result->expression->qualifiedType.typeDeclaration == &boolTypeDeclaration)
        {
            CastExpression* castExpression = new CastExpression(CastExpression::Kind::IMPLICIT);
            constructs.push_back(std::unique_ptr<Construct>(castExpression));
            castExpression->parent = result;
            castExpression->qualifiedType.typeDeclaration = &intTypeDeclaration;

            castExpression->expression = result->expression;
            result->expression->parent = castExpression;
            result->expression = castExpression;
        }

        result->qualifiedType = result->expression->qualifiedType;
        result->isLValue = false;

        return result;
    }
    else
    {
        Expression* result;
        if (!(result = parseMemberExpression(tokens, iterator, declarationScopes, parent)))
        {
            return nullptr;
        }

        return result;
    }
}

Expression* ASTContext::parseNotExpression(const std::vector<Token>& tokens,
                                           std::vector<Token>::const_iterator& iterator,
                                           std::vector<std::vector<Declaration*>>& declarationScopes,
                                           Construct* parent)
{
    if (isToken(Token::Type::OPERATOR_NOT, tokens, iterator))
    {
        UnaryOperatorExpression* result = new UnaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->parent = parent;
        result->operatorKind = UnaryOperatorExpression::Kind::NEGATION;

        ++iterator;

        if (!(result->expression = parseSignExpression(tokens, iterator, declarationScopes, result)))
        {
            return nullptr;
        }

        if (!result->expression->qualifiedType.typeDeclaration)
        {
            std::cerr << "Unary expression with a void type" << std::endl;
            return nullptr;
        }

        if (result->expression->qualifiedType.typeDeclaration == &intTypeDeclaration ||
            result->expression->qualifiedType.typeDeclaration == &floatTypeDeclaration)
        {
            CastExpression* castExpression = new CastExpression(CastExpression::Kind::IMPLICIT);
            constructs.push_back(std::unique_ptr<Construct>(castExpression));
            castExpression->parent = result;
            castExpression->qualifiedType.typeDeclaration = &boolTypeDeclaration;

            castExpression->expression = result->expression;
            result->expression->parent = castExpression;
            result->expression = castExpression;
        }

        result->qualifiedType.typeDeclaration = &boolTypeDeclaration;
        result->isLValue = false;

        return result;
    }
    else
    {
        Expression* result;
        if (!(result = parseSignExpression(tokens, iterator, declarationScopes, parent)))
        {
            return nullptr;
        }

        return result;
    }
}

Expression* ASTContext::parseMultiplicationExpression(const std::vector<Token>& tokens,
                                                      std::vector<Token>::const_iterator& iterator,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                                      Construct* parent)
{
    Expression* result;
    if (!(result = parseNotExpression(tokens, iterator, declarationScopes, parent)))
    {
        return nullptr;
    }

    while (isToken({Token::Type::OPERATOR_MULTIPLY, Token::Type::OPERATOR_DIVIDE}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        if (iterator->type == Token::Type::OPERATOR_MULTIPLY) expression->operatorKind = BinaryOperatorExpression::Kind::MULTIPLICATION;
        else if (iterator->type == Token::Type::OPERATOR_DIVIDE) expression->operatorKind = BinaryOperatorExpression::Kind::DIVISION;

        expression->leftExpression = result;

        ++iterator;

        if (!(expression->rightExpression = parseNotExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

        if (!expression->leftExpression->qualifiedType.typeDeclaration ||
            !expression->rightExpression->qualifiedType.typeDeclaration)
        {
            std::cerr << "Binary expression with a void type" << std::endl;
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType = expression->leftExpression->qualifiedType;
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
    {
        return nullptr;
    }

    while (isToken({Token::Type::OPERATOR_PLUS, Token::Type::OPERATOR_MINUS}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        if (iterator->type == Token::Type::OPERATOR_PLUS) expression->operatorKind = BinaryOperatorExpression::Kind::ADDITION;
        else if (iterator->type == Token::Type::OPERATOR_MINUS) expression->operatorKind = BinaryOperatorExpression::Kind::SUBTRACTION;

        expression->leftExpression = result;

        ++iterator;

        if (!(expression->rightExpression = parseMultiplicationExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

        if (!expression->leftExpression->qualifiedType.typeDeclaration ||
            !expression->rightExpression->qualifiedType.typeDeclaration)
        {
            std::cerr << "Binary expression with a void type" << std::endl;
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType = expression->leftExpression->qualifiedType;
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
    {
        return nullptr;
    }

    while (isToken({Token::Type::OPERATOR_LESS_THAN, Token::Type::OPERATOR_LESS_THAN_EQUAL}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        if (iterator->type == Token::Type::OPERATOR_LESS_THAN) expression->operatorKind = BinaryOperatorExpression::Kind::LESS_THAN;
        else if (iterator->type == Token::Type::OPERATOR_LESS_THAN_EQUAL) expression->operatorKind = BinaryOperatorExpression::Kind::LESS_THAN_EQUAL;

        ++iterator;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseAdditionExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

        if (!expression->leftExpression->qualifiedType.typeDeclaration ||
            !expression->rightExpression->qualifiedType.typeDeclaration)
        {
            std::cerr << "Binary expression with a void type" << std::endl;
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType = expression->leftExpression->qualifiedType;
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
    {
        return nullptr;
    }

    while (isToken({Token::Type::OPERATOR_GREATER_THAN, Token::Type::OPERATOR_GREATER_THAN_EQUAL}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        if (iterator->type == Token::Type::OPERATOR_GREATER_THAN) expression->operatorKind = BinaryOperatorExpression::Kind::GREATER_THAN;
        else if (iterator->type == Token::Type::OPERATOR_GREATER_THAN_EQUAL) expression->operatorKind = BinaryOperatorExpression::Kind::GREATER_THAN_EQUAL;

        ++iterator;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseLessThanExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

        if (!expression->leftExpression->qualifiedType.typeDeclaration ||
            !expression->rightExpression->qualifiedType.typeDeclaration)
        {
            std::cerr << "Binary expression with a void type" << std::endl;
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType = expression->leftExpression->qualifiedType;
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
    {
        return nullptr;
    }

    while (isToken({Token::Type::OPERATOR_EQUAL, Token::Type::OPERATOR_NOT_EQUAL}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        if (iterator->type == Token::Type::OPERATOR_EQUAL) expression->operatorKind = BinaryOperatorExpression::Kind::EQUALITY;
        else if (iterator->type == Token::Type::OPERATOR_NOT_EQUAL) expression->operatorKind = BinaryOperatorExpression::Kind::INEQUALITY;

        ++iterator;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseGreaterThanExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

        if (!expression->leftExpression->qualifiedType.typeDeclaration ||
            !expression->rightExpression->qualifiedType.typeDeclaration)
        {
            std::cerr << "Binary expression with a void type" << std::endl;
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType = expression->leftExpression->qualifiedType;
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
    {
        return nullptr;
    }

    while (isToken(Token::Type::OPERATOR_AND, tokens, iterator))
    {
        ++iterator;

        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;
        expression->operatorKind = BinaryOperatorExpression::Kind::AND;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseEqualityExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

        if (!expression->leftExpression->qualifiedType.typeDeclaration ||
            !expression->rightExpression->qualifiedType.typeDeclaration)
        {
            std::cerr << "Binary expression with a void type" << std::endl;
            return nullptr;
        }

        // TODO: check if both sides ar scalar
        expression->qualifiedType.typeDeclaration = &boolTypeDeclaration;
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
    {
        return nullptr;
    }

    while (isToken(Token::Type::OPERATOR_OR, tokens, iterator))
    {
        ++iterator;

        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;
        expression->operatorKind = BinaryOperatorExpression::Kind::OR;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseLogicalAndExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

        if (!expression->leftExpression->qualifiedType.typeDeclaration ||
            !expression->rightExpression->qualifiedType.typeDeclaration)
        {
            std::cerr << "Binary expression with a void type" << std::endl;
            return nullptr;
        }

        // TODO: check if both sides ar scalar
        expression->qualifiedType.typeDeclaration = &boolTypeDeclaration;
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
    {
        return nullptr;
    }

    while (isToken(Token::Type::OPERATOR_CONDITIONAL, tokens, iterator))
    {
        ++iterator;

        TernaryOperatorExpression* expression = new TernaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;
        expression->condition = result;

        if (!expression->condition->qualifiedType.typeDeclaration)
        {
            std::cerr << "Ternary expression with a void condition" << std::endl;
            return nullptr;
        }

        if (!(expression->leftExpression = parseTernaryExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

        if (!isToken(Token::Type::COLON, tokens, iterator))
        {
            std::cerr << "Expected a colon" << std::endl;
            return nullptr;
        }

        ++iterator;

        if (!(expression->rightExpression = parseTernaryExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

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
    {
        return nullptr;
    }

    while (isToken(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
    {
        ++iterator;

        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;
        expression->operatorKind = BinaryOperatorExpression::Kind::ASSIGNMENT;
        expression->leftExpression = result;

        if (expression->leftExpression->qualifiedType.isConst)
        {
            std::cerr << "Cannot assign to const variable" << std::endl;
            return nullptr;
        }

        if (!expression->leftExpression->isLValue)
        {
            std::cerr << "Expression is not assignable" << std::endl;
            return nullptr;
        }

        if (!(expression->rightExpression = parseTernaryExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

        if (!expression->leftExpression->qualifiedType.typeDeclaration ||
            !expression->rightExpression->qualifiedType.typeDeclaration)
        {
            std::cerr << "Binary expression with a void type" << std::endl;
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType = expression->leftExpression->qualifiedType;
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
    {
        return nullptr;
    }

    while (isToken({Token::Type::OPERATOR_PLUS_ASSIGNMENT, Token::Type::OPERATOR_MINUS_ASSIGNMENT}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        if (iterator->type == Token::Type::OPERATOR_PLUS_ASSIGNMENT) expression->operatorKind = BinaryOperatorExpression::Kind::ADDITION_ASSIGNMENT;
        else if (iterator->type == Token::Type::OPERATOR_MINUS_ASSIGNMENT) expression->operatorKind = BinaryOperatorExpression::Kind::SUBTRACTION_ASSIGNMENT;

        ++iterator;

        expression->leftExpression = result;

        if (expression->leftExpression->qualifiedType.isConst)
        {
            std::cerr << "Cannot assign to const variable" << std::endl;
            return nullptr;
        }

        if (!expression->leftExpression->isLValue)
        {
            std::cerr << "Expression is not assignable" << std::endl;
            return nullptr;
        }

        if (!(expression->rightExpression = parseAssignmentExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

        if (!expression->leftExpression->qualifiedType.typeDeclaration ||
            !expression->rightExpression->qualifiedType.typeDeclaration)
        {
            std::cerr << "Binary expression with a void type" << std::endl;
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType = expression->leftExpression->qualifiedType;
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
    {
        return nullptr;
    }

    while (isToken({Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT, Token::Type::OPERATOR_DIVIDE_ASSIGNMENT}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;

        if (iterator->type == Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT) expression->operatorKind = BinaryOperatorExpression::Kind::MULTIPLICATION_ASSIGNMENT;
        else if (iterator->type == Token::Type::OPERATOR_DIVIDE_ASSIGNMENT) expression->operatorKind = BinaryOperatorExpression::Kind::DIVISION_ASSIGNMENT;

        ++iterator;

        expression->leftExpression = result;

        if (expression->leftExpression->qualifiedType.isConst)
        {
            std::cerr << "Cannot assign to const variable" << std::endl;
            return nullptr;
        }

        if (!expression->leftExpression->isLValue)
        {
            std::cerr << "Expression is not assignable" << std::endl;
            return nullptr;
        }

        std::unique_ptr<Construct> right;
        if (!(expression->rightExpression = parseAdditionAssignmentExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

        if (!expression->leftExpression->qualifiedType.typeDeclaration ||
            !expression->rightExpression->qualifiedType.typeDeclaration)
        {
            std::cerr << "Binary expression with a void type" << std::endl;
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType = expression->leftExpression->qualifiedType;
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
    {
        return nullptr;
    }

    while (isToken(Token::Type::COMMA, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->parent = parent;
        expression->operatorKind = BinaryOperatorExpression::Kind::COMMA;
        expression->leftExpression = result;

        ++iterator;

        std::unique_ptr<Construct> right;
        if (!(expression->rightExpression = parseAdditionAssignmentExpression(tokens, iterator, declarationScopes, expression)))
        {
            return nullptr;
        }

        expression->qualifiedType = expression->rightExpression->qualifiedType;
        expression->isLValue = expression->rightExpression->isLValue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

void ASTContext::dump() const
{
    for (Declaration* declaration : declarations)
    {
        dumpConstruct(declaration);
    }
}

void ASTContext::dumpDeclaration(const Declaration* declaration, std::string indent) const
{
    std::cout << " " << declarationKindToString(declaration->getDeclarationKind());

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

            std::cout << " " << typeKindToString(typeDeclaration->getTypeKind());

            switch (typeDeclaration->getTypeKind())
            {
                case TypeDeclaration::Kind::NONE:
                {
                    break;
                }

                case TypeDeclaration::Kind::STRUCT:
                {
                    const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(typeDeclaration);
                    std::cout << ", name: " << structDeclaration->name;

                    if (structDeclaration->previousDeclaration)
                    {
                        std::cout << ", previous declaration: " << structDeclaration->previousDeclaration;
                    }

                    if (structDeclaration->definition)
                    {
                        std::cout << ", definition: " << structDeclaration->definition;
                    }

                    std::cout << std::endl;

                    for (const Declaration* memberDeclaration : structDeclaration->memberDeclarations)
                    {
                        dumpConstruct(memberDeclaration, indent + " ");
                    }

                    break;
                }

                case TypeDeclaration::Kind::SCALAR:
                {
                    const ScalarTypeDeclaration* scalarTypeDeclaration = static_cast<const ScalarTypeDeclaration*>(typeDeclaration);
                    std::cout << ", name: " << scalarTypeDeclaration->name << ", scalar type kind: " << scalarTypeKindToString(scalarTypeDeclaration->getScalarTypeKind());
                    break;
                }
            }
            break;
        }

        case Declaration::Kind::FIELD:
        {
            const FieldDeclaration* fieldDeclaration = static_cast<const FieldDeclaration*>(declaration);

            std::cout << ", name: " << fieldDeclaration->name << ", type: " << fieldDeclaration->qualifiedType.typeDeclaration->name;

            if (fieldDeclaration->semantic != Semantic::NONE)
            {
                std::cout << ", semantic: " << semanticToString(fieldDeclaration->semantic);
            }

            std::cout << std::endl;
            break;
        }

        case Declaration::Kind::CONSTRUCTOR:
        {
            const ConstructorDeclaration* constructorDeclaration = static_cast<const ConstructorDeclaration*>(declaration);

            for (ParameterDeclaration* parameter : constructorDeclaration->parameterDeclarations)
            {
                dumpConstruct(parameter, indent + "  ");
            }
        }

        case Declaration::Kind::FUNCTION:
        {
            const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(declaration);

            std::cout << ", name: " << functionDeclaration->name;

            std::cout << ", result type: ";

            if (functionDeclaration->qualifiedType.typeDeclaration)
            {
                std::cout << functionDeclaration->qualifiedType.typeDeclaration->name;
            }
            else
            {
                std::cout << "void";
            }

            if (functionDeclaration->isStatic) std::cout << " static";
            if (functionDeclaration->isInline) std::cout << " inline";

            if (functionDeclaration->program != FunctionDeclaration::Program::NONE)
            {
                std::cout << ", program: " << programToString(functionDeclaration->program);
            }

            if (functionDeclaration->previousDeclaration)
            {
                std::cout << ", previous declaration: " << functionDeclaration->previousDeclaration;
            }

            if (functionDeclaration->definition)
            {
                std::cout << ", definition: " << functionDeclaration->definition;
            }

            std::cout << std::endl;

            for (ParameterDeclaration* parameter : functionDeclaration->parameterDeclarations)
            {
                dumpConstruct(parameter, indent + "  ");
            }

            if (functionDeclaration->body)
            {
                dumpConstruct(functionDeclaration->body, indent + "  ");
            }
            break;
        }

        case Declaration::Kind::VARIABLE:
        {
            const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(declaration);
            std::cout << ", name: " << variableDeclaration->name << ", type: " << variableDeclaration->qualifiedType.typeDeclaration->name << std::endl;

            if (variableDeclaration->initialization)
            {
                dumpConstruct(variableDeclaration->initialization, indent + "  ");
            }

            break;
        }

        case Declaration::Kind::PARAMETER:
        {
            const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(declaration);
            std::cout << ", name: " << parameterDeclaration->name << ", type: " << parameterDeclaration->qualifiedType.typeDeclaration->name << std::endl;
            break;
        }

        default:
            break;
    }
}

void ASTContext::dumpStatement(const Statement* statement, std::string indent) const
{
    std::cout << " " << statementKindToString(statement->getStatementKind());

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

            for (Statement* statement : compoundStatement->statements)
            {
                dumpConstruct(statement, indent + "  ");
            }
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
    std::cout << " " << expressionKindToString(expression->getExpressionKind()) << ", lvalue: " << (expression->isLValue ? "true" : "false");

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
            {
                dumpConstruct(parameter, indent + "  ");
            }

            break;
        }

        case Expression::Kind::LITERAL:
        {
            const LiteralExpression* literalExpression = static_cast<const LiteralExpression*>(expression);

            std::cout << ", literal kind: " << literalKindToString(literalExpression->getLiteralKind()) << ", value: ";

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

        case Expression::Kind::UNARY:
        {
            const UnaryOperatorExpression* unaryOperatorExpression = static_cast<const UnaryOperatorExpression*>(expression);

            std::cout <<", operator: " << unaryOperatorKindToString(unaryOperatorExpression->operatorKind) << std::endl;

            dumpConstruct(unaryOperatorExpression->expression, indent + "  ");
            break;
        }

        case Expression::Kind::BINARY:
        {
            const BinaryOperatorExpression* binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(expression);

            std::cout << ", operator: " << binaryOperatorKindToString(binaryOperatorExpression->operatorKind) << std::endl;

            dumpConstruct(binaryOperatorExpression->leftExpression, indent + "  ");
            dumpConstruct(binaryOperatorExpression->rightExpression, indent + "  ");
            break;
        }

        case Expression::Kind::TERNARY:
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
            {
                dumpConstruct(parameter, indent + "  ");
            }

            break;
        }

        case Expression::Kind::INITIALIZER_LIST:
        {
            const InitializerListExpression* initializerListExpression = static_cast<const InitializerListExpression*>(expression);

            std::cout << std::endl;

            for (Expression* expression : initializerListExpression->expressions)
            {
                dumpConstruct(expression, indent + "  ");
            }

            break;
        }

        case Expression::Kind::CAST:
        {
            const CastExpression* castExpression = static_cast<const CastExpression*>(expression);

            std::cout << " " << castKindToString(castExpression->getCastKind()) << " " <<
                castExpression->expression->qualifiedType.typeDeclaration->name << " to " <<
                castExpression->qualifiedType.typeDeclaration->name << std::endl;

            dumpConstruct(castExpression->expression, indent + "  ");

            break;
        }
    }
}

void ASTContext::dumpConstruct(const Construct* construct, std::string indent) const
{
    std::cout << indent << construct;
    if (construct->parent) std::cout << ", parent: " << construct->parent;
    std::cout << " " << constructKindToString(construct->getKind());

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
