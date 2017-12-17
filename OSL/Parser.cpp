//
//  OSL
//

#include <iostream>
#include "Parser.hpp"

static const std::vector<std::string> builtinTypes = {
    "void", "bool", "int", "uint", "float", "double",
    "vec2", "vec3", "vec4", "mat3", "mat4"
};

ASTContext::ASTContext()
{
    boolType.name = "bool";
    boolType.scalar = true;
    boolType.isBuiltin = true;

    intType.name = "int";
    intType.scalar = true;
    intType.isBuiltin = true;

    floatType.name = "float";
    floatType.scalar = true;
    floatType.isBuiltin = true;

    float2Type.name = "float2";
    float2Type.isBuiltin = true;
    float2Type.hasDefinition = true;

    float3Type.name = "float3";
    float3Type.isBuiltin = true;
    float3Type.hasDefinition = true;

    float4Type.name = "float4";
    float4Type.isBuiltin = true;
    float4Type.hasDefinition = true;

    /*for (char first : {'x', 'y', 'z', 'w'})
        for (char second : {'x', 'y', 'z', 'w'})
            for (char third : {'x', 'y', 'z', 'w'})
                for (char fourth : {'x', 'y', 'z', 'w'})
                {
                    ParameterDeclaration parameter;
                }*/

    float3x3Type.name = "float3x3";
    float3x3Type.isBuiltin = true;
    float3x3Type.hasDefinition = true;

    float4x4Type.name = "float4x4";
    float4x4Type.isBuiltin = true;
    float4x4Type.hasDefinition = true;

    stringType.name = "string";
    stringType.isBuiltin = true;
    stringType.hasDefinition = true;

    samplerStateType.name = "SamplerState";
    samplerStateType.isBuiltin = true;
    samplerStateType.hasDefinition = true;

    texture2DType.name = "Texture2D";
    texture2DType.isBuiltin = true;
    texture2DType.hasDefinition = true;

    samplerParameter.name = "sampler";
    samplerParameter.qualifiedType.typeDeclaration = &samplerStateType;
    samplerParameter.qualifiedType.isConst = true;

    coordParameter.name = "coord";
    coordParameter.qualifiedType.typeDeclaration = &float2Type;
    coordParameter.qualifiedType.isConst = true;

    mulFunction.name = "texture2D";
    mulFunction.qualifiedType.typeDeclaration = &float4Type;
    mulFunction.parameterDeclarations.push_back(&samplerParameter);
    mulFunction.parameterDeclarations.push_back(&coordParameter);
    mulFunction.isBuiltin = true;
}

bool ASTContext::parse(const std::vector<Token>& tokens)
{
    constructs.clear();
    declarations.clear();

    auto iterator = tokens.cbegin();

    std::vector<std::vector<Declaration*>> declarationScopes;
    declarationScopes.push_back(std::vector<Declaration*>());

    declarationScopes.back().push_back(&boolType);
    declarationScopes.back().push_back(&intType);
    declarationScopes.back().push_back(&floatType);
    declarationScopes.back().push_back(&float2Type);
    declarationScopes.back().push_back(&float3Type);
    declarationScopes.back().push_back(&float4Type);
    declarationScopes.back().push_back(&float3x3Type);
    declarationScopes.back().push_back(&float4x4Type);
    declarationScopes.back().push_back(&stringType);
    declarationScopes.back().push_back(&samplerStateType);
    declarationScopes.back().push_back(&texture2DType);
    declarationScopes.back().push_back(&mulFunction);

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

bool ASTContext::isDeclaration(const std::vector<Token>& tokens,
                               std::vector<Token>::const_iterator iterator,
                               std::vector<std::vector<Declaration*>>& declarationScopes) const
{
    if (iterator == tokens.end()) return false;

    if (iterator->type == Token::Type::KEYWORD_CONST ||
        iterator->type == Token::Type::KEYWORD_STATIC ||
        iterator->type == Token::Type::KEYWORD_STRUCT)
    {
        return true;
    }
    else if (iterator->type == Token::Type::IDENTIFIER)
    {
        TypeDeclaration* typeDeclaration = findTypeDeclaration(iterator->value, declarationScopes);

        if (typeDeclaration) return true;
    }

    return false;
}

Declaration* ASTContext::parseTopLevelDeclaration(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        ++iterator;

        Declaration* declaration = new Declaration(Declaration::Kind::EMPTY);
        constructs.push_back(std::unique_ptr<Construct>(declaration));

        return declaration;
    }
    else
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes)))
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
                if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
                {
                    std::cerr << "Expected a semicolon" << std::endl;
                    return nullptr;
                }
                
                ++iterator;
            }
        }
        else
        {
            if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
            {
                std::cerr << "Expected a semicolon" << std::endl;
                return nullptr;
            }

            ++iterator;
        }

        return declaration;
    }

    return nullptr;
}

Declaration* ASTContext::parseDeclaration(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (checkToken(Token::Type::KEYWORD_STRUCT, tokens, iterator))
    {
        ++iterator;

        StructDeclaration* declaration;
        if (!(declaration = parseStructDeclaration(tokens, iterator, declarationScopes)))
        {
            std::cerr << "Failed to parse a structure declaration" << std::endl;
            return nullptr;
        }

        return declaration;
    }
    /*else if (checkToken(Token::Type::KEYWORD_TYPEDEF, tokens, iterator))
    {
        ++iterator;

        TypeDefinitionDeclaration* declaration;
        if (!(declaration = parseTypeDefinitionDeclaration(tokens, iterator, declarationScopes)))
        {
            std::cerr << "Failed to parse a type definition declaration" << std::endl;
            return nullptr;
        }

        return declaration;
    }*/
    else
    {
        QualifiedType qualifiedType;
        bool isStatic = false;

        for (;;)
        {
            if (checkToken(Token::Type::KEYWORD_CONST, tokens, iterator))
            {
                ++iterator;
                qualifiedType.isConst = true;
            }
            else if (checkToken(Token::Type::KEYWORD_STATIC, tokens, iterator))
            {
                ++iterator;
                isStatic = true;
            }
            else break;
        }

        if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
        {
            std::cerr << "Expected a type name" << std::endl;
            return nullptr;
        }

        qualifiedType.typeDeclaration = findTypeDeclaration(iterator->value, declarationScopes);

        if (!qualifiedType.typeDeclaration)
        {
            std::cerr << "Invalid type: " << iterator->value << std::endl;
            return nullptr;
        }

        if (qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::STRUCT)
        {
            StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(qualifiedType.typeDeclaration);

            if (!structDeclaration->hasDefinition)
            {
                std::cerr << "Incomplete type " << iterator->value << std::endl;
                return nullptr;
            }
        }

        ++iterator;

        if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
        {
            std::cerr << "Expected an identifier" << std::endl;
            return nullptr;
        }

        std::string name = iterator->value;
        
        ++iterator;

        if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator) &&
            (checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator + 1) ||
             isDeclaration(tokens, iterator + 1, declarationScopes)))  // function declaration
        {
            ++iterator;

            FunctionDeclaration* result = new FunctionDeclaration();
            constructs.push_back(std::unique_ptr<Construct>(result));
            result->qualifiedType = qualifiedType;
            result->isStatic = isStatic;
            result->name = name;

            // TODO: check if only one definition exists

            std::vector<QualifiedType> parameters;

            if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
            {
                for (;;)
                {
                    ParameterDeclaration* parameterDeclaration;
                    if (!(parameterDeclaration = parseParameterDeclaration(tokens, iterator, declarationScopes)))
                    {
                        return nullptr;
                    }

                    result->parameterDeclarations.push_back(parameterDeclaration);
                    parameters.push_back(parameterDeclaration->qualifiedType);

                    if (!checkToken(Token::Type::COMMA, tokens, iterator))
                    {
                        break;
                    }

                    ++iterator;
                }
            }

            if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
            {
                std::cerr << "Expected a right parenthesis" << std::endl;
                return nullptr;
            }

            ++iterator;

            result->previousDeclaration = findFunctionDeclaration(name, declarationScopes, parameters);

            while (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
            {
                ++iterator;

                if (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
                {
                    ++iterator;

                    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
                    {
                        std::cerr << "Expected an identifier" << std::endl;
                        return nullptr;
                    }

                    std::string attribute = iterator->value;

                    ++iterator;

                    if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
                    {
                        ++iterator;

                        if (attribute != "program")
                        {
                            std::cerr << "Invalid attribute " << attribute << std::endl;
                        }

                        if (checkToken(Token::Type::LITERAL_INT, tokens, iterator))
                        {
                            ++iterator;
                        }
                        else if (checkToken(Token::Type::LITERAL_FLOAT, tokens, iterator))
                        {
                            ++iterator;
                        }
                        else if (checkToken(Token::Type::LITERAL_CHAR, tokens, iterator))
                        {
                            ++iterator;
                        }
                        else if (checkToken(Token::Type::LITERAL_STRING, tokens, iterator))
                        {
                            if (attribute == "program")
                            {
                                FunctionDeclaration::Program program = FunctionDeclaration::Program::NONE;

                                // TODO: find slot number
                                if (iterator->value == "fragment") program = FunctionDeclaration::Program::FRAGMENT;
                                else if (iterator->value == "vertex") program = FunctionDeclaration::Program::VERTEX;
                                else
                                {
                                    std::cerr << "Invalid program" << std::endl;
                                    return nullptr;
                                }

                                result->program = program;
                            }

                            ++iterator;
                        }

                        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
                        {
                            std::cerr << "Expected a right parenthesis" << std::endl;
                            return nullptr;
                        }

                        ++iterator;
                    }

                    if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
                    {
                        std::cerr << "Expected a right bracket" << std::endl;
                        return nullptr;
                    }

                    ++iterator;
                }
                else
                {
                    std::cerr << "Expected a right bracket" << std::endl;
                    return nullptr;
                }

                if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
                {
                    std::cerr << "Expected a right bracket" << std::endl;
                    return nullptr;
                }

                ++iterator;
            }

            declarationScopes.back().push_back(result);

            if (checkToken(Token::Type::LEFT_BRACE, tokens, iterator))
            {
                declarationScopes.push_back(std::vector<Declaration*>()); // add scope for parameters

                for (ParameterDeclaration* parameterDeclaration : result->parameterDeclarations)
                    declarationScopes.back().push_back(parameterDeclaration);

                // parse body
                if (!(result->body = parseCompoundStatement(tokens, iterator, declarationScopes)))
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
            if (findDeclaration(name, declarationScopes.back()))
            {
                std::cerr << "Redefinition of " << name << std::endl;
                return nullptr;
            }

            VariableDeclaration* result = new VariableDeclaration();
            constructs.push_back(std::unique_ptr<VariableDeclaration>(result));
            result->qualifiedType = qualifiedType;
            result->isStatic = isStatic;
            result->name = name;

            while (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
            {
                ++iterator;

                if (checkToken(Token::Type::LITERAL_INT, tokens, iterator))
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

                if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
                {
                    std::cerr << "Expected a right bracket" << std::endl;
                    return nullptr;
                }

                ++iterator;
            }

            if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
            {
                ++iterator;

                if (!(result->initialization = parseMultiplicationAssignment(tokens, iterator, declarationScopes)))
                {
                    return nullptr;
                }

                // TODO: check for comma and parse multiple expressions

                if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
                {
                    std::cerr << "Expected a right parenthesis" << std::endl;
                    return nullptr;
                }

                ++iterator;
            }
            else if (checkToken(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
            {
                ++iterator;

                if (!(result->initialization = parseMultiplicationAssignment(tokens, iterator, declarationScopes)))
                {
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
                                                      std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected an identifier" << std::endl;
        return nullptr;
    }

    StructDeclaration* result = new StructDeclaration();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->name = iterator->value;
    result->previousDeclaration = findStructDeclaration(iterator->value, declarationScopes);

    // TODO: check if only one definition exists

    ++iterator;

    if (checkToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        ++iterator;

        result->hasDefinition = true;

        for (;;)
        {
            if (checkToken(Token::Type::RIGHT_BRACE, tokens, iterator))
            {
                ++iterator;

                if (result->fieldDeclarations.empty())
                {
                    std::cerr << "Structure must have at least one member" << std::endl;
                    return nullptr;
                }

                declarationScopes.back().push_back(result);
                break;
            }
            else
            {
                FieldDeclaration* fieldDeclaration;
                if (!(fieldDeclaration = parseFieldDeclaration(tokens, iterator, declarationScopes)))
                {
                    return nullptr;
                }

                if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
                {
                    std::cerr << "Expected a semicolon" << std::endl;
                    return nullptr;
                }

                if (findFieldDeclaration(fieldDeclaration->name, result))
                {
                    std::cerr << "Redefinition of field " << fieldDeclaration->name << std::endl;
                    return nullptr;
                }

                ++iterator;

                fieldDeclaration->structTypeDeclaration = result;

                result->fieldDeclarations.push_back(fieldDeclaration);
            }
        }
    }

    declarationScopes.back().push_back(result);

    return result;
}

FieldDeclaration* ASTContext::parseFieldDeclaration(const std::vector<Token>& tokens,
                                                    std::vector<Token>::const_iterator& iterator,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes)
{
    FieldDeclaration* fieldDeclaration = new FieldDeclaration();
    constructs.push_back(std::unique_ptr<Construct>(fieldDeclaration));

    for (;;)
    {
        if (checkToken(Token::Type::KEYWORD_CONST, tokens, iterator))
        {
            ++iterator;
            fieldDeclaration->qualifiedType.isConst = true;
        }
        else if (checkToken(Token::Type::KEYWORD_STATIC, tokens, iterator))
        {
            ++iterator;
            fieldDeclaration->isStatic = true;
        }
        else break;
    }

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return nullptr;
    }

    fieldDeclaration->qualifiedType.typeDeclaration = findTypeDeclaration(iterator->value, declarationScopes);

    if (!fieldDeclaration->qualifiedType.typeDeclaration)
    {
        std::cerr << "Invalid type: " << iterator->value << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected an identifier" << std::endl;
        return nullptr;
    }

    fieldDeclaration->name = iterator->value;

    ++iterator;

    while (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
    {
        ++iterator;

        if (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
        {
            ++iterator;

            if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
            {
                std::cerr << "Expected an identifier" << std::endl;
                return nullptr;
            }

            std::string attribute = iterator->value;

            ++iterator;

            if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
            {
                ++iterator;

                if (attribute != "semantic")
                {
                    std::cerr << "Invalid attribute " << attribute << std::endl;
                }

                if (checkToken(Token::Type::LITERAL_INT, tokens, iterator))
                {
                    ++iterator;
                }
                else if (checkToken(Token::Type::LITERAL_FLOAT, tokens, iterator))
                {
                    ++iterator;
                }
                else if (checkToken(Token::Type::LITERAL_CHAR, tokens, iterator))
                {
                    ++iterator;
                }
                else if (checkToken(Token::Type::LITERAL_STRING, tokens, iterator))
                {
                    if (attribute == "semantic")
                    {
                        Semantic semantic = Semantic::NONE;

                        // TODO: find slot number
                        if (iterator->value == "binormal") semantic = Semantic::BINORMAL;
                        else if (iterator->value == "blend_indices") semantic = Semantic::BLEND_INDICES;
                        else if (iterator->value == "blend_weight") semantic = Semantic::BLEND_WEIGHT;
                        else if (iterator->value == "color") semantic = Semantic::COLOR;
                        else if (iterator->value == "normal") semantic = Semantic::NORMAL;
                        else if (iterator->value == "position") semantic = Semantic::POSITION;
                        else if (iterator->value == "position_transformed") semantic = Semantic::POSITION_TRANSFORMED;
                        else if (iterator->value == "point_size") semantic = Semantic::POINT_SIZE;
                        else if (iterator->value == "tangent") semantic = Semantic::TANGENT;
                        else if (iterator->value == "texture_coordinates") semantic = Semantic::TEXTURE_COORDINATES;
                        else
                        {
                            std::cerr << "Invalid semantic" << std::endl;
                            return nullptr;
                        }

                        fieldDeclaration->semantic = semantic;
                    }

                    ++iterator;
                }

                if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
                {
                    std::cerr << "Expected a right parenthesis" << std::endl;
                    return nullptr;
                }

                ++iterator;
            }

            if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
            {
                std::cerr << "Expected a right bracket" << std::endl;
                return nullptr;
            }

            ++iterator;
        }
        else if (checkToken(Token::Type::LITERAL_INT, tokens, iterator))
        {
            int size = std::stoi(iterator->value);

            ++iterator;

            if (size <= 0)
            {
                std::cerr << "Array size must be greater than zero" << std::endl;
                return nullptr;
            }

            fieldDeclaration->qualifiedType.dimensions.push_back(static_cast<uint32_t>(size));
        }
        else
        {
            std::cerr << "Expected an integer literal" << std::endl;
            return nullptr;
        }

        if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
        {
            std::cerr << "Expected a right bracket" << std::endl;
            return nullptr;
        }

        ++iterator;
    }

    return fieldDeclaration;
}

ParameterDeclaration* ASTContext::parseParameterDeclaration(const std::vector<Token>& tokens,
                                                            std::vector<Token>::const_iterator& iterator,
                                                            std::vector<std::vector<Declaration*>>& declarationScopes)
{
    ParameterDeclaration* parameterDeclaration = new ParameterDeclaration();
    constructs.push_back(std::unique_ptr<Construct>(parameterDeclaration));

    for (;;)
    {
        if (checkToken(Token::Type::KEYWORD_CONST, tokens, iterator))
        {
            ++iterator;
            parameterDeclaration->qualifiedType.isConst = true;
        }
        else break;
    }

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return nullptr;
    }

    parameterDeclaration->qualifiedType.typeDeclaration = findTypeDeclaration(iterator->value, declarationScopes);

    if (!parameterDeclaration->qualifiedType.typeDeclaration)
    {
        std::cerr << "Invalid type: " << iterator->value << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected an identifier" << std::endl;
        return nullptr;
    }

    parameterDeclaration->name = iterator->value;

    ++iterator;

    while (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
    {
        ++iterator;

        if (checkToken(Token::Type::LITERAL_INT, tokens, iterator))
        {
            int size = std::stoi(iterator->value);

            ++iterator;

            if (size <= 0)
            {
                std::cerr << "Array size must be greater than zero" << std::endl;
                return nullptr;
            }

            parameterDeclaration->qualifiedType.dimensions.push_back(static_cast<uint32_t>(size));
        }
        else
        {
            std::cerr << "Expected an integer literal" << std::endl;
            return nullptr;
        }

        if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
        {
            std::cerr << "Expected a right bracket" << std::endl;
            return nullptr;
        }
        
        ++iterator;
    }

    return parameterDeclaration;
}

/*TypeDefinitionDeclaration* ASTContext::parseTypeDefinitionDeclaration(const std::vector<Token>& tokens,
                                                                      std::vector<Token>::const_iterator& iterator,
                                                                      std::vector<std::vector<Declaration*>>& declarationScopes)
{
    std::cerr << "Typedef is not supported" << std::endl;
    return nullptr;

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return nullptr;
    }

    TypeDefinitionDeclaration* result = new TypeDefinitionDeclaration();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::DECLARATION;
    result->declarationKind = Declaration::Kind::TYPE;
    result->typeKind = TypeDeclaration::Kind::TYPE_DEFINITION;
    result->Declaration = findTypeDeclaration(iterator->value, declarationScopes);

    if (!result->type)
    {
        std::cerr << "Invalid type: " << iterator->value << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return nullptr;
    }

    result->name = iterator->value;

    ++iterator;

    if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        std::cerr << "Expected a semicolon" << std::endl;
        return nullptr;
    }

    ++iterator;

    return result;
}*/

Statement* ASTContext::parseStatement(const std::vector<Token>& tokens,
                                      std::vector<Token>::const_iterator& iterator,
                                      std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (checkToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        return parseCompoundStatement(tokens, iterator, declarationScopes);
    }
    else if (checkToken(Token::Type::KEYWORD_IF, tokens, iterator))
    {
        return parseIfStatement(tokens, iterator, declarationScopes);
    }
    else if (checkToken(Token::Type::KEYWORD_FOR, tokens, iterator))
    {
        return parseForStatement(tokens, iterator, declarationScopes);
    }
    else if (checkToken(Token::Type::KEYWORD_SWITCH, tokens, iterator))
    {
        return parseSwitchStatement(tokens, iterator, declarationScopes);
    }
    else if (checkToken(Token::Type::KEYWORD_CASE, tokens, iterator))
    {
        return parseCaseStatement(tokens, iterator, declarationScopes);
    }
    else if (checkToken(Token::Type::KEYWORD_WHILE, tokens, iterator))
    {
        return parseWhileStatement(tokens, iterator, declarationScopes);
    }
    else if (checkToken(Token::Type::KEYWORD_DO, tokens, iterator))
    {
        return parseDoStatement(tokens, iterator, declarationScopes);
    }
    else if (checkToken(Token::Type::KEYWORD_BREAK, tokens, iterator))
    {
        ++iterator;

        BreakStatement* result = new BreakStatement();
        constructs.push_back(std::unique_ptr<Construct>(result));

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;

        return result;
    }
    else if (checkToken(Token::Type::KEYWORD_CONTINUE, tokens, iterator))
    {
        ++iterator;

        ContinueStatement* result = new ContinueStatement();
        constructs.push_back(std::unique_ptr<Construct>(result));

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;

        return result;
    }
    else if (checkToken(Token::Type::KEYWORD_RETURN, tokens, iterator))
    {
        ++iterator;

        ReturnStatement* result = new ReturnStatement();
        constructs.push_back(std::unique_ptr<Construct>(result));

        if (!(result->result = parseComma(tokens, iterator, declarationScopes)))
        {
            std::cerr << "Expected an expression" << std::endl;
            return nullptr;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;

        return result;
    }
    else if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (declaration->getDeclarationKind() != Declaration::Kind::VARIABLE)
        {
            std::cerr << "Expected a variable declaration" << std::endl;
            return nullptr;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;

        DeclarationStatement* declarationStatement = new DeclarationStatement();
        constructs.push_back(std::unique_ptr<Construct>(declarationStatement));
        declarationStatement->declaration = declaration;

        return declarationStatement;
    }
    else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        ++iterator;

        Statement* statement = new Statement(Statement::Kind::EMPTY);
        constructs.push_back(std::unique_ptr<Construct>(statement));

        return statement;
    }
    else
    {
        ExpressionStatement* expressionStatement = new ExpressionStatement();
        constructs.push_back(std::unique_ptr<Construct>(expressionStatement));

        if (!(expressionStatement->expression = parseComma(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;

        return expressionStatement;
    }

    return nullptr;
}

CompoundStatement* ASTContext::parseCompoundStatement(const std::vector<Token>& tokens,
                                                      std::vector<Token>::const_iterator& iterator,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (!checkToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        std::cerr << "Expected a left brace" << std::endl;
        return nullptr;
    }

    ++iterator;

    declarationScopes.push_back(std::vector<Declaration*>());

    CompoundStatement* result = new CompoundStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));

    for (;;)
    {
        if (checkToken(Token::Type::RIGHT_BRACE, tokens, iterator))
        {
            ++iterator;
            break;
        }
        else
        {
            Statement* statement;
            if (!(statement = parseStatement(tokens, iterator, declarationScopes)))
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
                                          std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (!checkToken(Token::Type::KEYWORD_IF, tokens, iterator))
    {
        std::cerr << "Expected the if keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    IfStatement* result = new IfStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes)))
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
        if (!(result->condition = parseComma(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }
    }

    if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    Statement* statement;
    if (!(statement = parseStatement(tokens, iterator, declarationScopes)))
    {
        std::cerr << "Failed to parse the statement" << std::endl;
        return nullptr;
    }

    result->body = statement;

    if (checkToken(Token::Type::KEYWORD_ELSE, tokens, iterator))
    {
        ++iterator;

        if (!(statement = parseStatement(tokens, iterator, declarationScopes)))
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
                                            std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (!checkToken(Token::Type::KEYWORD_FOR, tokens, iterator))
    {
        std::cerr << "Expected the for keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    ForStatement* result = new ForStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes)))
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

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;
    }
    else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        ++iterator;

        result->initialization = nullptr;
    }
    else
    {
        if (!(result->initialization = parseComma(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;
    }

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes)))
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

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;
    }
    else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        ++iterator;

        result->condition = nullptr;
    }
    else
    {
        if (!(result->condition = parseComma(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        ++iterator;
    }

    if (checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        ++iterator;

        result->increment = nullptr;
    }
    else
    {
        if (!(result->increment = parseComma(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return nullptr;
        }

        ++iterator;
    }

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    return result;
}

SwitchStatement* ASTContext::parseSwitchStatement(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (!checkToken(Token::Type::KEYWORD_SWITCH, tokens, iterator))
    {
        std::cerr << "Expected the switch keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    SwitchStatement* result = new SwitchStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes)))
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
        if (!(result->condition = parseComma(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }
    }

    if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    return result;
}

CaseStatement* ASTContext::parseCaseStatement(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (!checkToken(Token::Type::KEYWORD_CASE, tokens, iterator))
    {
        std::cerr << "Expected the case keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    CaseStatement* result = new CaseStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));

    if (!(result->condition = parseComma(tokens, iterator, declarationScopes)))
    {
        std::cerr << "Expected an expression" << std::endl;
        return nullptr;
    }

    if (!checkToken(Token::Type::COLON, tokens, iterator))
    {
        std::cerr << "Expected a colon" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    return result;
}

WhileStatement* ASTContext::parseWhileStatement(const std::vector<Token>& tokens,
                                                std::vector<Token>::const_iterator& iterator,
                                                std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (!checkToken(Token::Type::KEYWORD_WHILE, tokens, iterator))
    {
        std::cerr << "Expected the while keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    WhileStatement* result = new WhileStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes)))
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
        if (!(result->condition = parseComma(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }
    }

    if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    return result;
}

DoStatement* ASTContext::parseDoStatement(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (!checkToken(Token::Type::KEYWORD_DO, tokens, iterator))
    {
        std::cerr << "Expected the do keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    DoStatement* result = new DoStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    if (!checkToken(Token::Type::KEYWORD_WHILE, tokens, iterator))
    {
        std::cerr << "Expected a \"while\" keyword" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    // expression
    if (!(result->condition = parseComma(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

    ++iterator;

    if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        std::cerr << "Expected a semicolon" << std::endl;
        return nullptr;
    }

    ++iterator;

    return result;
}

Expression* ASTContext::parsePrimary(const std::vector<Token>& tokens,
                                     std::vector<Token>::const_iterator& iterator,
                                     std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (checkToken(Token::Type::LITERAL_INT, tokens, iterator))
    {
        IntegerLiteralExpression* result = new IntegerLiteralExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->qualifiedType.typeDeclaration = &intType;
        result->value = strtoll(iterator->value.c_str(), nullptr, 0);

        ++iterator;

        return result;
    }
    else if (checkToken(Token::Type::LITERAL_FLOAT, tokens, iterator))
    {
        FloatingPointLiteralExpression* result = new FloatingPointLiteralExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->qualifiedType.typeDeclaration = &floatType;
        result->value = strtod(iterator->value.c_str(), nullptr);

        ++iterator;

        return result;
    }
    else if (checkToken(Token::Type::LITERAL_STRING, tokens, iterator))
    {
        StringLiteralExpression* result = new StringLiteralExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->qualifiedType.typeDeclaration = &stringType;
        result->value = iterator->value;

        ++iterator;

        return result;
    }
    else if (checkTokens({Token::Type::KEYWORD_TRUE, Token::Type::KEYWORD_FALSE}, tokens, iterator))
    {
        BooleanLiteralExpression* result = new BooleanLiteralExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->qualifiedType.typeDeclaration = &boolType;
        result->value = (iterator->type == Token::Type::KEYWORD_TRUE);

        ++iterator;

        return result;
    }
    else if (checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::string name = iterator->value;

        ++iterator;

        if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            ++iterator;

            CallExpression* result = new CallExpression();
            constructs.push_back(std::unique_ptr<Construct>(result));

            std::vector<QualifiedType> parameters;

            if (checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator)) // no arguments
            {
                ++iterator;
            }
            else
            {
                for (;;)
                {
                    Expression* parameter;

                    if (!(parameter = parseMultiplicationAssignment(tokens, iterator, declarationScopes)))
                    {
                        return nullptr;
                    }

                    result->parameters.push_back(parameter);
                    parameters.push_back(parameter->qualifiedType);

                    if (checkToken(Token::Type::COMMA, tokens, iterator))
                        ++iterator;
                    else
                        break;
                }

                if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
                {
                    std::cerr << "Expected a right parenthesis" << std::endl;
                    return nullptr;
                }

                ++iterator;
            }

            DeclarationReferenceExpression* declRefExpression = new DeclarationReferenceExpression();
            constructs.push_back(std::unique_ptr<Construct>(declRefExpression));

            FunctionDeclaration* functionDeclaration = findFunctionDeclaration(name, declarationScopes, parameters);

            if (!functionDeclaration)
            {
                std::cerr << "Invalid function reference: " << name << std::endl;
                return nullptr;
            }

            declRefExpression->declaration = functionDeclaration;
            declRefExpression->qualifiedType = functionDeclaration->qualifiedType;
            result->declarationReference = declRefExpression;
            result->qualifiedType = functionDeclaration->qualifiedType;

            return result;
        }
        else if (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
        {
            ++iterator;

            ArraySubscriptExpression* result = new ArraySubscriptExpression();
            constructs.push_back(std::unique_ptr<Construct>(result));

            DeclarationReferenceExpression* declRefExpression = new DeclarationReferenceExpression();
            constructs.push_back(std::unique_ptr<Construct>(declRefExpression));
            declRefExpression->declaration = findDeclaration(name, declarationScopes);

            if (!declRefExpression->declaration)
            {
                std::cerr << "Invalid declaration reference: " << name << std::endl;
                return nullptr;
            }

            if (declRefExpression->declaration->getDeclarationKind() != Declaration::Kind::VARIABLE &&
                declRefExpression->declaration->getDeclarationKind() != Declaration::Kind::PARAMETER)
            {
                std::cerr << "Expected a variable" << std::endl;
                return nullptr;
            }

            VariableDeclaration* variableDeclaration = static_cast<VariableDeclaration*>(declRefExpression->declaration);
            declRefExpression->qualifiedType = variableDeclaration->qualifiedType;

            result->declarationReference = declRefExpression;
            result->qualifiedType = declRefExpression->qualifiedType;

            if (!(result->expression = parseComma(tokens, iterator, declarationScopes)))
            {
                return nullptr;
            }

            if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
            {
                std::cerr << "Expected a right brace" << std::endl;
                return nullptr;
            }

            ++iterator;

            return result;
        }
        else
        {
            DeclarationReferenceExpression* result = new DeclarationReferenceExpression();
            constructs.push_back(std::unique_ptr<Construct>(result));
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
                    break;
                }
                case Declaration::Kind::VARIABLE:
                {
                    VariableDeclaration* variableDeclaration = static_cast<VariableDeclaration*>(result->declaration);
                    result->qualifiedType.typeDeclaration = variableDeclaration->qualifiedType.typeDeclaration;
                    result->qualifiedType.isConst = variableDeclaration->qualifiedType.isConst;
                    result->qualifiedType.dimensions = variableDeclaration->qualifiedType.dimensions;
                    break;
                }
                case Declaration::Kind::PARAMETER:
                {
                    ParameterDeclaration* parameterDeclaration = static_cast<ParameterDeclaration*>(result->declaration);
                    result->qualifiedType.typeDeclaration = parameterDeclaration->qualifiedType.typeDeclaration;
                    result->qualifiedType.isConst = parameterDeclaration->qualifiedType.isConst;
                    result->qualifiedType.dimensions = parameterDeclaration->qualifiedType.dimensions;
                    break;
                }
                default:
                    std::cerr << "Invalid declaration reference " << name;
                    return nullptr;
            }

            return result;
        }
    }
    else if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        ++iterator;

        ParenExpression* result = new ParenExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));

        if (!(result->expression = parseComma(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return nullptr;
        }

        ++iterator;

        result->qualifiedType.typeDeclaration = result->expression->qualifiedType.typeDeclaration;

        return result;
    }
    else
    {
        std::cerr << "Expected an expression" << std::endl;
        return nullptr;
    }
}

Expression* ASTContext::parseMember(const std::vector<Token>& tokens,
                                    std::vector<Token>::const_iterator& iterator,
                                    std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Expression* result;
    if (!(result = parsePrimary(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    while (checkToken(Token::Type::OPERATOR_DOT, tokens, iterator))
    {
        ++iterator;

        MemberExpression* expression = new MemberExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->expression = result;

        if (!result->qualifiedType.typeDeclaration)
        {
            std::cerr << "Expression has no result type" << std::endl;
            return nullptr;
        }

        if (result->qualifiedType.typeDeclaration->getTypeKind() != TypeDeclaration::Kind::STRUCT)
        {
            std::cerr << result->qualifiedType.typeDeclaration->name << " is not a structure" << std::endl;
            return nullptr;
        }

        StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->qualifiedType.typeDeclaration);

        if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
        {
            std::cerr << "Expected an identifier" << std::endl;
            return nullptr;
        }

        expression->fieldDeclaration = findFieldDeclaration(iterator->value, structDeclaration);

        if (!expression->fieldDeclaration)
        {
            std::cerr << "Structure " << structDeclaration->name <<  " has no member " << iterator->value << std::endl;
            return nullptr;
        }

        ++iterator;

        expression->qualifiedType.typeDeclaration = expression->fieldDeclaration->qualifiedType.typeDeclaration;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseSign(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (checkTokens({Token::Type::OPERATOR_PLUS, Token::Type::OPERATOR_MINUS}, tokens, iterator))
    {
        UnaryOperatorExpression* result = new UnaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));

        if (iterator->type == Token::Type::OPERATOR_PLUS) result->operatorKind = UnaryOperatorExpression::Kind::POSITIVE;
        else if (iterator->type == Token::Type::OPERATOR_MINUS) result->operatorKind = UnaryOperatorExpression::Kind::NEGATIVE;

        ++iterator;

        if (!(result->expression = parseMember(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        result->qualifiedType.typeDeclaration = result->expression->qualifiedType.typeDeclaration;

        return result;
    }
    else
    {
        Expression* result;
        if (!(result = parseMember(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        return result;
    }
}

Expression* ASTContext::parseNot(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (checkToken(Token::Type::OPERATOR_NOT, tokens, iterator))
    {
        UnaryOperatorExpression* result = new UnaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->operatorKind = UnaryOperatorExpression::Kind::NEGATION;

        ++iterator;

        if (!(result->expression = parseSign(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        result->qualifiedType.typeDeclaration = findTypeDeclaration("bool", declarationScopes);

        return result;
    }
    else
    {
        Expression* result;
        if (!(result = parseSign(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        return result;
    }
}

Expression* ASTContext::parseMultiplication(const std::vector<Token>& tokens,
                                            std::vector<Token>::const_iterator& iterator,
                                            std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Expression* result;
    if (!(result = parseNot(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_MULTIPLY, Token::Type::OPERATOR_DIVIDE}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));

        if (iterator->type == Token::Type::OPERATOR_MULTIPLY) expression->operatorKind = BinaryOperatorExpression::Kind::MULTIPLICATION;
        else if (iterator->type == Token::Type::OPERATOR_DIVIDE) expression->operatorKind = BinaryOperatorExpression::Kind::DIVISION;

        expression->leftExpression = result;

        ++iterator;

        if (!(expression->rightExpression = parseNot(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.typeDeclaration = expression->leftExpression->qualifiedType.typeDeclaration;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseAddition(const std::vector<Token>& tokens,
                                      std::vector<Token>::const_iterator& iterator,
                                      std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Expression* result;
    if (!(result = parseMultiplication(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_PLUS, Token::Type::OPERATOR_MINUS}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));

        if (iterator->type == Token::Type::OPERATOR_PLUS) expression->operatorKind = BinaryOperatorExpression::Kind::ADDITION;
        else if (iterator->type == Token::Type::OPERATOR_MINUS) expression->operatorKind = BinaryOperatorExpression::Kind::SUBTRACTION;

        expression->leftExpression = result;

        ++iterator;

        if (!(expression->rightExpression = parseMultiplication(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.typeDeclaration = expression->leftExpression->qualifiedType.typeDeclaration;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseLessThan(const std::vector<Token>& tokens,
                                      std::vector<Token>::const_iterator& iterator,
                                      std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Expression* result;
    if (!(result = parseAddition(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_LESS_THAN, Token::Type::OPERATOR_LESS_THAN_EQUAL}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));

        if (iterator->type == Token::Type::OPERATOR_LESS_THAN) expression->operatorKind = BinaryOperatorExpression::Kind::LESS_THAN;
        else if (iterator->type == Token::Type::OPERATOR_LESS_THAN_EQUAL) expression->operatorKind = BinaryOperatorExpression::Kind::LESS_THAN_EQUAL;

        expression->leftExpression = result;

        ++iterator;

        if (!(expression->rightExpression = parseAddition(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.typeDeclaration = expression->leftExpression->qualifiedType.typeDeclaration;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseGreaterThan(const std::vector<Token>& tokens,
                                         std::vector<Token>::const_iterator& iterator,
                                         std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Expression* result;
    if (!(result = parseLessThan(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_GREATER_THAN, Token::Type::OPERATOR_GREATER_THAN_EQUAL}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));

        if (iterator->type == Token::Type::OPERATOR_GREATER_THAN) expression->operatorKind = BinaryOperatorExpression::Kind::GREATER_THAN;
        else if (iterator->type == Token::Type::OPERATOR_GREATER_THAN_EQUAL) expression->operatorKind = BinaryOperatorExpression::Kind::GREATER_THAN_EQUAL;

        expression->leftExpression = result;

        ++iterator;

        if (!(expression->rightExpression = parseLessThan(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.typeDeclaration = expression->leftExpression->qualifiedType.typeDeclaration;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseEquality(const std::vector<Token>& tokens,
                                      std::vector<Token>::const_iterator& iterator,
                                      std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Expression* result;
    if (!(result = parseGreaterThan(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_EQUAL, Token::Type::OPERATOR_NOT_EQUAL}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));

        if (iterator->type == Token::Type::OPERATOR_EQUAL) expression->operatorKind = BinaryOperatorExpression::Kind::EQUALITY;
        else if (iterator->type == Token::Type::OPERATOR_NOT_EQUAL) expression->operatorKind = BinaryOperatorExpression::Kind::INEQUALITY;

        expression->leftExpression = result;

        ++iterator;

        if (!(expression->rightExpression = parseGreaterThan(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.typeDeclaration = expression->leftExpression->qualifiedType.typeDeclaration;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseTernary(const std::vector<Token>& tokens,
                                     std::vector<Token>::const_iterator& iterator,
                                     std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Expression* result;
    if (!(result = parseEquality(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    while (checkToken(Token::Type::OPERATOR_CONDITIONAL, tokens, iterator))
    {
        TernaryOperatorExpression* expression = new TernaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->condition = result;

        ++iterator;

        if (!(expression->leftExpression = parseTernary(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::COLON, tokens, iterator))
        {
            std::cerr << "Expected a colon" << std::endl;
            return nullptr;
        }

        ++iterator;

        if (!(expression->rightExpression = parseTernary(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.typeDeclaration = expression->leftExpression->qualifiedType.typeDeclaration;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseAssignment(const std::vector<Token>& tokens,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Expression* result;
    if (!(result = parseTernary(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    while (checkToken(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->operatorKind = BinaryOperatorExpression::Kind::ASSIGNMENT;
        expression->leftExpression = result;

        ++iterator;

        if (!(expression->rightExpression = parseTernary(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.typeDeclaration = expression->leftExpression->qualifiedType.typeDeclaration;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseAdditionAssignment(const std::vector<Token>& tokens,
                                                std::vector<Token>::const_iterator& iterator,
                                                std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Expression* result;
    if (!(result = parseAssignment(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_PLUS_ASSIGNMENT, Token::Type::OPERATOR_MINUS_ASSIGNMENT}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));

        if (iterator->type == Token::Type::OPERATOR_PLUS_ASSIGNMENT) expression->operatorKind = BinaryOperatorExpression::Kind::ADDITION_ASSIGNMENT;
        else if (iterator->type == Token::Type::OPERATOR_MINUS_ASSIGNMENT) expression->operatorKind = BinaryOperatorExpression::Kind::SUBTRACTION_ASSIGNMENT;

        expression->leftExpression = result;

        ++iterator;

        if (!(expression->rightExpression = parseAssignment(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.typeDeclaration = expression->leftExpression->qualifiedType.typeDeclaration;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseMultiplicationAssignment(const std::vector<Token>& tokens,
                                                      std::vector<Token>::const_iterator& iterator,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Expression* result;
    if (!(result = parseAdditionAssignment(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT, Token::Type::OPERATOR_DIVIDE_ASSIGNMENT}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));

        if (iterator->type == Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT) expression->operatorKind = BinaryOperatorExpression::Kind::MULTIPLICATION_ASSIGNMENT;
        else if (iterator->type == Token::Type::OPERATOR_DIVIDE_ASSIGNMENT) expression->operatorKind = BinaryOperatorExpression::Kind::DIVISION_ASSIGNMENT;

        expression->leftExpression = result;

        ++iterator;

        std::unique_ptr<Construct> right;
        if (!(expression->rightExpression = parseAdditionAssignment(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.typeDeclaration = expression->leftExpression->qualifiedType.typeDeclaration;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseComma(const std::vector<Token>& tokens,
                                   std::vector<Token>::const_iterator& iterator,
                                   std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Expression* result;
    if (!(result = parseMultiplicationAssignment(tokens, iterator, declarationScopes)))
    {
        return nullptr;
    }

    while (checkToken(Token::Type::COMMA, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->operatorKind = BinaryOperatorExpression::Kind::COMMA;
        expression->leftExpression = result;

        ++iterator;

        std::unique_ptr<Construct> right;
        if (!(expression->rightExpression = parseAdditionAssignment(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        expression->qualifiedType.typeDeclaration = expression->rightExpression->qualifiedType.typeDeclaration;

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

                    std::cout << std::endl;

                    for (const FieldDeclaration* fieldDeclaration : structDeclaration->fieldDeclarations)
                    {
                        dumpConstruct(fieldDeclaration, indent + " ");
                    }

                    break;
                }

                case TypeDeclaration::Kind::SIMPLE:
                {
                    const SimpleTypeDeclaration* simpleTypeDeclaration = static_cast<const SimpleTypeDeclaration*>(typeDeclaration);
                    std::cout << ", name: " << simpleTypeDeclaration->name << ", scalar: " << simpleTypeDeclaration->scalar << std::endl;
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

        case Declaration::Kind::FUNCTION:
        {
            const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(declaration);

            std::cout << ", name: " << functionDeclaration->name;

            if (functionDeclaration->qualifiedType.typeDeclaration)
            {
                std::cout << ", result type: " << functionDeclaration->qualifiedType.typeDeclaration->name;
            }

            if (functionDeclaration->program != FunctionDeclaration::Program::NONE)
            {
                std::cout << ", program: " << programToString(functionDeclaration->program);
            }

            if (functionDeclaration->previousDeclaration)
            {
                std::cout << ", previous declaration: " << functionDeclaration->previousDeclaration;
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
    std::cout << " " << expressionKindToString(expression->getExpressionKind());

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

            std::cout << std::endl;

            dumpConstruct(memberExpression->expression, indent + "  ");
            dumpConstruct(memberExpression->fieldDeclaration, indent + "  ");
            break;
        }

        case Expression::Kind::ARRAY_SUBSCRIPT:
        {
            const ArraySubscriptExpression* arraySubscriptExpression = static_cast<const ArraySubscriptExpression*>(expression);

            std::cout << std::endl;

            dumpConstruct(arraySubscriptExpression->declarationReference, indent + "  ");
            dumpConstruct(arraySubscriptExpression->expression, indent + "  ");
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

            std::cout <<", operator: " << binaryOperatorKindToString(binaryOperatorExpression->operatorKind) << std::endl;

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
    }
}

void ASTContext::dumpConstruct(const Construct* construct, std::string indent) const
{
    std::cout << indent << construct << " " << constructKindToString(construct->getKind());

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

void ASTContext::dumpDeclarationScopes(const std::vector<std::vector<Declaration*>>& declarationScopes) const
{
    std::string indent;

    for (auto& scope : declarationScopes)
    {
        for (auto& declaration : scope)
        {
            dumpDeclaration(declaration, indent);
        }

        indent += "  ";
    }
}
