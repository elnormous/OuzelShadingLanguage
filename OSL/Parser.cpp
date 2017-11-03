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
    std::unique_ptr<SimpleType> boolType(new SimpleType());
    boolType->kind = Construct::Kind::TYPE_SIMPLE;
    boolType->name = "bool";
    boolType->scalar = true;
    builtinTypes.push_back(std::move(boolType));

    std::unique_ptr<SimpleType> intType(new SimpleType());
    intType->kind = Construct::Kind::TYPE_SIMPLE;
    intType->name = "int";
    intType->scalar = true;
    builtinTypes.push_back(std::move(intType));

    std::unique_ptr<SimpleType> floatType(new SimpleType());
    floatType->kind = Construct::Kind::TYPE_SIMPLE;
    floatType->name = "float";
    floatType->scalar = true;
    builtinTypes.push_back(std::move(floatType));

    std::unique_ptr<StructType> vec2Type(new StructType());
    vec2Type->kind = Construct::Kind::TYPE_STRUCT;
    vec2Type->name = "vec2";
    builtinTypes.push_back(std::move(vec2Type));

    std::unique_ptr<StructType> vec3Type(new StructType());
    vec3Type->kind = Construct::Kind::TYPE_STRUCT;
    vec3Type->name = "vec3";
    builtinTypes.push_back(std::move(vec3Type));

    std::unique_ptr<StructType> vec4Type(new StructType());
    vec4Type->kind = Construct::Kind::TYPE_STRUCT;
    vec4Type->name = "vec4";
    builtinTypes.push_back(std::move(vec4Type));

    std::unique_ptr<StructType> stringType(new StructType());
    stringType->kind = Construct::Kind::TYPE_STRUCT;
    stringType->name = "string";
    builtinTypes.push_back(std::move(stringType));

    std::unique_ptr<StructType> samplerStateType(new StructType());
    samplerStateType->kind = Construct::Kind::TYPE_STRUCT;
    samplerStateType->name = "SamplerState";
    builtinTypes.push_back(std::move(samplerStateType));

    std::unique_ptr<StructType> texture2DType(new StructType());
    texture2DType->kind = Construct::Kind::TYPE_STRUCT;
    texture2DType->name = "Texture2D";
    builtinTypes.push_back(std::move(texture2DType));
}

bool ASTContext::parse(const std::vector<Token>& tokens)
{
    constructs.clear();

    auto iterator = tokens.cbegin();

    std::vector<std::vector<Declaration*>> declarations;
    declarations.push_back(std::vector<Declaration*>());

    translationUnit = parseTopLevel(tokens, iterator, declarations);

    return translationUnit != nullptr;
}

TranslationUnit* ASTContext::parseTopLevel(const std::vector<Token>& tokens,
                                           std::vector<Token>::const_iterator& iterator,
                                           std::vector<std::vector<Declaration*>>& declarations)
{
    TranslationUnit* result = new TranslationUnit();
    constructs.push_back(std::unique_ptr<Construct>(translationUnit));

    result->kind = Construct::Kind::TRANSLATION_UNIT;

    while (iterator != tokens.end())
    {
        if (checkToken(Token::Type::KEYWORD_STRUCT, tokens, iterator))
        {
            StructDeclaration* decl;
            if (!(decl = parseStructDeclaration(tokens, iterator, declarations)))
            {
                std::cerr << "Failed to parse a structure declaration" << std::endl;
                return nullptr;
            }

            result->declarations.push_back(decl);
        }
        /*else if (checkToken(Token::Type::KEYWORD_TYPEDEF, tokens, iterator))
        {
            TypeDefinitionDeclaration* decl;
            if (!(decl = parseTypeDefinitionDeclaration(tokens, iterator, declarations)))
            {
                std::cerr << "Failed to parse a type definition declaration" << std::endl;
                return nullptr;
            }

            result->declarations.push_back(decl);
        }*/
        else if (checkToken(Token::Type::KEYWORD_FUNCTION, tokens, iterator))
        {
            FunctionDeclaration* decl;
            if (!(decl = parseFunctionDeclaration(tokens, iterator, declarations)))
            {
                std::cerr << "Failed to parse a function declaration" << std::endl;
                return nullptr;
            }

            result->declarations.push_back(decl);
        }
        else if (checkTokens({Token::Type::KEYWORD_STATIC, Token::Type::KEYWORD_CONST, Token::Type::KEYWORD_VAR}, tokens, iterator))
        {
            VariableDeclaration* decl;
            if (!(decl = parseVariableDeclaration(tokens, iterator, declarations)))
            {
                std::cerr << "Failed to parse a variable declaration" << std::endl;
                return nullptr;
            }

            result->declarations.push_back(decl);
        }
        else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            Declaration* decl = new Declaration();
            constructs.push_back(std::unique_ptr<Construct>(decl));
            decl->kind = Construct::Kind::DECLARATION_EMPTY;
            result->declarations.push_back(decl);
        }
        else
        {
            std::cerr << "Expected a keyword" << std::endl;
            return nullptr;
        }
    }

    return result;
}

StructDeclaration* ASTContext::parseStructDeclaration(const std::vector<Token>& tokens,
                                                      std::vector<Token>::const_iterator& iterator,
                                                      std::vector<std::vector<Declaration*>>& declarations)
{
    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected an identifier" << std::endl;
        return nullptr;
    }

    StructDeclaration* result = new StructDeclaration();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::DECLARATION_STRUCT;

    StructType* type = new StructType();
    constructs.push_back(std::unique_ptr<Construct>(type));
    type->kind = Construct::Kind::TYPE_STRUCT;
    type->name = (iterator - 1)->value;
    type->declaration = result;
    result->type = type;

    if (checkToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        for (;;)
        {
            if (checkToken(Token::Type::RIGHT_BRACE, tokens, iterator))
            {
                if (result->fieldDeclarations.empty())
                {
                    std::cerr << "Structure must have at least one member" << std::endl;
                    return nullptr;
                }

                declarations.back().push_back(result);
                break;
            }
            else if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
            {
                FieldDeclaration* fieldDeclaration = new FieldDeclaration();
                constructs.push_back(std::unique_ptr<Construct>(fieldDeclaration));
                fieldDeclaration->kind = Construct::Kind::DECLARATION_FIELD;

                if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
                {
                    std::cerr << "Expected an identifier" << std::endl;
                    return nullptr;
                }

                Field* field = new Field();
                constructs.push_back(std::unique_ptr<Construct>(field));
                field->kind = Construct::Kind::FIELD;
                field->structType = type;
                field->name = (iterator - 1)->value;
                field->declaration = fieldDeclaration;

                if (!checkToken(Token::Type::COLON, tokens, iterator))
                {
                    std::cerr << "Expected a colon" << std::endl;
                    return nullptr;
                }

                if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
                {
                    std::cerr << "Expected a type name" << std::endl;
                    return nullptr;
                }

                field->qualifiedType.type = findType((iterator - 1)->value, declarations);

                if (!field->qualifiedType.type)
                {
                    std::cerr << "Invalid type: " << (iterator - 1)->value << std::endl;
                    return nullptr;
                }

                fieldDeclaration->field = field;

                if (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator)) // parse attributes
                {
                    bool firstAttribute = true;

                    for (;;)
                    {
                        if (checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
                        {
                            break;
                        }
                        else if ((firstAttribute || checkToken(Token::Type::COMMA, tokens, iterator)) &&
                                 checkToken(Token::Type::IDENTIFIER, tokens, iterator))
                        {
                            firstAttribute = false;

                            std::string attribute = (iterator - 1)->value;

                            if (!checkToken(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
                            {
                                std::cerr << "Expected an equality sign" << std::endl;
                                return nullptr;
                            }

                            if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
                            {
                                std::cerr << "Expected an identifier" << std::endl;
                                return nullptr;
                            }

                            if (attribute == "semantic")
                            {
                                Semantic semantic = Semantic::NONE;

                                // TODO: find slot number
                                if ((iterator - 1)->value == "binormal") semantic = Semantic::BINORMAL;
                                else if ((iterator - 1)->value == "blend_indices") semantic = Semantic::BLEND_INDICES;
                                else if ((iterator - 1)->value == "blend_weight") semantic = Semantic::BLEND_WEIGHT;
                                else if ((iterator - 1)->value == "color") semantic = Semantic::COLOR;
                                else if ((iterator - 1)->value == "normal") semantic = Semantic::NORMAL;
                                else if ((iterator - 1)->value == "position") semantic = Semantic::POSITION;
                                else if ((iterator - 1)->value == "position_transformed") semantic = Semantic::POSITION_TRANSFORMED;
                                else if ((iterator - 1)->value == "point_size") semantic = Semantic::POINT_SIZE;
                                else if ((iterator - 1)->value == "tangent") semantic = Semantic::TANGENT;
                                else if ((iterator - 1)->value == "texture_coordinates") semantic = Semantic::TEXTURE_COORDINATES;
                                else
                                {
                                    std::cerr << "Invalid semantic" << std::endl;
                                    return nullptr;
                                }

                                field->semantic = semantic;
                            }
                            else
                            {
                                std::cerr << "Invalid attribute" << std::endl;
                                return nullptr;
                            }
                        }
                        else
                        {
                            std::cerr << "Expected an identifier" << std::endl;
                            return nullptr;
                        }
                    }
                }

                if (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
                {
                    field->qualifiedType.isArray = true;

                    if (!checkToken(Token::Type::LITERAL_INT, tokens, iterator))
                    {
                        std::cerr << "Expected an integer literal" << std::endl;
                        return nullptr;
                    }

                    field->qualifiedType.arraySize = std::stoi((iterator - 1)->value);

                    if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
                    {
                        std::cerr << "Expected a right bracket" << std::endl;
                        return nullptr;
                    }
                }

                if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
                {
                    std::cerr << "Expected a semicolon" << std::endl;
                    return nullptr;
                }

                type->fields.push_back(field);
                result->fieldDeclarations.push_back(fieldDeclaration);
            }
            else
            {
                std::cerr << "Expected an attribute" << std::endl;
                return nullptr;
            }
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }
    }
    else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        declarations.back().push_back(result);
    }
    else
    {
        std::cerr << "Expected a left brace or a semicolon" << std::endl;
        return nullptr;
    }

    return result;
}

/*TypeDefinitionDeclaration* ASTContext::parseTypeDefinitionDeclaration(const std::vector<Token>& tokens,
                                                                      std::vector<Token>::const_iterator& iterator,
                                                                      std::vector<std::vector<Declaration*>>& declarations)
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
    result->kind = Construct::Kind::DECLARATION_TYPE_DEFINITION;
    result->type = findType((iterator - 1)->value, declarations);

    if (!result->type)
    {
        std::cerr << "Invalid type: " << (iterator - 1)->value << std::endl;
        return nullptr;
    }

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return nullptr;
    }

    result->name = (iterator - 1)->value;

    if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        std::cerr << "Expected a semicolon" << std::endl;
        return nullptr;
    }

    return result;
}*/

FunctionDeclaration* ASTContext::parseFunctionDeclaration(const std::vector<Token>& tokens,
                                                          std::vector<Token>::const_iterator& iterator,
                                                          std::vector<std::vector<Declaration*>>& declarations)
{
    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a function name" << std::endl;
        return nullptr;
    }

    FunctionDeclaration* result = new FunctionDeclaration();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::DECLARATION_FUNCTION;
    result->name = (iterator - 1)->value;

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Unexpected end of function declaration" << std::endl;
        return nullptr;
    }

    bool firstParameter = true;

    for (;;)
    {
        if (checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            break;
        }
        else if ((firstParameter || checkToken(Token::Type::COMMA, tokens, iterator)) &&
                 checkToken(Token::Type::IDENTIFIER, tokens, iterator))
        {
            firstParameter = false;

            ParameterDeclaration* parameter = new ParameterDeclaration();
            constructs.push_back(std::unique_ptr<Construct>(parameter));
            parameter->kind = Construct::Kind::DECLARATION_PARAMETER;
            parameter->name = (iterator - 1)->value;

            if (!checkToken(Token::Type::COLON, tokens, iterator))
            {
                std::cerr << "Expected a colon" << std::endl;
                return nullptr;
            }

            if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
            {
                std::cerr << "Expected a type name" << std::endl;
                return nullptr;
            }

            parameter->qualifiedType.type = findType((iterator - 1)->value, declarations);

            if (!parameter->qualifiedType.type)
            {
                std::cerr << "Invalid type: " << (iterator - 1)->value << std::endl;
                return nullptr;
            }

            if (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
            {
                parameter->qualifiedType.isArray = true;

                if (!checkToken(Token::Type::LITERAL_INT, tokens, iterator))
                {
                    std::cerr << "Expected an integer literal" << std::endl;
                    return nullptr;
                }

                parameter->qualifiedType.arraySize = std::stoi((iterator - 1)->value);

                if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
                {
                    std::cerr << "Expected a right bracket" << std::endl;
                    return nullptr;
                }
            }

            result->parameterDeclarations.push_back(parameter);
        }
        else
        {
            std::cerr << "Expected a comma, keyword or a right parenthesis" << std::endl;
            return nullptr;
        }
    }

    if (!checkToken(Token::Type::COLON, tokens, iterator))
    {
        std::cerr << "Expected a colon" << std::endl;
        return nullptr;
    }

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return nullptr;
    }

    result->qualifiedType.type = findType((iterator - 1)->value, declarations);

    if (!result->qualifiedType.type)
    {
        std::cerr << "Invalid type: " << (iterator - 1)->value << std::endl;
        return nullptr;
    }

    if (checkToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        declarations.back().push_back(result);

        // parse body
        if (!(result->body = parseCompoundStatement(tokens, iterator, declarations)))
        {
            std::cerr << "Failed to parse a compound statement" << std::endl;
            return nullptr;
        }
    }
    else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        declarations.back().push_back(result);
    }
    else
    {
        std::cerr << "Expected a left brace or a semicolon" << std::endl;
        return nullptr;
    }

    return result;
}

VariableDeclaration* ASTContext::parseVariableDeclaration(const std::vector<Token>& tokens,
                                                          std::vector<Token>::const_iterator& iterator,
                                                          std::vector<std::vector<Declaration*>>& declarations)
{
    VariableDeclaration* result = new VariableDeclaration();
    constructs.push_back(std::unique_ptr<VariableDeclaration>(result));
    result->kind = Construct::Kind::DECLARATION_VARIABLE;

    if ((iterator - 1)->type == Token::Type::KEYWORD_STATIC)
    {
        result->qualifiedType.isStatic = true;
    }
    else
    {
        --iterator;
    }

    if (checkToken(Token::Type::KEYWORD_CONST, tokens, iterator))
    {
        result->qualifiedType.isConst = true;
    }
    else if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
    {
        result->qualifiedType.isConst = false;
    }
    else
    {
        std::cerr << "Expected const or var" << std::endl;
        return nullptr;
    }

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Unexpected end of variable declaration" << std::endl;
        return nullptr;
    }

    result->name = (iterator - 1)->value;

    if (!checkToken(Token::Type::COLON, tokens, iterator))
    {
        std::cerr << "Expected a colon" << std::endl;
        return nullptr;
    }

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return nullptr;
    }

    result->qualifiedType.type = findType((iterator - 1)->value, declarations);

    if (!result->qualifiedType.type)
    {
        std::cerr << "Invalid type: " << (iterator - 1)->value << std::endl;
        return nullptr;
    }

    if (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
    {
        result->qualifiedType.isArray = true;

        if (!checkToken(Token::Type::LITERAL_INT, tokens, iterator))
        {
            std::cerr << "Expected an integer literal" << std::endl;
            return nullptr;
        }

        result->qualifiedType.arraySize = std::stoi((iterator - 1)->value);

        if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
        {
            std::cerr << "Expected a right bracket" << std::endl;
            return nullptr;
        }
    }

    if (checkToken(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
    {
        if (!(result->initialization = parseExpression(tokens, iterator, declarations)))
        {
            return nullptr;
        }
    }
    else if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        if (!(result->initialization = parseExpression(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return nullptr;
        }
    }

    declarations.back().push_back(result);

    return result;
}

Statement* ASTContext::parseStatement(const std::vector<Token>& tokens,
                                      std::vector<Token>::const_iterator& iterator,
                                      std::vector<std::vector<Declaration*>>& declarations)
{
    if (checkToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        return parseCompoundStatement(tokens, iterator, declarations);
    }
    else if (checkToken(Token::Type::KEYWORD_IF, tokens, iterator))
    {
        return parseIfStatement(tokens, iterator, declarations);
    }
    else if (checkToken(Token::Type::KEYWORD_FOR, tokens, iterator))
    {
        return parseForStatement(tokens, iterator, declarations);
    }
    else if (checkToken(Token::Type::KEYWORD_SWITCH, tokens, iterator))
    {
        return parseSwitchStatement(tokens, iterator, declarations);
    }
    else if (checkToken(Token::Type::KEYWORD_CASE, tokens, iterator))
    {
        return parseCaseStatement(tokens, iterator, declarations);
    }
    else if (checkToken(Token::Type::KEYWORD_WHILE, tokens, iterator))
    {
        return parseWhileStatement(tokens, iterator, declarations);
    }
    else if (checkToken(Token::Type::KEYWORD_DO, tokens, iterator))
    {
        return parseDoStatement(tokens, iterator, declarations);
    }
    else if (checkToken(Token::Type::KEYWORD_BREAK, tokens, iterator))
    {
        BreakStatement* result = new BreakStatement();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::STATEMENT_BREAK;

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        return result;
    }
    else if (checkToken(Token::Type::KEYWORD_CONTINUE, tokens, iterator))
    {
        ContinueStatement* result = new ContinueStatement();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::STATEMENT_CONTINUE;

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        return result;
    }
    else if (checkToken(Token::Type::KEYWORD_RETURN, tokens, iterator))
    {
        return parseReturnStatement(tokens, iterator, declarations);
    }
    else if (checkTokens({Token::Type::KEYWORD_STATIC, Token::Type::KEYWORD_CONST, Token::Type::KEYWORD_VAR}, tokens, iterator))
    {
        VariableDeclaration* declaration;
        if (!(declaration = parseVariableDeclaration(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        DeclarationStatement* declarationStatement = new DeclarationStatement();
        constructs.push_back(std::unique_ptr<Construct>(declarationStatement));
        declarationStatement->kind = Construct::Kind::STATEMENT_DECLARATION;
        declarationStatement->declaration = declaration;

        return declarationStatement;
    }
    else
    {
        Expression* expression;
        if (!(expression = parseExpression(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        return expression;
    }

    return nullptr;
}

CompoundStatement* ASTContext::parseCompoundStatement(const std::vector<Token>& tokens,
                                                      std::vector<Token>::const_iterator& iterator,
                                                      std::vector<std::vector<Declaration*>>& declarations)
{
    declarations.push_back(std::vector<Declaration*>());

    CompoundStatement* result = new CompoundStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_COMPOUND;

    for (;;)
    {
        if (checkToken(Token::Type::RIGHT_BRACE, tokens, iterator))
        {
            declarations.pop_back();
            break;
        }
        else
        {
            Statement* statement;
            if (!(statement = parseStatement(tokens, iterator, declarations)))
            {
                std::cerr << "Failed to parse a statement" << std::endl;
                return nullptr;
            }

            result->statements.push_back(statement);
        }
    }
    
    return result;
}

DeclarationStatement* ASTContext::parseVariableDeclStatement(const std::vector<Token>& tokens,
                                                             std::vector<Token>::const_iterator& iterator,
                                                             std::vector<std::vector<Declaration*>>& declarations)
{
    VariableDeclaration* declaration;
    if (!(declaration = parseVariableDeclaration(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    DeclarationStatement* result = new DeclarationStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_DECLARATION;
    result->declaration = declaration;

    return result;
}

IfStatement* ASTContext::parseIfStatement(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarations)
{
    IfStatement* result = new IfStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_IF;

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
    {
        if (!(result->condition = parseVariableDeclStatement(tokens, iterator, declarations)))
        {
            return nullptr;
        }
    }
    else
    {
        if (!(result->condition = parseExpression(tokens, iterator, declarations)))
        {
            return nullptr;
        }
    }

    if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

    Statement* statement;
    if (!(statement = parseStatement(tokens, iterator, declarations)))
    {
        std::cerr << "Failed to parse the statement" << std::endl;
        return nullptr;
    }

    result->body = statement;

    if (checkToken(Token::Type::KEYWORD_ELSE, tokens, iterator))
    {
        if (!(statement = parseStatement(tokens, iterator, declarations)))
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
                                            std::vector<std::vector<Declaration*>>& declarations)
{
    ForStatement* result = new ForStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_FOR;

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
    {
        if (!(result->initialization = parseVariableDeclStatement(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }
    }
    else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        Statement* emptyStatement = new Statement();
        constructs.push_back(std::unique_ptr<Construct>(emptyStatement));
        result->kind = Construct::Kind::STATEMENT_EMPTY;
        result->initialization = emptyStatement;
    }
    else
    {
        if (!(result->initialization = parseExpression(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }
    }

    if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
    {
        if (!(result->condition = parseVariableDeclStatement(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }
    }
    else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        Statement* emptyStatement = new Statement();
        constructs.push_back(std::unique_ptr<Construct>(emptyStatement));
        result->kind = Construct::Kind::STATEMENT_EMPTY;
        result->initialization = emptyStatement;
    }
    else
    {
        if (!(result->condition = parseExpression(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }
    }

    if (checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        Statement* emptyStatement = new Statement();
        constructs.push_back(std::unique_ptr<Construct>(emptyStatement));
        result->kind = Construct::Kind::STATEMENT_EMPTY;
        result->initialization = emptyStatement;
    }
    else
    {
        if (!(result->increment = parseExpression(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return nullptr;
        }
    }

    if (!(result->body = parseStatement(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    return result;
}

SwitchStatement* ASTContext::parseSwitchStatement(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarations)
{
    SwitchStatement* result = new SwitchStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_SWITCH;

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
    {
        if (!(result->condition = parseVariableDeclStatement(tokens, iterator, declarations)))
        {
            return nullptr;
        }
    }
    else
    {
        if (!(result->condition = parseExpression(tokens, iterator, declarations)))
        {
            return nullptr;
        }
    }

    if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

    if (!(result->body = parseStatement(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    return result;
}

CaseStatement* ASTContext::parseCaseStatement(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<Declaration*>>& declarations)
{
    CaseStatement* result = new CaseStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_CASE;

    if (!(result->condition = parseExpression(tokens, iterator, declarations)))
    {
        std::cerr << "Expected an expression" << std::endl;
        return nullptr;
    }

    if (!checkToken(Token::Type::COLON, tokens, iterator))
    {
        std::cerr << "Expected a colon" << std::endl;
        return nullptr;
    }

    if (!(result->body = parseStatement(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    return result;
}

WhileStatement* ASTContext::parseWhileStatement(const std::vector<Token>& tokens,
                                                std::vector<Token>::const_iterator& iterator,
                                                std::vector<std::vector<Declaration*>>& declarations)
{
    WhileStatement* result = new WhileStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_WHILE;

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
    {
        if (!(result->condition = parseVariableDeclStatement(tokens, iterator, declarations)))
        {
            return nullptr;
        }
    }
    else
    {
        if (!(result->condition = parseExpression(tokens, iterator, declarations)))
        {
            return nullptr;
        }
    }

    if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

    if (!(result->body = parseStatement(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    return result;
}

DoStatement* ASTContext::parseDoStatement(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarations)
{
    DoStatement* result = new DoStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_DO;

    if (!(result->body = parseStatement(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    if (!checkToken(Token::Type::KEYWORD_WHILE, tokens, iterator))
    {
        std::cerr << "Expected a \"while\" keyword" << std::endl;
        return nullptr;
    }

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    // expression
    if (!(result->condition = parseExpression(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

    if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        std::cerr << "Expected a semicolon" << std::endl;
        return nullptr;
    }

    return result;
}

ReturnStatement* ASTContext::parseReturnStatement(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<Declaration*>>& declarations)
{
    ReturnStatement* result = new ReturnStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_RETURN;

    if (!(result->result = parseExpression(tokens, iterator, declarations)))
    {
        std::cerr << "Expected an expression" << std::endl;
        return nullptr;
    }

    if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        std::cerr << "Expected a semicolon" << std::endl;
        return nullptr;
    }

    return result;
}

Expression* ASTContext::parseExpression(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::vector<Declaration*>>& declarations)
{
    return parseMultiplicationAssignment(tokens, iterator, declarations);
}

Expression* ASTContext::parseMultiplicationAssignment(const std::vector<Token>& tokens,
                                                      std::vector<Token>::const_iterator& iterator,
                                                      std::vector<std::vector<Declaration*>>& declarations)
{
    Expression* result;
    if (!(result = parseAdditionAssignment(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT, Token::Type::OPERATOR_DIVIDE_ASSIGNMENT}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        std::unique_ptr<Construct> right;
        if (!(expression->rightExpression = parseAdditionAssignment(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;

        result = expression;
    }

    return result;
}

Expression* ASTContext::parseAdditionAssignment(const std::vector<Token>& tokens,
                                                std::vector<Token>::const_iterator& iterator,
                                                std::vector<std::vector<Declaration*>>& declarations)
{
    Expression* result;
    if (!(result = parseAssignment(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_PLUS_ASSIGNMENT, Token::Type::OPERATOR_MINUS_ASSIGNMENT}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseAssignment(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;

        result = expression;
    }

    return result;
}

Expression* ASTContext::parseAssignment(const std::vector<Token>& tokens,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<std::vector<Declaration*>>& declarations)
{
    Expression* result;
    if (!(result = parseTernary(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    while (checkToken(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseTernary(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;

        result = expression;
    }

    return result;
}

Expression* ASTContext::parseTernary(const std::vector<Token>& tokens,
                                     std::vector<Token>::const_iterator& iterator,
                                     std::vector<std::vector<Declaration*>>& declarations)
{
    Expression* result;
    if (!(result = parseEquality(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    while (checkToken(Token::Type::OPERATOR_CONDITIONAL, tokens, iterator))
    {
        TernaryOperatorExpression* expression = new TernaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->kind = Construct::Kind::OPERATOR_TERNARY;
        expression->value = (iterator - 1)->value;
        expression->condition = result;

        if (!(expression->leftExpression = parseTernary(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::COLON, tokens, iterator))
        {
            std::cerr << "Expected a colon" << std::endl;
            return nullptr;
        }

        if (!(expression->rightExpression = parseTernary(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;

        result = expression;
    }

    return result;
}

Expression* ASTContext::parseEquality(const std::vector<Token>& tokens,
                                      std::vector<Token>::const_iterator& iterator,
                                      std::vector<std::vector<Declaration*>>& declarations)
{
    Expression* result;
    if (!(result = parseGreaterThan(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_EQUAL, Token::Type::OPERATOR_NOT_EQUAL}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseGreaterThan(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseGreaterThan(const std::vector<Token>& tokens,
                                         std::vector<Token>::const_iterator& iterator,
                                         std::vector<std::vector<Declaration*>>& declarations)
{
    Expression* result;
    if (!(result = parseLessThan(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_GREATER_THAN, Token::Type::OPERATOR_GREATER_THAN_EQUAL}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseLessThan(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseLessThan(const std::vector<Token>& tokens,
                                      std::vector<Token>::const_iterator& iterator,
                                      std::vector<std::vector<Declaration*>>& declarations)
{
    Expression* result;
    if (!(result = parseAddition(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_LESS_THAN, Token::Type::OPERATOR_LESS_THAN_EQUAL}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseAddition(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseAddition(const std::vector<Token>& tokens,
                                      std::vector<Token>::const_iterator& iterator,
                                      std::vector<std::vector<Declaration*>>& declarations)
{
    Expression* result;
    if (!(result = parseMultiplication(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_PLUS, Token::Type::OPERATOR_MINUS}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseMultiplication(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseMultiplication(const std::vector<Token>& tokens,
                                            std::vector<Token>::const_iterator& iterator,
                                            std::vector<std::vector<Declaration*>>& declarations)
{
    Expression* result;
    if (!(result = parseNot(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_MULTIPLY, Token::Type::OPERATOR_DIVIDE}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseNot(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        // TODO: fix this
        expression->qualifiedType.type = expression->leftExpression->qualifiedType.type;

        result = expression;
    }

    return result;
}

Expression* ASTContext::parseNot(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::vector<Declaration*>>& declarations)
{
    if (checkToken(Token::Type::OPERATOR_NOT, tokens, iterator))
    {
        UnaryOperatorExpression* result = new UnaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::OPERATOR_UNARY;
        result->value = (iterator - 1)->value;

        if (!(result->expression = parseSign(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        result->qualifiedType.type = findType("bool", declarations);

        return result;
    }
    else
    {
        Expression* result;
        if (!(result = parseSign(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        return result;
    }
}

Expression* ASTContext::parseSign(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<Declaration*>>& declarations)
{
    if (checkTokens({Token::Type::OPERATOR_PLUS, Token::Type::OPERATOR_MINUS}, tokens, iterator))
    {
        UnaryOperatorExpression* result = new UnaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::OPERATOR_UNARY;
        result->value = (iterator - 1)->value;

        if (!(result->expression = parseMember(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        result->qualifiedType.type = result->expression->qualifiedType.type;

        return result;
    }
    else
    {
        Expression* result;
        if (!(result = parseMember(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        return result;
    }
}

Expression* ASTContext::parseMember(const std::vector<Token>& tokens,
                                    std::vector<Token>::const_iterator& iterator,
                                    std::vector<std::vector<Declaration*>>& declarations)
{
    Expression* result;
    if (!(result = parsePrimary(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    while (checkToken(Token::Type::OPERATOR_DOT, tokens, iterator))
    {
        MemberExpression* expression = new MemberExpression();
        constructs.push_back(std::unique_ptr<Construct>(expression));
        expression->kind = Construct::Kind::EXPRESSION_MEMBER;
        expression->expression = result;

        if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
        {
            std::cerr << "Expected an identifier" << std::endl;
            return nullptr;
        }

        if (!result->qualifiedType.type)
        {
            std::cerr << "Expression has no result type" << std::endl;
            return nullptr;
        }

        if (result->qualifiedType.type->kind != Construct::Kind::TYPE_STRUCT)
        {
            std::cerr << result->qualifiedType.type->name << " is not a structure" << std::endl;
            return nullptr;
        }

        StructType* structType = static_cast<StructType*>(result->qualifiedType.type);

        expression->field = findField((iterator - 1)->value, structType);

        if (!expression->field)
        {
            std::cerr << "Structure " << structType->name <<  " has no member " << (iterator - 1)->value << std::endl;
            return nullptr;
        }

        expression->qualifiedType.type = expression->field->qualifiedType.type;

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parsePrimary(const std::vector<Token>& tokens,
                                     std::vector<Token>::const_iterator& iterator,
                                     std::vector<std::vector<Declaration*>>& declarations)
{
    if (checkToken(Token::Type::LITERAL_INT, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION_LITERAL;
        result->qualifiedType.type = findType("int", declarations);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkToken(Token::Type::LITERAL_FLOAT, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION_LITERAL;
        result->qualifiedType.type = findType("float", declarations);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkToken(Token::Type::LITERAL_STRING, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION_LITERAL;
        result->qualifiedType.type = findType("string", declarations);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkTokens({Token::Type::KEYWORD_TRUE, Token::Type::KEYWORD_FALSE}, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION_LITERAL;
        result->qualifiedType.type = findType("bool", declarations);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::string name = (iterator - 1)->value;

        if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            CallExpression* result = new CallExpression();
            constructs.push_back(std::unique_ptr<Construct>(result));
            result->kind = Construct::Kind::EXPRESSION_CALL;

            DeclarationReferenceExpression* declRefExpression = new DeclarationReferenceExpression();
            constructs.push_back(std::unique_ptr<Construct>(declRefExpression));
            declRefExpression->kind = Construct::Kind::EXPRESSION_DECLARATION_REFERENCE;
            declRefExpression->declaration = findDeclaration(name, declarations);

            if (!declRefExpression->declaration)
            {
                std::cerr << "Invalid declaration reference: " << name << std::endl;
                return nullptr;
            }

            result->declarationReference = declRefExpression;

            if (declRefExpression->declaration->kind != Construct::Kind::DECLARATION_FUNCTION)
            {
                std::cerr << name << " is not a function" << std::endl;
                return nullptr;
            }

            FunctionDeclaration* functionDeclaration = static_cast<FunctionDeclaration*>(declRefExpression->declaration);
            declRefExpression->qualifiedType.type = functionDeclaration->qualifiedType.type;
            result->qualifiedType.type = functionDeclaration->qualifiedType.type;

            bool firstParameter = true;
            std::unique_ptr<Construct> parameter;

            for (;;)
            {
                Expression* parameter;
                if (checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
                {
                    break;
                }
                else if ((firstParameter || checkToken(Token::Type::COMMA, tokens, iterator)) &&
                         (parameter = parseExpression(tokens, iterator, declarations)))
                {
                    firstParameter = false;
                    result->parameters.push_back(parameter);
                }
                else
                {
                    std::cerr << "Expected a comma, keyword or a right parenthesis" << std::endl;
                    return nullptr;
                }
            }

            return result;
        }
        else if (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
        {
            ArraySubscriptExpression* result = new ArraySubscriptExpression();
            constructs.push_back(std::unique_ptr<Construct>(result));
            result->kind = Construct::Kind::EXPRESSION_ARRAY_SUBSCRIPT;

            DeclarationReferenceExpression* declRefExpression = new DeclarationReferenceExpression();
            constructs.push_back(std::unique_ptr<Construct>(declRefExpression));
            declRefExpression->kind = Construct::Kind::EXPRESSION_DECLARATION_REFERENCE;
            declRefExpression->declaration = findDeclaration(name, declarations);

            if (!declRefExpression->declaration)
            {
                std::cerr << "Invalid declaration reference: " << name << std::endl;
                return nullptr;
            }

            if (declRefExpression->declaration->kind != Construct::Kind::DECLARATION_VARIABLE)
            {
                std::cerr << "Expected a variable" << std::endl;
                return nullptr;
            }

            VariableDeclaration* variableDeclaration = static_cast<VariableDeclaration*>(declRefExpression->declaration);
            declRefExpression->qualifiedType = variableDeclaration->qualifiedType;

            result->declarationReference = declRefExpression;
            result->qualifiedType = declRefExpression->qualifiedType;

            if (!(result->expression = parseExpression(tokens, iterator, declarations)))
            {
                return nullptr;
            }

            if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
            {
                std::cerr << "Expected a right brace" << std::endl;
                return nullptr;
            }

            return result;
        }
        else
        {
            DeclarationReferenceExpression* result = new DeclarationReferenceExpression();
            constructs.push_back(std::unique_ptr<Construct>(result));
            result->kind = Construct::Kind::EXPRESSION_DECLARATION_REFERENCE;
            result->declaration = findDeclaration((iterator - 1)->value, declarations);

            if (!result->declaration)
            {
                std::cerr << "Invalid declaration reference: " << (iterator - 1)->value << std::endl;
                return nullptr;
            }

            switch (result->declaration->kind)
            {
                case Construct::Kind::DECLARATION_STRUCT:
                {
                    StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->declaration);
                    result->qualifiedType.type = structDeclaration->type;
                    break;
                }
                case Construct::Kind::DECLARATION_VARIABLE:
                {
                    VariableDeclaration* variableDeclaration = static_cast<VariableDeclaration*>(result->declaration);
                    result->qualifiedType.type = variableDeclaration->qualifiedType.type;
                    break;
                }
                default:
                    break;
            }

            return result;
        }
    }
    else if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        ParenExpression* result = new ParenExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION_PAREN;

        if (!(result->expression = parseExpression(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        result->qualifiedType.type = result->expression->qualifiedType.type;

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return nullptr;
        }

        return result;
    }
    else
    {
        std::cerr << "Expected an expression" << std::endl;
        return nullptr;
    }
}

void ASTContext::dump()
{
    if (translationUnit)
    {
        ASTContext::dumpNode(translationUnit);
    }
}

void ASTContext::dumpNode(const Construct* node, std::string indent)
{
    std::cout << indent << node << " " << nodeKindToString(node->kind);

    switch (node->kind)
    {
        case Construct::Kind::NONE:
        {
            std::cout << std::endl;
            break;
        }

        case Construct::Kind::TYPE_SIMPLE:
        {
            const SimpleType* simpleType = static_cast<const SimpleType*>(node);
            std::cout << ", name" << simpleType->name << ", scalar: " << simpleType->scalar << std::endl;
            break;
        }

        case Construct::Kind::TYPE_STRUCT:
        {
            const StructType* structType = static_cast<const StructType*>(node);
            std::cout << ", name" << structType->name << std::endl;
            break;
        }

        case Construct::Kind::FIELD:
        {
            const Field* field = static_cast<const Field*>(node);
            std::cout << ", name" << field->name << ", type: " << field->qualifiedType.type->name << std::endl;
            break;
        }

        case Construct::Kind::TRANSLATION_UNIT:
        {
            const TranslationUnit* translationUnit = static_cast<const TranslationUnit*>(node);

            std::cout << std::endl;

            for (Declaration* declaration : translationUnit->declarations)
            {
                dumpNode(declaration, indent + "  ");
            }
            break;
        }

        case Construct::Kind::DECLARATION_EMPTY:
        {
            std::cout << std::endl;
            break;
        }

        case Construct::Kind::DECLARATION_STRUCT:
        {
            const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(node);

            std::cout << ", name: " << structDeclaration->type->name << std::endl;

            for (const FieldDeclaration* fieldDeclaration : structDeclaration->fieldDeclarations)
            {
                dumpNode(fieldDeclaration, indent + "  ");
            }

            break;
        }

        case Construct::Kind::DECLARATION_FIELD:
        {
            const FieldDeclaration* fieldDeclaration = static_cast<const FieldDeclaration*>(node);

            std::cout << ", name: " << fieldDeclaration->field->name << std::endl;

            dumpNode(fieldDeclaration->field, indent + "  ");
            break;
        }

        case Construct::Kind::DECLARATION_FUNCTION:
        {
            const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(node);

            std::cout << ", name: " << functionDeclaration->name;

            if (functionDeclaration->qualifiedType.type)
            {
                std::cout << ", result type: " << functionDeclaration->qualifiedType.type->name;
            }

            std::cout << std::endl;

            for (ParameterDeclaration* parameter : functionDeclaration->parameterDeclarations)
            {
                dumpNode(parameter, indent + "  ");
            }

            if (functionDeclaration->body)
            {
                dumpNode(functionDeclaration->body, indent + "  ");
            }
            break;
        }

        case Construct::Kind::DECLARATION_VARIABLE:
        {
            const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(node);
            std::cout << ", name: " << variableDeclaration->name << ", type: " << variableDeclaration->qualifiedType.type->name << std::endl;
            break;
        }

        case Construct::Kind::DECLARATION_PARAMETER:
        {
            const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(node);
            std::cout << ", name: " << parameterDeclaration->name << ", type: " << parameterDeclaration->qualifiedType.type->name << std::endl;
            break;
        }

        case Construct::Kind::EXPRESSION_CALL:
        {
            const CallExpression* callExpression = static_cast<const CallExpression*>(node);

            std::cout << std::endl;

            dumpNode(callExpression->declarationReference, indent + "  ");

            for (Expression* parameter : callExpression->parameters)
            {
                dumpNode(parameter, indent + "  ");
            }
            
            break;
        }

        case Construct::Kind::EXPRESSION_LITERAL:
        {
            const Expression* expression = static_cast<const Expression*>(node);
            std::cout << ", value: " << expression->value << ", type: " << expression->qualifiedType.type->name << std::endl;
            break;
        }

        case Construct::Kind::EXPRESSION_DECLARATION_REFERENCE:
        {
            const DeclarationReferenceExpression* declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(node);

            std::cout << std::endl;

            dumpNode(declarationReferenceExpression->declaration, indent + "  ");
            break;
        }

        case Construct::Kind::EXPRESSION_PAREN:
        {
            const ParenExpression* parenExpression = static_cast<const ParenExpression*>(node);

            std::cout << std::endl;

            dumpNode(parenExpression->expression, indent + "  ");
            break;
        }

        case Construct::Kind::EXPRESSION_MEMBER:
        {
            const MemberExpression* memberExpression = static_cast<const MemberExpression*>(node);

            std::cout << std::endl;

            dumpNode(memberExpression->expression, indent + "  ");
            dumpNode(memberExpression->field, indent + "  ");
            break;
        }

        case Construct::Kind::EXPRESSION_ARRAY_SUBSCRIPT:
        {
            const ArraySubscriptExpression* arraySubscriptExpression = static_cast<const ArraySubscriptExpression*>(node);

            std::cout << std::endl;

            dumpNode(arraySubscriptExpression->declarationReference, indent + "  ");
            dumpNode(arraySubscriptExpression->expression, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_EMPTY:
        {
            std::cout << std::endl;
            break;
        }

        case Construct::Kind::STATEMENT_DECLARATION:
        {
            const DeclarationStatement* declarationStatement = static_cast<const DeclarationStatement*>(node);

            std::cout << std::endl;

            dumpNode(declarationStatement->declaration, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_COMPOUND:
        {
            const CompoundStatement* compoundStatement = static_cast<const CompoundStatement*>(node);

            std::cout << std::endl;

            for (Statement* statement : compoundStatement->statements)
            {
                dumpNode(statement, indent + "  ");
            }
            break;
        }

        case Construct::Kind::STATEMENT_IF:
        {
            const IfStatement* ifStatement = static_cast<const IfStatement*>(node);

            std::cout << std::endl;

            dumpNode(ifStatement->condition, indent + "  ");
            dumpNode(ifStatement->body, indent + "  ");
            if (ifStatement->elseBody) dumpNode(ifStatement->elseBody, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_FOR:
        {
            const ForStatement* forStatement = static_cast<const ForStatement*>(node);

            std::cout << std::endl;

            dumpNode(forStatement->initialization, indent + "  ");
            dumpNode(forStatement->condition, indent + "  ");
            dumpNode(forStatement->increment, indent + "  ");
            dumpNode(forStatement->body, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_SWITCH:
        {
            const SwitchStatement* switchStatement = static_cast<const SwitchStatement*>(node);

            std::cout << std::endl;

            dumpNode(switchStatement->condition, indent + "  ");
            dumpNode(switchStatement->body, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_CASE:
        {
            const CaseStatement* caseStatement = static_cast<const CaseStatement*>(node);

            std::cout << std::endl;

            dumpNode(caseStatement->condition, indent + "  ");
            dumpNode(caseStatement->body, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_WHILE:
        {
            const WhileStatement* whileStatement = static_cast<const WhileStatement*>(node);

            std::cout << std::endl;

            dumpNode(whileStatement->condition, indent + "  ");
            dumpNode(whileStatement->body, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_DO:
        {
            const DoStatement* doStatement = static_cast<const DoStatement*>(node);

            std::cout << std::endl;

            dumpNode(doStatement->body, indent + "  ");
            dumpNode(doStatement->condition, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_BREAK:
        {
            std::cout << std::endl;
            break;
        }

        case Construct::Kind::STATEMENT_CONTINUE:
        {
            std::cout << std::endl;
            break;
        }

        case Construct::Kind::STATEMENT_RETURN:
        {
            const ReturnStatement* returnStatement = static_cast<const ReturnStatement*>(node);

            std::cout << std::endl;

            if (returnStatement->result)
            {
                dumpNode(returnStatement->result, indent + "  ");
            }
            break;
        }

        case Construct::Kind::OPERATOR_UNARY:
        {
            const UnaryOperatorExpression* unaryOperatorExpression = static_cast<const UnaryOperatorExpression*>(node);

            std::cout << std::endl;

            dumpNode(unaryOperatorExpression->expression, indent + "  ");
            break;
        }
            
        case Construct::Kind::OPERATOR_BINARY:
        {
            const BinaryOperatorExpression* binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(node);
            
            std::cout << std::endl;

            dumpNode(binaryOperatorExpression->leftExpression, indent + "  ");
            dumpNode(binaryOperatorExpression->rightExpression, indent + "  ");
            break;
        }
            
        case Construct::Kind::OPERATOR_TERNARY:
        {
            const TernaryOperatorExpression* ternaryOperatorExpression = static_cast<const TernaryOperatorExpression*>(node);
            
            std::cout << std::endl;

            dumpNode(ternaryOperatorExpression->condition, indent + "  ");
            dumpNode(ternaryOperatorExpression->leftExpression, indent + "  ");
            dumpNode(ternaryOperatorExpression->rightExpression, indent + "  ");
            break;
        }
    }
}
