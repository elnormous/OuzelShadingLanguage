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
}

bool ASTContext::parse(const std::vector<Token>& tokens)
{
    constructs.clear();
    types.clear();
    fields.clear();
    declarations.clear();

    std::unique_ptr<SimpleType> boolType(new SimpleType());
    boolType->name = "bool";
    boolType->scalar = true;
    types.push_back(std::move(boolType));

    std::unique_ptr<SimpleType> intType(new SimpleType());
    intType->name = "int";
    intType->scalar = true;
    types.push_back(std::move(intType));

    std::unique_ptr<SimpleType> floatType(new SimpleType());
    floatType->name = "float";
    floatType->scalar = true;
    types.push_back(std::move(floatType));

    std::unique_ptr<StructType> vec2Type(new StructType());
    vec2Type->name = "vec2";
    vec2Type->record = true;
    types.push_back(std::move(vec2Type));

    std::unique_ptr<StructType> vec3Type(new StructType());
    vec3Type->name = "vec3";
    vec3Type->record = true;
    types.push_back(std::move(vec3Type));

    std::unique_ptr<StructType> vec4Type(new StructType());
    vec4Type->name = "vec4";
    vec4Type->record = true;
    types.push_back(std::move(vec4Type));

    std::unique_ptr<StructType> stringType(new StructType());
    stringType->record = true;
    stringType->name = "string";
    types.push_back(std::move(stringType));

    std::unique_ptr<StructType> samplerStateType(new StructType());
    samplerStateType->record = true;
    samplerStateType->name = "SamplerState";
    types.push_back(std::move(samplerStateType));

    std::unique_ptr<StructType> texture2DType(new StructType());
    texture2DType->record = true;
    texture2DType->name = "Texture2D";
    types.push_back(std::move(texture2DType));

    auto iterator = tokens.cbegin();

    std::vector<std::vector<Declaration*>> declarationScopes;
    declarationScopes.push_back(std::vector<Declaration*>());

    while (iterator != tokens.end())
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes)))
        {
            return false;
        }

        declarations.push_back(declaration);
    }

    return true;
}

bool ASTContext::isDeclaration(const std::vector<Token>& tokens,
                               std::vector<Token>::const_iterator& iterator,
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
        Type* type = findType(iterator->value, declarationScopes);

        if (type)
        {
            return true;
        }
    }

    return false;
}

Declaration* ASTContext::parseDeclaration(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (checkToken(Token::Type::KEYWORD_STRUCT, tokens, iterator))
    {
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
        TypeDefinitionDeclaration* declaration;
        if (!(declaration = parseTypeDefinitionDeclaration(tokens, iterator, declarationScopes)))
        {
            std::cerr << "Failed to parse a type definition declaration" << std::endl;
            return nullptr;
        }

        return declaration;
    }*/
    else if (isDeclaration(tokens, iterator, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(tokens, iterator, declarationScopes)))
        {
            std::cerr << "Failed to parse a function declaration" << std::endl;
            return nullptr;
        }

        return declaration;
    }
    else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        Declaration* declaration = new Declaration();
        constructs.push_back(std::unique_ptr<Construct>(declaration));
        declaration->kind = Construct::Kind::DECLARATION_EMPTY;

        return declaration;
    }
    else
    {
        std::cerr << "Expected a keyword" << std::endl;
        return nullptr;
    }
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
    result->kind = Construct::Kind::DECLARATION_STRUCT;

    StructType* type = new StructType();
    types.push_back(std::unique_ptr<Type>(type));
    type->record = true;
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

                declarationScopes.back().push_back(result);
                break;
            }
            else if (isDeclaration(tokens, iterator, declarationScopes))
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
                fields.push_back(std::unique_ptr<Field>(field));
                field->structType = type;
                field->name = (iterator - 1)->value;
                field->declaration = fieldDeclaration;

                if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
                {
                    std::cerr << "Expected a type name" << std::endl;
                    return nullptr;
                }

                field->qualifiedType.type = findType((iterator - 1)->value, declarationScopes);

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
        declarationScopes.back().push_back(result);
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
    result->kind = Construct::Kind::DECLARATION_TYPE_DEFINITION;
    result->type = findType((iterator - 1)->value, declarationScopes);

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
                                                          std::vector<std::vector<Declaration*>>& declarationScopes)
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

            if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
            {
                std::cerr << "Expected a type name" << std::endl;
                return nullptr;
            }

            parameter->qualifiedType.type = findType((iterator - 1)->value, declarationScopes);

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

    result->qualifiedType.type = findType((iterator - 1)->value, declarationScopes);

    if (!result->qualifiedType.type)
    {
        std::cerr << "Invalid type: " << (iterator - 1)->value << std::endl;
        return nullptr;
    }

    if (checkToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        declarationScopes.back().push_back(result);

        // parse body
        if (!(result->body = parseCompoundStatement(tokens, iterator, declarationScopes)))
        {
            std::cerr << "Failed to parse a compound statement" << std::endl;
            return nullptr;
        }
    }
    else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        declarationScopes.back().push_back(result);
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
                                                          std::vector<std::vector<Declaration*>>& declarationScopes)
{
    VariableDeclaration* result = new VariableDeclaration();
    constructs.push_back(std::unique_ptr<VariableDeclaration>(result));
    result->kind = Construct::Kind::DECLARATION_VARIABLE;

    if (iterator->type == Token::Type::KEYWORD_STATIC)
    {
        result->qualifiedType.isStatic = true;
    }

    if (checkToken(Token::Type::KEYWORD_CONST, tokens, iterator))
    {
        result->qualifiedType.isConst = true;
    }

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Unexpected end of variable declaration" << std::endl;
        return nullptr;
    }

    result->name = (iterator - 1)->value;

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return nullptr;
    }

    result->qualifiedType.type = findType((iterator - 1)->value, declarationScopes);

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
        if (!(result->initialization = parseExpression(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }
    }
    else if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        if (!(result->initialization = parseExpression(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return nullptr;
        }
    }

    declarationScopes.back().push_back(result);

    return result;
}

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
        return parseReturnStatement(tokens, iterator, declarationScopes);
    }
    else if (isDeclaration(tokens, iterator, declarationScopes))
    {
        VariableDeclaration* declaration;
        if (!(declaration = parseVariableDeclaration(tokens, iterator, declarationScopes)))
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
    else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        Statement* statement = new Statement();
        constructs.push_back(std::unique_ptr<Construct>(statement));
        statement->kind = Construct::Kind::STATEMENT_EMPTY;

        return statement;
    }
    else
    {
        ExpressionStatement* expressionStatement = new ExpressionStatement();
        constructs.push_back(std::unique_ptr<Construct>(expressionStatement));
        expressionStatement->kind = Construct::Kind::STATEMENT_EXPRESSION;

        if (!(expressionStatement->expression = parseExpression(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        return expressionStatement;
    }

    return nullptr;
}

CompoundStatement* ASTContext::parseCompoundStatement(const std::vector<Token>& tokens,
                                                      std::vector<Token>::const_iterator& iterator,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes)
{
    declarationScopes.push_back(std::vector<Declaration*>());

    CompoundStatement* result = new CompoundStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_COMPOUND;

    for (;;)
    {
        if (checkToken(Token::Type::RIGHT_BRACE, tokens, iterator))
        {
            declarationScopes.pop_back();
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
    
    return result;
}

IfStatement* ASTContext::parseIfStatement(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes)
{
    IfStatement* result = new IfStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_IF;

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        if (!(result->condition = parseVariableDeclaration(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }
    }
    else
    {
        if (!(result->condition = parseExpression(tokens, iterator, declarationScopes)))
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
    if (!(statement = parseStatement(tokens, iterator, declarationScopes)))
    {
        std::cerr << "Failed to parse the statement" << std::endl;
        return nullptr;
    }

    result->body = statement;

    if (checkToken(Token::Type::KEYWORD_ELSE, tokens, iterator))
    {
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
    ForStatement* result = new ForStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_FOR;

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        if (!(result->initialization = parseVariableDeclaration(tokens, iterator, declarationScopes)))
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
        if (!(result->initialization = parseExpression(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }
    }

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        if (!(result->condition = parseVariableDeclaration(tokens, iterator, declarationScopes)))
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
        if (!(result->condition = parseExpression(tokens, iterator, declarationScopes)))
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
        if (!(result->increment = parseExpression(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return nullptr;
        }
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
    SwitchStatement* result = new SwitchStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_SWITCH;

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        if (!(result->condition = parseVariableDeclaration(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }
    }
    else
    {
        if (!(result->condition = parseExpression(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }
    }

    if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

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
    CaseStatement* result = new CaseStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_CASE;

    if (!(result->condition = parseExpression(tokens, iterator, declarationScopes)))
    {
        std::cerr << "Expected an expression" << std::endl;
        return nullptr;
    }

    if (!checkToken(Token::Type::COLON, tokens, iterator))
    {
        std::cerr << "Expected a colon" << std::endl;
        return nullptr;
    }

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
    WhileStatement* result = new WhileStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_WHILE;

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a left parenthesis" << std::endl;
        return nullptr;
    }

    if (isDeclaration(tokens, iterator, declarationScopes))
    {
        if (!(result->condition = parseVariableDeclaration(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }
    }
    else
    {
        if (!(result->condition = parseExpression(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }
    }

    if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Expected a right parenthesis" << std::endl;
        return nullptr;
    }

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
    DoStatement* result = new DoStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_DO;

    if (!(result->body = parseStatement(tokens, iterator, declarationScopes)))
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
    if (!(result->condition = parseExpression(tokens, iterator, declarationScopes)))
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
                                                  std::vector<std::vector<Declaration*>>& declarationScopes)
{
    ReturnStatement* result = new ReturnStatement();
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->kind = Construct::Kind::STATEMENT_RETURN;

    if (!(result->result = parseExpression(tokens, iterator, declarationScopes)))
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
                                        std::vector<std::vector<Declaration*>>& declarationScopes)
{
    return parseMultiplicationAssignment(tokens, iterator, declarationScopes);
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
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        std::unique_ptr<Construct> right;
        if (!(expression->rightExpression = parseAdditionAssignment(tokens, iterator, declarationScopes)))
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
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseAssignment(tokens, iterator, declarationScopes)))
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
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseTernary(tokens, iterator, declarationScopes)))
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
        expression->kind = Construct::Kind::OPERATOR_TERNARY;
        expression->value = (iterator - 1)->value;
        expression->condition = result;

        if (!(expression->leftExpression = parseTernary(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::COLON, tokens, iterator))
        {
            std::cerr << "Expected a colon" << std::endl;
            return nullptr;
        }

        if (!(expression->rightExpression = parseTernary(tokens, iterator, declarationScopes)))
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
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseGreaterThan(tokens, iterator, declarationScopes)))
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
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseLessThan(tokens, iterator, declarationScopes)))
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
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseAddition(tokens, iterator, declarationScopes)))
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
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseMultiplication(tokens, iterator, declarationScopes)))
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
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseNot(tokens, iterator, declarationScopes)))
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
                                 std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (checkToken(Token::Type::OPERATOR_NOT, tokens, iterator))
    {
        UnaryOperatorExpression* result = new UnaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::OPERATOR_UNARY;
        result->value = (iterator - 1)->value;

        if (!(result->expression = parseSign(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        result->qualifiedType.type = findType("bool", declarationScopes);

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

Expression* ASTContext::parseSign(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (checkTokens({Token::Type::OPERATOR_PLUS, Token::Type::OPERATOR_MINUS}, tokens, iterator))
    {
        UnaryOperatorExpression* result = new UnaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::OPERATOR_UNARY;
        result->value = (iterator - 1)->value;

        if (!(result->expression = parseMember(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        result->qualifiedType.type = result->expression->qualifiedType.type;

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

        if (!result->qualifiedType.type->record)
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
                                     std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (checkToken(Token::Type::LITERAL_INT, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION_LITERAL;
        result->qualifiedType.type = findType("int", declarationScopes);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkToken(Token::Type::LITERAL_FLOAT, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION_LITERAL;
        result->qualifiedType.type = findType("float", declarationScopes);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkToken(Token::Type::LITERAL_STRING, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION_LITERAL;
        result->qualifiedType.type = findType("string", declarationScopes);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkTokens({Token::Type::KEYWORD_TRUE, Token::Type::KEYWORD_FALSE}, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION_LITERAL;
        result->qualifiedType.type = findType("bool", declarationScopes);
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
            declRefExpression->declaration = findDeclaration(name, declarationScopes);

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
                         (parameter = parseExpression(tokens, iterator, declarationScopes)))
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
            declRefExpression->declaration = findDeclaration(name, declarationScopes);

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

            if (!(result->expression = parseExpression(tokens, iterator, declarationScopes)))
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
            result->declaration = findDeclaration((iterator - 1)->value, declarationScopes);

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

        if (!(result->expression = parseExpression(tokens, iterator, declarationScopes)))
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

void ASTContext::dump() const
{
    for (Declaration* declaration : declarations)
    {
        dumpConstruct(declaration);
    }
}

void ASTContext::dumpType(const Type* type, std::string indent) const
{
    if (type->record)
    {
        const StructType* structType = static_cast<const StructType*>(type);
        std::cout << ", name" << structType->name << std::endl;
    }
    else
    {
        const SimpleType* simpleType = static_cast<const SimpleType*>(type);
        std::cout << ", name" << simpleType->name << ", scalar: " << simpleType->scalar << std::endl;
    }
}

void ASTContext::dumpField(const Field* field, std::string indent) const
{
    std::cout << ", name" << field->name << ", type: " << field->qualifiedType.type->name << std::endl;
}

void ASTContext::dumpConstruct(const Construct* construct, std::string indent) const
{
    std::cout << indent << construct << " " << constructKindToString(construct->kind);

    switch (construct->kind)
    {
        case Construct::Kind::NONE:
        {
            std::cout << std::endl;
            break;
        }

        case Construct::Kind::DECLARATION_EMPTY:
        {
            std::cout << std::endl;
            break;
        }

        case Construct::Kind::DECLARATION_STRUCT:
        {
            const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(construct);

            std::cout << ", name: " << structDeclaration->type->name << std::endl;

            for (const FieldDeclaration* fieldDeclaration : structDeclaration->fieldDeclarations)
            {
                dumpConstruct(fieldDeclaration, indent + "  ");
            }

            break;
        }

        case Construct::Kind::DECLARATION_FIELD:
        {
            const FieldDeclaration* fieldDeclaration = static_cast<const FieldDeclaration*>(construct);

            std::cout << ", name: " << fieldDeclaration->field->name << std::endl;

            dumpField(fieldDeclaration->field, indent + "  ");
            break;
        }

        case Construct::Kind::DECLARATION_FUNCTION:
        {
            const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(construct);

            std::cout << ", name: " << functionDeclaration->name;

            if (functionDeclaration->qualifiedType.type)
            {
                std::cout << ", result type: " << functionDeclaration->qualifiedType.type->name;
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

        case Construct::Kind::DECLARATION_VARIABLE:
        {
            const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(construct);
            std::cout << ", name: " << variableDeclaration->name << ", type: " << variableDeclaration->qualifiedType.type->name << std::endl;
            break;
        }

        case Construct::Kind::DECLARATION_PARAMETER:
        {
            const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(construct);
            std::cout << ", name: " << parameterDeclaration->name << ", type: " << parameterDeclaration->qualifiedType.type->name << std::endl;
            break;
        }

        case Construct::Kind::EXPRESSION_CALL:
        {
            const CallExpression* callExpression = static_cast<const CallExpression*>(construct);

            std::cout << std::endl;

            dumpConstruct(callExpression->declarationReference, indent + "  ");

            for (Expression* parameter : callExpression->parameters)
            {
                dumpConstruct(parameter, indent + "  ");
            }
            
            break;
        }

        case Construct::Kind::EXPRESSION_LITERAL:
        {
            const Expression* expression = static_cast<const Expression*>(construct);
            std::cout << ", value: " << expression->value << ", type: " << expression->qualifiedType.type->name << std::endl;
            break;
        }

        case Construct::Kind::EXPRESSION_DECLARATION_REFERENCE:
        {
            const DeclarationReferenceExpression* declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(construct);

            std::cout << std::endl;

            dumpConstruct(declarationReferenceExpression->declaration, indent + "  ");
            break;
        }

        case Construct::Kind::EXPRESSION_PAREN:
        {
            const ParenExpression* parenExpression = static_cast<const ParenExpression*>(construct);

            std::cout << std::endl;

            dumpConstruct(parenExpression->expression, indent + "  ");
            break;
        }

        case Construct::Kind::EXPRESSION_MEMBER:
        {
            const MemberExpression* memberExpression = static_cast<const MemberExpression*>(construct);

            std::cout << std::endl;

            dumpConstruct(memberExpression->expression, indent + "  ");
            dumpField(memberExpression->field, indent + "  ");
            break;
        }

        case Construct::Kind::EXPRESSION_ARRAY_SUBSCRIPT:
        {
            const ArraySubscriptExpression* arraySubscriptExpression = static_cast<const ArraySubscriptExpression*>(construct);

            std::cout << std::endl;

            dumpConstruct(arraySubscriptExpression->declarationReference, indent + "  ");
            dumpConstruct(arraySubscriptExpression->expression, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_EMPTY:
        {
            std::cout << std::endl;
            break;
        }

        case Construct::Kind::STATEMENT_EXPRESSION:
        {
            const ExpressionStatement* expressionStatement = static_cast<const ExpressionStatement*>(construct);

            std::cout << std::endl;

            dumpConstruct(expressionStatement->expression, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_DECLARATION:
        {
            const DeclarationStatement* declarationStatement = static_cast<const DeclarationStatement*>(construct);

            std::cout << std::endl;

            dumpConstruct(declarationStatement->declaration, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_COMPOUND:
        {
            const CompoundStatement* compoundStatement = static_cast<const CompoundStatement*>(construct);

            std::cout << std::endl;

            for (Statement* statement : compoundStatement->statements)
            {
                dumpConstruct(statement, indent + "  ");
            }
            break;
        }

        case Construct::Kind::STATEMENT_IF:
        {
            const IfStatement* ifStatement = static_cast<const IfStatement*>(construct);

            std::cout << std::endl;

            dumpConstruct(ifStatement->condition, indent + "  ");
            dumpConstruct(ifStatement->body, indent + "  ");
            if (ifStatement->elseBody) dumpConstruct(ifStatement->elseBody, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_FOR:
        {
            const ForStatement* forStatement = static_cast<const ForStatement*>(construct);

            std::cout << std::endl;

            dumpConstruct(forStatement->initialization, indent + "  ");
            dumpConstruct(forStatement->condition, indent + "  ");
            dumpConstruct(forStatement->increment, indent + "  ");
            dumpConstruct(forStatement->body, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_SWITCH:
        {
            const SwitchStatement* switchStatement = static_cast<const SwitchStatement*>(construct);

            std::cout << std::endl;

            dumpConstruct(switchStatement->condition, indent + "  ");
            dumpConstruct(switchStatement->body, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_CASE:
        {
            const CaseStatement* caseStatement = static_cast<const CaseStatement*>(construct);

            std::cout << std::endl;

            dumpConstruct(caseStatement->condition, indent + "  ");
            dumpConstruct(caseStatement->body, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_WHILE:
        {
            const WhileStatement* whileStatement = static_cast<const WhileStatement*>(construct);

            std::cout << std::endl;

            dumpConstruct(whileStatement->condition, indent + "  ");
            dumpConstruct(whileStatement->body, indent + "  ");
            break;
        }

        case Construct::Kind::STATEMENT_DO:
        {
            const DoStatement* doStatement = static_cast<const DoStatement*>(construct);

            std::cout << std::endl;

            dumpConstruct(doStatement->body, indent + "  ");
            dumpConstruct(doStatement->condition, indent + "  ");
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
            const ReturnStatement* returnStatement = static_cast<const ReturnStatement*>(construct);

            std::cout << std::endl;

            if (returnStatement->result)
            {
                dumpConstruct(returnStatement->result, indent + "  ");
            }
            break;
        }

        case Construct::Kind::OPERATOR_UNARY:
        {
            const UnaryOperatorExpression* unaryOperatorExpression = static_cast<const UnaryOperatorExpression*>(construct);

            std::cout <<", operator: " << unaryOperatorExpression->value << std::endl;

            dumpConstruct(unaryOperatorExpression->expression, indent + "  ");
            break;
        }
            
        case Construct::Kind::OPERATOR_BINARY:
        {
            const BinaryOperatorExpression* binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(construct);
            
            std::cout <<", operator: " << binaryOperatorExpression->value << std::endl;

            dumpConstruct(binaryOperatorExpression->leftExpression, indent + "  ");
            dumpConstruct(binaryOperatorExpression->rightExpression, indent + "  ");
            break;
        }
            
        case Construct::Kind::OPERATOR_TERNARY:
        {
            const TernaryOperatorExpression* ternaryOperatorExpression = static_cast<const TernaryOperatorExpression*>(construct);
            
            std::cout << std::endl;

            dumpConstruct(ternaryOperatorExpression->condition, indent + "  ");
            dumpConstruct(ternaryOperatorExpression->leftExpression, indent + "  ");
            dumpConstruct(ternaryOperatorExpression->rightExpression, indent + "  ");
            break;
        }
    }
}
