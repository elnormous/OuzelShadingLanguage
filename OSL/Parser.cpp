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
    boolType->typeKind = Type::Kind::BUILTIN;
    boolType->name = "bool";
    boolType->scalar = true;
    types.push_back(std::move(boolType));

    std::unique_ptr<SimpleType> intType(new SimpleType());
    intType->typeKind = Type::Kind::BUILTIN;
    intType->name = "int";
    intType->scalar = true;
    types.push_back(std::move(intType));

    std::unique_ptr<SimpleType> floatType(new SimpleType());
    floatType->typeKind = Type::Kind::BUILTIN;
    floatType->name = "float";
    floatType->scalar = true;
    types.push_back(std::move(floatType));

    std::unique_ptr<StructType> vec2Type(new StructType());
    vec2Type->typeKind = Type::Kind::STRUCT;
    vec2Type->name = "vec2";
    types.push_back(std::move(vec2Type));

    std::unique_ptr<StructType> vec3Type(new StructType());
    vec3Type->typeKind = Type::Kind::STRUCT;
    vec3Type->name = "vec3";
    types.push_back(std::move(vec3Type));

    std::unique_ptr<StructType> vec4Type(new StructType());
    vec4Type->typeKind = Type::Kind::STRUCT;
    vec4Type->name = "vec4";
    types.push_back(std::move(vec4Type));

    std::unique_ptr<StructType> stringType(new StructType());
    stringType->typeKind = Type::Kind::STRUCT;
    stringType->name = "string";
    types.push_back(std::move(stringType));

    std::unique_ptr<StructType> samplerStateType(new StructType());
    samplerStateType->typeKind = Type::Kind::STRUCT;
    samplerStateType->name = "SamplerState";
    types.push_back(std::move(samplerStateType));

    std::unique_ptr<StructType> texture2DType(new StructType());
    texture2DType->typeKind = Type::Kind::STRUCT;
    texture2DType->name = "Texture2D";
    types.push_back(std::move(texture2DType));

    auto iterator = tokens.cbegin();

    std::vector<std::vector<Declaration*>> declarationScopes;
    declarationScopes.push_back(std::vector<Declaration*>());

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

Declaration* ASTContext::parseTopLevelDeclaration(const std::vector<Token>& tokens,
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
        declaration->kind = Construct::Kind::DECLARATION;
        declaration->declarationKind = Declaration::Kind::EMPTY;

        return declaration;
    }
    else
    {
        std::cerr << "Expected a declaration" << std::endl;
        return nullptr;
    }
}

Declaration* ASTContext::parseDeclaration(const std::vector<Token>& tokens,
                                          std::vector<Token>::const_iterator& iterator,
                                          std::vector<std::vector<Declaration*>>& declarationScopes)
{
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
    result->kind = Construct::Kind::DECLARATION;
    result->declarationKind = Declaration::Kind::STRUCT;

    StructType* type = new StructType();
    types.push_back(std::unique_ptr<Type>(type));
    type->typeKind = Type::Kind::STRUCT;
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
            else
            {
                FieldDeclaration* fieldDeclaration;
                if (!(fieldDeclaration = parseFieldDeclaration(tokens, iterator, declarationScopes)))
                {
                    return nullptr;
                }

                fieldDeclaration->field->structType = type;
                type->fields.push_back(fieldDeclaration->field);

                result->fieldDeclarations.push_back(fieldDeclaration);
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

FieldDeclaration* ASTContext::parseFieldDeclaration(const std::vector<Token>& tokens,
                                                    std::vector<Token>::const_iterator& iterator,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes)
{
    FieldDeclaration* fieldDeclaration = new FieldDeclaration();
    constructs.push_back(std::unique_ptr<Construct>(fieldDeclaration));
    fieldDeclaration->kind = Construct::Kind::DECLARATION;
    fieldDeclaration->declarationKind = Declaration::Kind::FIELD;

    Field* field = new Field();
    fields.push_back(std::unique_ptr<Field>(field));
    field->declaration = fieldDeclaration;

    fieldDeclaration->field = field;

    for (;;)
    {
        if (checkToken(Token::Type::KEYWORD_CONST, tokens, iterator)) field->qualifiedType.isConst = true;
        else if (checkToken(Token::Type::KEYWORD_STATIC, tokens, iterator)) field->qualifiedType.isStatic = true;
        else break;
    }

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

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected an identifier" << std::endl;
        return nullptr;
    }

    field->name = (iterator - 1)->value;

    if (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator)) // parse attributes
    {
        bool firstAttribute = true;

        for (;;)
        {
            if (checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
            {
                break;
            }
            else if (firstAttribute || checkToken(Token::Type::COMMA, tokens, iterator))
            {
                firstAttribute = false;


            }
            else
            {
                std::cerr << "Expected a comma" << std::endl;
                return nullptr;
            }
        }
    }

    if (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
    {
        if (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
        {
            /*checkToken(Token::Type::IDENTIFIER, tokens, iterator);

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
            }*/
        }
        else if (checkToken(Token::Type::LITERAL_INT, tokens, iterator))
        {
            int size = std::stoi((iterator - 1)->value);

            if (size <= 0)
            {
                std::cerr << "Array size must be greater than zero" << std::endl;
                return nullptr;
            }

            field->qualifiedType.dimensions.push_back(static_cast<uint32_t>(size));

            if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
            {
                std::cerr << "Expected a right bracket" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cerr << "Expected an integer literal" << std::endl;
            return nullptr;
        }
    }

    if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        std::cerr << "Expected a semicolon" << std::endl;
        return nullptr;
    }

    return fieldDeclaration;
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
    result->kind = Construct::Kind::DECLARATION;
    result->declarationKind = Declaration::Kind::FUNCTION;
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
        else if (firstParameter || checkToken(Token::Type::COMMA, tokens, iterator))
        {
            firstParameter = false;

            if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
            {
                std::cerr << "Expected a keyword" << std::endl;
                return nullptr;
            }

            ParameterDeclaration* parameter = new ParameterDeclaration();
            constructs.push_back(std::unique_ptr<Construct>(parameter));
            parameter->kind = Construct::Kind::DECLARATION;
            parameter->declarationKind = Declaration::Kind::PARAMETER;
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
                if (!checkToken(Token::Type::LITERAL_INT, tokens, iterator))
                {
                    std::cerr << "Expected an integer literal" << std::endl;
                    return nullptr;
                }

                int size = std::stoi((iterator - 1)->value);

                if (size <= 0)
                {
                    std::cerr << "Array size must be greater than zero" << std::endl;
                    return nullptr;
                }

                parameter->qualifiedType.dimensions.push_back(static_cast<uint32_t>(size));

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
            std::cerr << "Expected a comma" << std::endl;
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
    result->kind = Construct::Kind::DECLARATION;
    result->declarationKind = Declaration::Kind::VARIABLE;

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
        if (!checkToken(Token::Type::LITERAL_INT, tokens, iterator))
        {
            std::cerr << "Expected an integer literal" << std::endl;
            return nullptr;
        }

        int size = std::stoi((iterator - 1)->value);

        if (size <= 0)
        {
            std::cerr << "Array size must be greater than zero" << std::endl;
            return nullptr;
        }

        result->qualifiedType.dimensions.push_back(static_cast<uint32_t>(size));

        if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
        {
            std::cerr << "Expected a right bracket" << std::endl;
            return nullptr;
        }
    }

    if (checkToken(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
    {
        if (!(result->initialization = parseMultiplicationAssignment(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }
    }
    else if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        if (!(result->initialization = parseMultiplicationAssignment(tokens, iterator, declarationScopes)))
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
        result->kind = Construct::Kind::STATEMENT;
        result->statementKind = Statement::Kind::BREAK;

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
        result->kind = Construct::Kind::STATEMENT;
        result->statementKind = Statement::Kind::CONTINUE;

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
        declarationStatement->kind = Construct::Kind::STATEMENT;
        declarationStatement->statementKind = Statement::Kind::DECLARATION;
        declarationStatement->declaration = declaration;

        return declarationStatement;
    }
    else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        Statement* statement = new Statement();
        constructs.push_back(std::unique_ptr<Construct>(statement));
        statement->kind = Construct::Kind::STATEMENT;
        statement->statementKind = Statement::Kind::EMPTY;

        return statement;
    }
    else
    {
        ExpressionStatement* expressionStatement = new ExpressionStatement();
        constructs.push_back(std::unique_ptr<Construct>(expressionStatement));
        expressionStatement->kind = Construct::Kind::STATEMENT;
        expressionStatement->statementKind = Statement::Kind::EXPRESSION;

        if (!(expressionStatement->expression = parseComma(tokens, iterator, declarationScopes)))
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
    result->kind = Construct::Kind::STATEMENT;
    result->statementKind = Statement::Kind::COMPOUND;

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
    result->kind = Construct::Kind::STATEMENT;
    result->statementKind = Statement::Kind::IF;

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
    result->kind = Construct::Kind::STATEMENT;
    result->statementKind = Statement::Kind::FOR;

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
        result->kind = Construct::Kind::STATEMENT;
        result->statementKind = Statement::Kind::EMPTY;
        result->initialization = emptyStatement;
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
        result->kind = Construct::Kind::STATEMENT;
        result->statementKind = Statement::Kind::EMPTY;
        result->initialization = emptyStatement;
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
    }

    if (checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
    {
        Statement* emptyStatement = new Statement();
        constructs.push_back(std::unique_ptr<Construct>(emptyStatement));
        result->kind = Construct::Kind::STATEMENT;
        result->statementKind = Statement::Kind::EMPTY;
        result->initialization = emptyStatement;
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
    result->kind = Construct::Kind::STATEMENT;
    result->statementKind = Statement::Kind::SWITCH;

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
    result->kind = Construct::Kind::STATEMENT;
    result->statementKind = Statement::Kind::CASE;

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
    result->kind = Construct::Kind::STATEMENT;
    result->statementKind = Statement::Kind::WHILE;

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
    result->kind = Construct::Kind::STATEMENT;
    result->statementKind = Statement::Kind::DO;

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
    if (!(result->condition = parseComma(tokens, iterator, declarationScopes)))
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
    result->kind = Construct::Kind::STATEMENT;
    result->statementKind = Statement::Kind::RETURN;

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
        expression->kind = Construct::Kind::EXPRESSION;
        expression->expressionKind = Expression::Kind::BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        std::unique_ptr<Construct> right;
        if (!(expression->rightExpression = parseAdditionAssignment(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        expression->qualifiedType.type = expression->rightExpression->qualifiedType.type;

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
        expression->kind = Construct::Kind::EXPRESSION;
        expression->expressionKind = Expression::Kind::BINARY;
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
        expression->kind = Construct::Kind::EXPRESSION;
        expression->expressionKind = Expression::Kind::BINARY;
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
        expression->kind = Construct::Kind::EXPRESSION;
        expression->expressionKind = Expression::Kind::BINARY;
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
        expression->kind = Construct::Kind::EXPRESSION;
        expression->expressionKind = Expression::Kind::TERNARY;
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
        expression->kind = Construct::Kind::EXPRESSION;
        expression->expressionKind = Expression::Kind::BINARY;
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
        expression->kind = Construct::Kind::EXPRESSION;
        expression->expressionKind = Expression::Kind::BINARY;
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
        expression->kind = Construct::Kind::EXPRESSION;
        expression->expressionKind = Expression::Kind::BINARY;
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
        expression->kind = Construct::Kind::EXPRESSION;
        expression->expressionKind = Expression::Kind::BINARY;
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
        expression->kind = Construct::Kind::EXPRESSION;
        expression->expressionKind = Expression::Kind::BINARY;
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
        result->kind = Construct::Kind::EXPRESSION;
        result->expressionKind = Expression::Kind::UNARY;
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
        result->kind = Construct::Kind::EXPRESSION;
        result->expressionKind = Expression::Kind::UNARY;
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
        expression->kind = Construct::Kind::EXPRESSION;
        expression->expressionKind = Expression::Kind::MEMBER;
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

        if (result->qualifiedType.type->typeKind != Type::Kind::STRUCT)
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
        result->kind = Construct::Kind::EXPRESSION;
        result->expressionKind = Expression::Kind::LITERAL;
        result->qualifiedType.type = findType("int", declarationScopes);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkToken(Token::Type::LITERAL_FLOAT, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION;
        result->expressionKind = Expression::Kind::LITERAL;
        result->qualifiedType.type = findType("float", declarationScopes);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkToken(Token::Type::LITERAL_STRING, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION;
        result->expressionKind = Expression::Kind::LITERAL;
        result->qualifiedType.type = findType("string", declarationScopes);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkTokens({Token::Type::KEYWORD_TRUE, Token::Type::KEYWORD_FALSE}, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION;
        result->expressionKind = Expression::Kind::LITERAL;
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
            result->kind = Construct::Kind::EXPRESSION;
            result->expressionKind = Expression::Kind::CALL;

            DeclarationReferenceExpression* declRefExpression = new DeclarationReferenceExpression();
            constructs.push_back(std::unique_ptr<Construct>(declRefExpression));
            declRefExpression->kind = Construct::Kind::EXPRESSION;
            declRefExpression->expressionKind = Expression::Kind::DECLARATION_REFERENCE;
            declRefExpression->declaration = findDeclaration(name, declarationScopes);

            if (!declRefExpression->declaration)
            {
                std::cerr << "Invalid declaration reference: " << name << std::endl;
                return nullptr;
            }

            result->declarationReference = declRefExpression;

            if (declRefExpression->declaration->declarationKind != Declaration::Kind::FUNCTION)
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
                if (checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
                {
                    break;
                }
                else if (firstParameter || checkToken(Token::Type::COMMA, tokens, iterator))
                {
                    firstParameter = false;

                    Expression* parameter;

                    if (!(parameter = parseMultiplicationAssignment(tokens, iterator, declarationScopes)))
                    {
                        return nullptr;
                    }

                    result->parameters.push_back(parameter);
                }
                else
                {
                    std::cerr << "Expected a comma" << std::endl;
                    return nullptr;
                }
            }

            return result;
        }
        else if (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
        {
            ArraySubscriptExpression* result = new ArraySubscriptExpression();
            constructs.push_back(std::unique_ptr<Construct>(result));
            result->kind = Construct::Kind::EXPRESSION;
            result->expressionKind = Expression::Kind::ARRAY_SUBSCRIPT;

            DeclarationReferenceExpression* declRefExpression = new DeclarationReferenceExpression();
            constructs.push_back(std::unique_ptr<Construct>(declRefExpression));
            declRefExpression->kind = Construct::Kind::EXPRESSION;
            declRefExpression->expressionKind = Expression::Kind::DECLARATION_REFERENCE;
            declRefExpression->declaration = findDeclaration(name, declarationScopes);

            if (!declRefExpression->declaration)
            {
                std::cerr << "Invalid declaration reference: " << name << std::endl;
                return nullptr;
            }

            if (declRefExpression->declaration->declarationKind != Declaration::Kind::VARIABLE)
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

            return result;
        }
        else
        {
            DeclarationReferenceExpression* result = new DeclarationReferenceExpression();
            constructs.push_back(std::unique_ptr<Construct>(result));
            result->kind = Construct::Kind::EXPRESSION;
            result->expressionKind = Expression::Kind::DECLARATION_REFERENCE;
            result->declaration = findDeclaration((iterator - 1)->value, declarationScopes);

            if (!result->declaration)
            {
                std::cerr << "Invalid declaration reference: " << (iterator - 1)->value << std::endl;
                return nullptr;
            }

            switch (result->declaration->declarationKind)
            {
                case Declaration::Kind::STRUCT:
                {
                    StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->declaration);
                    result->qualifiedType.type = structDeclaration->type;
                    break;
                }
                case Declaration::Kind::VARIABLE:
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
        result->kind = Construct::Kind::EXPRESSION;
        result->expressionKind = Expression::Kind::PAREN;

        if (!(result->expression = parseComma(tokens, iterator, declarationScopes)))
        {
            return nullptr;
        }

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return nullptr;
        }

        result->qualifiedType.type = result->expression->qualifiedType.type;

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
    std::cout << indent << typeKindToString(type->typeKind);

    switch (type->typeKind)
    {
        case Type::Kind::NONE:
        {
            break;
        }

        case Type::Kind::STRUCT:
        {
            const StructType* structType = static_cast<const StructType*>(type);
            std::cout << ", name: " << structType->name << std::endl;
            break;
        }

        case Type::Kind::BUILTIN:
        {
            const SimpleType* simpleType = static_cast<const SimpleType*>(type);
            std::cout << ", name: " << simpleType->name << ", scalar: " << simpleType->scalar << std::endl;
            break;
        }
    }
}

void ASTContext::dumpField(const Field* field, std::string indent) const
{
    std::cout << indent << ", name: " << field->name << ", type: " << field->qualifiedType.type->name << std::endl;
}

void ASTContext::dumpDeclaration(const Declaration* declaration, std::string indent) const
{
    std::cout << indent << declarationKindToString(declaration->declarationKind);

    switch (declaration->declarationKind)
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

        case Declaration::Kind::STRUCT:
        {
            const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(declaration);

            std::cout << ", name: " << structDeclaration->type->name << std::endl;

            for (const FieldDeclaration* fieldDeclaration : structDeclaration->fieldDeclarations)
            {
                dumpConstruct(fieldDeclaration, " ");
            }

            break;
        }

        case Declaration::Kind::FIELD:
        {
            const FieldDeclaration* fieldDeclaration = static_cast<const FieldDeclaration*>(declaration);

            dumpField(fieldDeclaration->field);
            break;
        }

        case Declaration::Kind::FUNCTION:
        {
            const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(declaration);

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

        case Declaration::Kind::VARIABLE:
        {
            const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(declaration);
            std::cout << ", name: " << variableDeclaration->name << ", type: " << variableDeclaration->qualifiedType.type->name << std::endl;
            break;
        }

        case Declaration::Kind::PARAMETER:
        {
            const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(declaration);
            std::cout << ", name: " << parameterDeclaration->name << ", type: " << parameterDeclaration->qualifiedType.type->name << std::endl;
            break;
        }

        default:
            break;
    }
}

void ASTContext::dumpStatement(const Statement* statement, std::string indent) const
{
    std::cout << indent << statementKindToString(statement->statementKind);

    switch (statement->statementKind)
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

            dumpConstruct(forStatement->initialization, indent + "  ");
            dumpConstruct(forStatement->condition, indent + "  ");
            dumpConstruct(forStatement->increment, indent + "  ");
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
    std::cout << indent << expressionKindToString(expression->expressionKind);

    switch (expression->expressionKind)
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
            std::cout << ", value: " << expression->value << ", type: " << expression->qualifiedType.type->name << std::endl;
            break;
        }

        case Expression::Kind::DECLARATION_REFERENCE:
        {
            const DeclarationReferenceExpression* declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(expression);

            std::cout << std::endl;

            dumpConstruct(declarationReferenceExpression->declaration, indent + "  ");
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
            dumpField(memberExpression->field, indent + "  ");
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

            std::cout <<", operator: " << unaryOperatorExpression->value << std::endl;

            dumpConstruct(unaryOperatorExpression->expression, indent + "  ");
            break;
        }

        case Expression::Kind::BINARY:
        {
            const BinaryOperatorExpression* binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(expression);

            std::cout <<", operator: " << binaryOperatorExpression->value << std::endl;

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
    std::cout << indent << construct << " " << constructKindToString(construct->kind);

    switch (construct->kind)
    {
        case Construct::Kind::NONE:
        {
            std::cout << std::endl;
            break;
        }

        case Construct::Kind::DECLARATION:
        {
            const Declaration* declaration = static_cast<const Declaration*>(construct);
            dumpDeclaration(declaration, " ");
            break;
        }

        case Construct::Kind::STATEMENT:
        {
            const Statement* statement = static_cast<const Statement*>(construct);
            dumpStatement(statement, " ");
            break;
        }

        case Construct::Kind::EXPRESSION:
        {
            const Expression* expression = static_cast<const Expression*>(construct);
            dumpExpression(expression, " ");
            break;
        }
    }
}
