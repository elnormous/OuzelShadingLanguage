//
//  Parser.cpp
//  OSLParser
//
//  Created by Elviss Strazdins on 25/04/2017.
//  Copyright (c) 2017 Elviss Strazdins. All rights reserved.
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
            if (!(decl = parseStructDecl(tokens, iterator, declarations)))
            {
                std::cerr << "Failed to parse a structure declaration" << std::endl;
                return nullptr;
            }

            result->declarations.push_back(decl);
        }
        /*else if (checkToken(Token::Type::KEYWORD_TYPEDEF, tokens, iterator))
        {
            std::unique_ptr<Construct> decl;
            if (!parseTypedefDecl(tokens, iterator, declarations, decl))
            {
                std::cerr << "Failed to parse a type definition declaration" << std::endl;
                return nullptr;
            }

            result->children.push_back(std::move(decl));
        }*/
        else if (checkToken(Token::Type::KEYWORD_FUNCTION, tokens, iterator))
        {
            FunctionDeclaration* decl;
            if (!(decl = parseFunctionDecl(tokens, iterator, declarations)))
            {
                std::cerr << "Failed to parse a function declaration" << std::endl;
                return nullptr;
            }

            result->declarations.push_back(decl);
        }
        else if (checkTokens({Token::Type::KEYWORD_STATIC, Token::Type::KEYWORD_CONST, Token::Type::KEYWORD_VAR}, tokens, iterator))
        {
            VariableDeclaration* decl;
            if (!(decl = parseVariableDecl(tokens, iterator, declarations)))
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

StructDeclaration* ASTContext::parseStructDecl(const std::vector<Token>& tokens,
                                               std::vector<Token>::const_iterator& iterator,
                                               std::vector<std::vector<Declaration*>>& declarations)
{
    StructDeclaration* result = new StructDeclaration();
    constructs.push_back(std::unique_ptr<Construct>(result));

    if (checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        result->kind = Construct::Kind::DECLARATION_STRUCT;

        StructType* type = new StructType();
        constructs.push_back(std::unique_ptr<Construct>(type));
        type->kind = Construct::Kind::TYPE_STRUCT;
        type->name = (iterator - 1)->value;
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
                    field->name = (iterator - 1)->value;

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

                    field->reference = findDeclaration((iterator - 1)->value, declarations);

                    if (!field->reference)
                    {
                        std::cerr << "Invalid declaration reference: " << (iterator - 1)->value << std::endl;
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
    }

    return result;
}

/*bool ASTContext::parseTypedefDecl(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<Declaration*>>& declarations,
                                  std::unique_ptr<Construct>& result)
{
    std::cerr << "Typedef is not supported" << std::endl;
    return false;

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return false;
    }

    result.reset(new Construct());
    result->kind = Construct::Kind::DECLARATION_TYPE_DEFINITION;
    result->reference = findDeclaration((iterator - 1)->value, declarations);

    if (!result->reference)
    {
        std::cerr << "Invalid declaration reference: " << (iterator - 1)->value << std::endl;
        return false;
    }

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return false;
    }

    result->name = (iterator - 1)->value;

    if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        std::cerr << "Expected a semicolon" << std::endl;
        return false;
    }

    return true;
}*/

FunctionDeclaration* ASTContext::parseFunctionDecl(const std::vector<Token>& tokens,
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

            parameter->reference = findDeclaration((iterator - 1)->value, declarations);

            if (!parameter->reference)
            {
                std::cerr << "Invalid declaration reference: " << (iterator - 1)->value << std::endl;
                return nullptr;
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

    result->reference = findDeclaration((iterator - 1)->value, declarations);

    if (!result->reference)
    {
        std::cerr << "Invalid declaration reference: " << (iterator - 1)->value << std::endl;
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

VariableDeclaration* ASTContext::parseVariableDecl(const std::vector<Token>& tokens,
                                                   std::vector<Token>::const_iterator& iterator,
                                                   std::vector<std::vector<Declaration*>>& declarations)
{
    VariableDeclaration* result = new VariableDeclaration();
    constructs.push_back(std::unique_ptr<VariableDeclaration>(result));
    result->kind = Construct::Kind::DECLARATION_VARIABLE;

    if ((iterator - 1)->type == Token::Type::KEYWORD_STATIC)
    {
        result->isStatic = true;
    }
    else
    {
        --iterator;
    }

    if (checkToken(Token::Type::KEYWORD_CONST, tokens, iterator))
    {
        result->isConst = true;
    }
    else if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
    {
        result->isConst = false;
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

    result->reference = findDeclaration((iterator - 1)->value, declarations);

    if (!result->reference)
    {
        std::cerr << "Invalid declaration reference: " << (iterator - 1)->value << std::endl;
        return nullptr;
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
        if (!(declaration = parseVariableDecl(tokens, iterator, declarations)))
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
    if (!(declaration = parseVariableDecl(tokens, iterator, declarations)))
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
        return nullptr;
    }

    result->body = statement;

    if (!checkToken(Token::Type::KEYWORD_ELSE, tokens, iterator))
    {
        if (!(statement = parseStatement(tokens, iterator, declarations)))
        {
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
        constructs.push_back(std::unique_ptr<Construct>(result));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        std::unique_ptr<Construct> right;
        if (!(expression->rightExpression = parseAdditionAssignment(tokens, iterator, declarations)))
        {
            return nullptr;
        }

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
        constructs.push_back(std::unique_ptr<Construct>(result));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseAssignment(tokens, iterator, declarations)))
        {
            return nullptr;
        }

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
        constructs.push_back(std::unique_ptr<Construct>(result));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseTernary(tokens, iterator, declarations)))
        {
            return nullptr;
        }

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
        constructs.push_back(std::unique_ptr<Construct>(result));
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
        constructs.push_back(std::unique_ptr<Construct>(result));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseGreaterThan(tokens, iterator, declarations)))
        {
            return nullptr;
        }

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
        constructs.push_back(std::unique_ptr<Construct>(result));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseLessThan(tokens, iterator, declarations)))
        {
            return nullptr;
        }

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
        constructs.push_back(std::unique_ptr<Construct>(result));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseAddition(tokens, iterator, declarations)))
        {
            return nullptr;
        }

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
        constructs.push_back(std::unique_ptr<Construct>(result));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseMultiplication(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        result = expression;
    }
    
    return result;
}

Expression* ASTContext::parseMultiplication(const std::vector<Token>& tokens,
                                            std::vector<Token>::const_iterator& iterator,
                                            std::vector<std::vector<Declaration*>>& declarations)
{
    Expression* result;
    if (!(result = parseUnary(tokens, iterator, declarations)))
    {
        return nullptr;
    }

    while (checkTokens({Token::Type::OPERATOR_MULTIPLY, Token::Type::OPERATOR_DIVIDE}, tokens, iterator))
    {
        BinaryOperatorExpression* expression = new BinaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        expression->kind = Construct::Kind::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;
        expression->leftExpression = result;

        if (!(expression->rightExpression = parseUnary(tokens, iterator, declarations)))
        {
            return nullptr;
        }

        result = expression;
    }

    return result;
}

Expression* ASTContext::parseUnary(const std::vector<Token>& tokens,
                                   std::vector<Token>::const_iterator& iterator,
                                   std::vector<std::vector<Declaration*>>& declarations)
{
    if (checkTokens({Token::Type::OPERATOR_PLUS, Token::Type::OPERATOR_MINUS, Token::Type::OPERATOR_NOT}, tokens, iterator))
    {
        UnaryOperatorExpression* result = new UnaryOperatorExpression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::OPERATOR_UNARY;
        result->value = (iterator - 1)->value;

        if (!(result->expression = parseMember(tokens, iterator, declarations)))
        {
            return nullptr;
        }

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
        constructs.push_back(std::unique_ptr<Construct>(result));
        expression->kind = Construct::Kind::EXPRESSION_MEMBER;
        expression->expression = result;

        Declaration* declaration = findDeclaration((iterator - 2)->value, declarations);

        if (!declaration)
        {
            std::cerr << "Invalid declaration reference: " << (iterator - 2)->value << std::endl;
            return nullptr;
        }

        if (declaration->kind != Construct::Kind::DECLARATION_STRUCT)
        {
            std::cerr << "Expected a reference to structure, but got: " << (iterator - 2)->value << std::endl;
            return nullptr;
        }

        StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(declaration);

        std::unique_ptr<Construct> declRefExpression(new Construct());
        declRefExpression->kind = Construct::Kind::EXPRESSION_DECLARATION_REFERENCE;
        declRefExpression->reference = declaration;

        // TODO: implement
        //expression->declarationReference = declRefExpression;

        if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
        {
            std::cerr << "Expected an identifier" << std::endl;
            return nullptr;
        }

        DeclarationReferenceExpression* fieldRefExpression = new DeclarationReferenceExpression();
        constructs.push_back(std::unique_ptr<Construct>(fieldRefExpression));
        fieldRefExpression->kind = Construct::Kind::EXPRESSION_DECLARATION_REFERENCE;
        fieldRefExpression->reference = findField((iterator - 1)->value, structDeclaration);

        if (!fieldRefExpression->reference)
        {
            std::cerr << "Invalid member reference: " << (iterator - 1)->value << std::endl;
            return nullptr;
        }

        expression->declarationReference = fieldRefExpression;

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
        result->reference = findDeclaration("int", declarations);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkToken(Token::Type::LITERAL_FLOAT, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION_LITERAL;
        result->reference = findDeclaration("float", declarations);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkToken(Token::Type::LITERAL_STRING, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION_LITERAL;
        result->reference = findDeclaration("string", declarations);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkTokens({Token::Type::KEYWORD_TRUE, Token::Type::KEYWORD_FALSE}, tokens, iterator))
    {
        Expression* result = new Expression();
        constructs.push_back(std::unique_ptr<Construct>(result));
        result->kind = Construct::Kind::EXPRESSION_LITERAL;
        result->reference = findDeclaration("bool", declarations);
        result->value = (iterator - 1)->value;
        return result;
    }
    else if (checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            CallExpression* result = new CallExpression();
            constructs.push_back(std::unique_ptr<Construct>(result));
            result->kind = Construct::Kind::EXPRESSION_CALL;

            DeclarationReferenceExpression* declRefExpression = new DeclarationReferenceExpression();
            constructs.push_back(std::unique_ptr<Construct>(declRefExpression));
            declRefExpression->kind = Construct::Kind::EXPRESSION_DECLARATION_REFERENCE;
            declRefExpression->reference = findDeclaration((iterator - 2)->value, declarations);

            if (!declRefExpression->reference)
            {
                std::cerr << "Invalid declaration reference: " << (iterator - 2)->value << std::endl;
                return nullptr;
            }

            result->declarationReference = declRefExpression;

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
            declRefExpression->reference = findDeclaration((iterator - 2)->value, declarations);

            if (!declRefExpression->reference)
            {
                std::cerr << "Invalid declaration reference: " << (iterator - 2)->value << std::endl;
                return nullptr;
            }

            result->declarationReference = declRefExpression;

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
            result->reference = findDeclaration((iterator - 1)->value, declarations);

            if (!result->reference)
            {
                std::cerr << "Invalid declaration reference: " << (iterator - 1)->value << std::endl;
                return nullptr;
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

    //if (!node->name.empty()) std::cout << ", name: " << node->name;
    if (node->reference)
    {
        std::cout << ", reference: ";
        //if (node->isStatic) std::cout << "static ";
        //if (node->isConst) std::cout << "const ";
        //std::cout << node->reference->name;

    }
    //if (!node->value.empty()) std::cout << ", value: " << node->value;
    //if (node->semantic != Construct::Semantic::NONE) std::cout << ", semantic: " << semanticToString(node->semantic);

    std::cout << std::endl;

    /*for (const std::unique_ptr<Construct>& child : node->children)
    {
        ASTContext::dumpNode(child.get(), indent + "  ");
    }*/
}
