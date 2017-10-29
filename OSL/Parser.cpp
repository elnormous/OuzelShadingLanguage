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
    std::unique_ptr<ASTNode> boolType(new ASTNode());
    boolType->type = ASTNode::Type::DECLARATION_STRUCT;
    boolType->name = "bool";
    builtinDeclarations.push_back(std::move(boolType));

    std::unique_ptr<ASTNode> intType(new ASTNode());
    intType->type = ASTNode::Type::DECLARATION_STRUCT;
    intType->name = "int";
    builtinDeclarations.push_back(std::move(intType));

    std::unique_ptr<ASTNode> floatType(new ASTNode());
    floatType->type = ASTNode::Type::DECLARATION_STRUCT;
    floatType->name = "float";
    builtinDeclarations.push_back(std::move(floatType));

    std::unique_ptr<ASTNode> vec2Type(new ASTNode());
    vec2Type->type = ASTNode::Type::DECLARATION_STRUCT;
    vec2Type->name = "vec2";
    builtinDeclarations.push_back(std::move(vec2Type));

    std::unique_ptr<ASTNode> vec3Type(new ASTNode());
    vec3Type->type = ASTNode::Type::DECLARATION_STRUCT;
    vec3Type->name = "vec3";
    builtinDeclarations.push_back(std::move(vec3Type));

    std::unique_ptr<ASTNode> vec4Type(new ASTNode());
    vec4Type->type = ASTNode::Type::DECLARATION_STRUCT;
    vec4Type->name = "vec4";
    builtinDeclarations.push_back(std::move(vec4Type));

    std::unique_ptr<ASTNode> stringType(new ASTNode());
    stringType->type = ASTNode::Type::DECLARATION_STRUCT;
    stringType->name = "string";
    builtinDeclarations.push_back(std::move(stringType));

    std::unique_ptr<ASTNode> samplerStateType(new ASTNode());
    samplerStateType->type = ASTNode::Type::DECLARATION_STRUCT;
    samplerStateType->name = "SamplerState";
    builtinDeclarations.push_back(std::move(samplerStateType));

    std::unique_ptr<ASTNode> texture2DType(new ASTNode());
    texture2DType->type = ASTNode::Type::DECLARATION_STRUCT;
    texture2DType->name = "Texture2D";
    builtinDeclarations.push_back(std::move(texture2DType));
}

bool ASTContext::parse(const std::vector<Token>& tokens)
{
    translationUnit.reset();

    auto iterator = tokens.cbegin();

    std::vector<std::vector<ASTNode*>> declarations;
    declarations.push_back(std::vector<ASTNode*>());

    return parseTopLevel(tokens, iterator, declarations, translationUnit);
}

bool ASTContext::parseTopLevel(const std::vector<Token>& tokens,
                               std::vector<Token>::const_iterator& iterator,
                               std::vector<std::vector<ASTNode*>>& declarations,
                               std::unique_ptr<ASTNode>& result)
{
    result.reset(new ASTNode());
    result->type = ASTNode::Type::TRANSLATION_UNIT;

    while (iterator != tokens.end())
    {
        if (checkToken(Token::Type::KEYWORD_STRUCT, tokens, iterator))
        {
            std::unique_ptr<ASTNode> decl;
            if (!parseStructDecl(tokens, iterator, declarations, decl))
            {
                std::cerr << "Failed to parse a structure declaration" << std::endl;
                return false;
            }

            result->children.push_back(std::move(decl));
        }
        else if (checkToken(Token::Type::KEYWORD_TYPEDEF, tokens, iterator))
        {
            std::unique_ptr<ASTNode> decl;
            if (!parseTypedefDecl(tokens, iterator, declarations, decl))
            {
                std::cerr << "Failed to parse a type definition declaration" << std::endl;
                return false;
            }

            result->children.push_back(std::move(decl));
        }
        else if (checkToken(Token::Type::KEYWORD_FUNCTION, tokens, iterator))
        {
            std::unique_ptr<ASTNode> decl;
            if (!parseFunctionDecl(tokens, iterator, declarations, decl))
            {
                std::cerr << "Failed to parse a function declaration" << std::endl;
                return false;
            }

            result->children.push_back(std::move(decl));
        }
        else if (checkTokens({Token::Type::KEYWORD_STATIC, Token::Type::KEYWORD_CONST, Token::Type::KEYWORD_VAR}, tokens, iterator))
        {
            std::unique_ptr<ASTNode> decl;
            if (!parseVariableDecl(tokens, iterator, declarations, decl))
            {
                std::cerr << "Failed to parse a variable declaration" << std::endl;
                return false;
            }

            result->children.push_back(std::move(decl));
        }
        else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::unique_ptr<ASTNode> decl(new ASTNode());
            decl->type = ASTNode::Type::DECLARATION_EMPTY;
            result->children.push_back(std::move(decl));
        }
        else
        {
            std::cerr << "Expected a keyword" << std::endl;
            return false;
        }
    }

    return true;
}

bool ASTContext::parseStructDecl(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::vector<ASTNode*>>& declarations,
                                 std::unique_ptr<ASTNode>& result)
{
    if (checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::DECLARATION_STRUCT;
        result->name = (iterator - 1)->value;

        if (checkToken(Token::Type::LEFT_BRACE, tokens, iterator))
        {
            for (;;)
            {
                if (checkToken(Token::Type::RIGHT_BRACE, tokens, iterator))
                {
                    if (result->children.empty())
                    {
                        std::cerr << "Structure must have at least one member" << std::endl;
                        return false;
                    }

                    declarations.back().push_back(result.get());
                    break;
                }
                else if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
                {
                    std::unique_ptr<ASTNode> field(new ASTNode());
                    field->type = ASTNode::Type::DECLARATION_FIELD;

                    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
                    {
                        std::cerr << "Expected an identifier" << std::endl;
                        return false;
                    }

                    field->name = (iterator - 1)->value;

                    if (!checkToken(Token::Type::COLON, tokens, iterator))
                    {
                        std::cerr << "Expected a colon" << std::endl;
                        return false;
                    }

                    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
                    {
                        std::cerr << "Expected a type name" << std::endl;
                        return false;
                    }

                    field->reference = findDeclaration((iterator - 1)->value, declarations);

                    if (!field->reference)
                    {
                        std::cerr << "Invalid declaration reference: " << (iterator - 1)->value << std::endl;
                        return false;
                    }

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
                                    return false;
                                }

                                if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
                                {
                                    std::cerr << "Expected an identifier" << std::endl;
                                    return false;
                                }

                                if (attribute == "semantic")
                                {
                                    ASTNode::Semantic semantic = ASTNode::Semantic::NONE;

                                    // TODO: find slot number
                                    if ((iterator - 1)->value == "binormal") semantic = ASTNode::Semantic::BINORMAL;
                                    else if ((iterator - 1)->value == "blend_indices") semantic = ASTNode::Semantic::BLEND_INDICES;
                                    else if ((iterator - 1)->value == "blend_weight") semantic = ASTNode::Semantic::BLEND_WEIGHT;
                                    else if ((iterator - 1)->value == "color") semantic = ASTNode::Semantic::COLOR;
                                    else if ((iterator - 1)->value == "normal") semantic = ASTNode::Semantic::NORMAL;
                                    else if ((iterator - 1)->value == "position") semantic = ASTNode::Semantic::POSITION;
                                    else if ((iterator - 1)->value == "position_transformed") semantic = ASTNode::Semantic::POSITION_TRANSFORMED;
                                    else if ((iterator - 1)->value == "point_size") semantic = ASTNode::Semantic::POINT_SIZE;
                                    else if ((iterator - 1)->value == "tangent") semantic = ASTNode::Semantic::TANGENT;
                                    else if ((iterator - 1)->value == "texture_coordinates") semantic = ASTNode::Semantic::TEXTURE_COORDINATES;
                                    else
                                    {
                                        std::cerr << "Invalid semantic" << std::endl;
                                        return false;
                                    }

                                    field->semantic = semantic;
                                }
                                else
                                {
                                    std::cerr << "Invalid attribute" << std::endl;
                                    return false;
                                }
                            }
                            else
                            {
                                std::cerr << "Expected an identifier" << std::endl;
                                return false;
                            }
                        }
                    }

                    if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
                    {
                        std::cerr << "Expected a semicolon" << std::endl;
                        return false;
                    }

                    result->children.push_back(std::move(field));
                }
                else
                {
                    std::cerr << "Expected an attribute" << std::endl;
                    return false;
                }
            }
        }
        else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            declarations.back().push_back(result.get());
        }
        else
        {
            std::cerr << "Expected a left brace or a semicolon" << std::endl;
            return false;
        }
    }

    return true;
}

bool ASTContext::parseTypedefDecl(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<ASTNode*>>& declarations,
                                  std::unique_ptr<ASTNode>& result)
{
    std::cerr << "Typedef is not supported" << std::endl;
    return false;

    /*if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return false;
    }

    result.reset(new ASTNode());
    result->type = ASTNode::Type::DECLARATION_TYPE_DEFINITION;
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

    return true;*/
}

bool ASTContext::parseFunctionDecl(const std::vector<Token>& tokens,
                                   std::vector<Token>::const_iterator& iterator,
                                   std::vector<std::vector<ASTNode*>>& declarations,
                                   std::unique_ptr<ASTNode>& result)
{
    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a function name" << std::endl;
        return false;
    }

    result.reset(new ASTNode());
    result->type = ASTNode::Type::DECLARATION_FUNCTION;
    result->name = (iterator - 1)->value;

    if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Unexpected end of function declaration" << std::endl;
        return false;
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

            std::unique_ptr<ASTNode> parameter(new ASTNode());
            parameter->type = ASTNode::Type::DECLARATION_PARAMETER;
            parameter->name = (iterator - 1)->value;

            if (!checkToken(Token::Type::COLON, tokens, iterator))
            {
                std::cerr << "Expected a colon" << std::endl;
                return false;
            }

            if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
            {
                std::cerr << "Expected a type name" << std::endl;
                return false;
            }

            parameter->reference = findDeclaration((iterator - 1)->value, declarations);

            if (!parameter->reference)
            {
                std::cerr << "Invalid declaration reference: " << (iterator - 1)->value << std::endl;
                return false;
            }

            result->children.push_back(std::move(parameter));
        }
        else
        {
            std::cerr << "Expected a comma, keyword or a right parenthesis" << std::endl;
            return false;
        }
    }

    if (!checkToken(Token::Type::COLON, tokens, iterator))
    {
        std::cerr << "Expected a colon" << std::endl;
        return false;
    }

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return false;
    }

    result->reference = findDeclaration((iterator - 1)->value, declarations);

    if (!result->reference)
    {
        std::cerr << "Invalid declaration reference: " << (iterator - 1)->value << std::endl;
        return false;
    }

    if (checkToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        declarations.back().push_back(result.get());

        // parse body
        std::unique_ptr<ASTNode> compound;
        if (!parseCompoundStatement(tokens, iterator, declarations, compound))
        {
            std::cerr << "Failed to parse a compound statement" << std::endl;
            return false;
        }

        result->children.push_back(std::move(compound));
    }
    else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
    {
        declarations.back().push_back(result.get());
    }
    else
    {
        std::cerr << "Expected a left brace or a semicolon" << std::endl;
        return false;
    }

    return true;
}

bool ASTContext::parseVariableDecl(const std::vector<Token>& tokens,
                                   std::vector<Token>::const_iterator& iterator,
                                   std::vector<std::vector<ASTNode*>>& declarations,
                                   std::unique_ptr<ASTNode>& result)
{
    result.reset(new ASTNode());
    result->type = ASTNode::Type::DECLARATION_VARIABLE;

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
        return false;
    }

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Unexpected end of variable declaration" << std::endl;
        return false;
    }

    result->name = (iterator - 1)->value;

    if (!checkToken(Token::Type::COLON, tokens, iterator))
    {
        std::cerr << "Expected a colon" << std::endl;
        return false;
    }

    if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return false;
    }

    result->reference = findDeclaration((iterator - 1)->value, declarations);

    if (!result->reference)
    {
        std::cerr << "Invalid declaration reference: " << (iterator - 1)->value << std::endl;
        return false;
    }

    if (checkToken(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression;
        if (!parseExpression(tokens, iterator, declarations, expression))
        {
            return false;
        }

        result->children.push_back(std::move(expression));
    }
    else if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression;
        if (!parseExpression(tokens, iterator, declarations, expression))
        {
            return false;
        }

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return false;
        }

        result->children.push_back(std::move(expression));

    }

    declarations.back().push_back(result.get());

    return true;
}

bool ASTContext::parseCompoundStatement(const std::vector<Token>& tokens,
                                        std::vector<Token>::const_iterator& iterator,
                                        std::vector<std::vector<ASTNode*>>& declarations,
                                        std::unique_ptr<ASTNode>& result)
{
    declarations.push_back(std::vector<ASTNode*>());

    result.reset(new ASTNode());
    result->type = ASTNode::Type::STATEMENT_COMPOUND;

    for (;;)
    {
        if (checkToken(Token::Type::RIGHT_BRACE, tokens, iterator))
        {
            declarations.pop_back();
            break;
        }
        else
        {
            std::unique_ptr<ASTNode> statement;
            if (!parseStatement(tokens, iterator, declarations, statement))
            {
                std::cerr << "Failed to parse a statement" << std::endl;
                return false;
            }

            result->children.push_back(std::move(statement));
        }
    }

    return true;
}

bool ASTContext::parseStatement(const std::vector<Token>& tokens,
                                std::vector<Token>::const_iterator& iterator,
                                std::vector<std::vector<ASTNode*>>& declarations,
                                std::unique_ptr<ASTNode>& result)
{
    if (checkToken(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        return parseCompoundStatement(tokens, iterator, declarations, result);
    }
    else if (checkToken(Token::Type::KEYWORD_IF, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_IF;

        if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a left parenthesis" << std::endl;
            return false;
        }

        if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
        {
            std::unique_ptr<ASTNode> declaration;
            if (!parseVariableDecl(tokens, iterator, declarations, declaration))
            {
                return false;
            }

            result->children.push_back(std::move(declaration));
        }
        else
        {
            std::unique_ptr<ASTNode> expression;
            if (!parseExpression(tokens, iterator, declarations, expression))
            {
                return false;
            }

            result->children.push_back(std::move(expression));
        }

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return false;
        }

        std::unique_ptr<ASTNode> statement;
        if (!parseStatement(tokens, iterator, declarations, statement))
        {
            return false;
        }

        result->children.push_back(std::move(statement));
    }
    else if (checkToken(Token::Type::KEYWORD_FOR, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_FOR;

        if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a left parenthesis" << std::endl;
            return false;
        }

        std::unique_ptr<ASTNode> node;

        if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
        {
            if (!parseVariableDecl(tokens, iterator, declarations, node))
            {
                return false;
            }

            if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
            {
                std::cerr << "Expected a semicolon" << std::endl;
                return false;
            }
        }
        else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            node.reset(new ASTNode());
            node->type = ASTNode::Type::NONE;
        }
        else
        {
            if (!parseExpression(tokens, iterator, declarations, node))
            {
                return false;
            }

            if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
            {
                std::cerr << "Expected a semicolon" << std::endl;
                return false;
            }
        }

        result->children.push_back(std::move(node));

        if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
        {
            if (!parseVariableDecl(tokens, iterator, declarations, node))
            {
                return false;
            }

            if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
            {
                std::cerr << "Expected a semicolon" << std::endl;
                return false;
            }
        }
        else if (checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            node.reset(new ASTNode());
            node->type = ASTNode::Type::NONE;
        }
        else
        {
            if (!parseExpression(tokens, iterator, declarations, node))
            {
                return false;
            }

            if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
            {
                std::cerr << "Expected a semicolon" << std::endl;
                return false;
            }
        }

        result->children.push_back(std::move(node));

        if (checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            node.reset(new ASTNode());
            node->type = ASTNode::Type::NONE;
        }
        else
        {
            if (!parseExpression(tokens, iterator, declarations, node))
            {
                return false;
            }

            if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
            {
                std::cerr << "Expected a right parenthesis" << std::endl;
                return false;
            }
        }

        result->children.push_back(std::move(node));

        std::unique_ptr<ASTNode> statement;
        if (!parseStatement(tokens, iterator, declarations, statement))
        {
            return false;
        }

        result->children.push_back(std::move(statement));
    }
    else if (checkToken(Token::Type::KEYWORD_SWITCH, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_SWITCH;

        if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a left parenthesis" << std::endl;
            return false;
        }

        if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
        {
            std::unique_ptr<ASTNode> declaration;
            if (!parseVariableDecl(tokens, iterator, declarations, declaration))
            {
                return false;
            }

            result->children.push_back(std::move(declaration));
        }
        else
        {
            std::unique_ptr<ASTNode> expression;
            if (!parseExpression(tokens, iterator, declarations, expression))
            {
                return false;
            }

            result->children.push_back(std::move(expression));
        }

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return false;
        }

        std::unique_ptr<ASTNode> statement;
        if (!parseStatement(tokens, iterator, declarations, statement))
        {
            return false;
        }

        result->children.push_back(std::move(statement));
    }
    else if (checkToken(Token::Type::KEYWORD_CASE, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_CASE;

        if (!checkToken(Token::Type::LITERAL_INT, tokens, iterator))
        {
            std::cerr << "Expected an integer literal" << std::endl;
            return false;
        }

        result->value = (iterator - 1)->value;

        if (!checkToken(Token::Type::COLON, tokens, iterator))
        {
            std::cerr << "Expected a colon" << std::endl;
            return false;
        }

        std::unique_ptr<ASTNode> statement;
        if (!parseStatement(tokens, iterator, declarations, statement))
        {
            return false;
        }

        result->children.push_back(std::move(statement));
    }
    else if (checkToken(Token::Type::KEYWORD_WHILE, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_WHILE;

        if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a left parenthesis" << std::endl;
            return false;
        }

        if (checkToken(Token::Type::KEYWORD_VAR, tokens, iterator))
        {
            std::unique_ptr<ASTNode> declaration;
            if (!parseVariableDecl(tokens, iterator, declarations, declaration))
            {
                return false;
            }

            result->children.push_back(std::move(declaration));
        }
        else
        {
            std::unique_ptr<ASTNode> expression;
            if (!parseExpression(tokens, iterator, declarations, expression))
            {
                return false;
            }

            result->children.push_back(std::move(expression));
        }

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return false;
        }

        std::unique_ptr<ASTNode> statement;
        if (!parseStatement(tokens, iterator, declarations, statement))
        {
            return false;
        }

        result->children.push_back(std::move(statement));
    }
    else if (checkToken(Token::Type::KEYWORD_DO, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_DO;

        std::unique_ptr<ASTNode> statement;
        if (!parseStatement(tokens, iterator, declarations, statement))
        {
            return false;
        }

        result->children.push_back(std::move(statement));

        if (!checkToken(Token::Type::KEYWORD_WHILE, tokens, iterator))
        {
            std::cerr << "Expected a \"while\" keyword" << std::endl;
            return false;
        }

        if (!checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a left parenthesis" << std::endl;
            return false;
        }

        // expression
        std::unique_ptr<ASTNode> expression;
        if (!parseExpression(tokens, iterator, declarations, expression))
        {
            return false;
        }

        result->children.push_back(std::move(expression));

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return false;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return false;
        }
    }
    else if (checkToken(Token::Type::KEYWORD_BREAK, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_BREAK;

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return false;
        }
    }
    else if (checkToken(Token::Type::KEYWORD_CONTINUE, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_CONTINUE;

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return false;
        }
    }
    else if (checkToken(Token::Type::KEYWORD_RETURN, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_RETURN;

        std::unique_ptr<ASTNode> expression;
        if (!parseExpression(tokens, iterator, declarations, expression))
        {
            std::cerr << "Expected an expression" << std::endl;
            return false;
        }

        result->children.push_back(std::move(expression));

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return false;
        }
    }
    else if (checkTokens({Token::Type::KEYWORD_STATIC, Token::Type::KEYWORD_CONST, Token::Type::KEYWORD_VAR}, tokens, iterator))
    {
        if (!parseVariableDecl(tokens, iterator, declarations, result))
        {
            return false;
        }

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return false;
        }
    }
    else
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_EXPRESSION;

        std::unique_ptr<ASTNode> expression;
        if (!parseExpression(tokens, iterator, declarations, expression))
        {
            return false;
        }

        result->children.push_back(std::move(expression));

        if (!checkToken(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return false;
        }
    }

    return true;
}

bool ASTContext::parseExpression(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::vector<ASTNode*>>& declarations,
                                 std::unique_ptr<ASTNode>& result)
{
    return parseMultiplicationAssignment(tokens, iterator, declarations, result);
}

bool ASTContext::parseMultiplicationAssignment(const std::vector<Token>& tokens,
                                               std::vector<Token>::const_iterator& iterator,
                                               std::vector<std::vector<ASTNode*>>& declarations,
                                               std::unique_ptr<ASTNode>& result)
{
    if (!parseAdditionAssignment(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (checkTokens({Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT, Token::Type::OPERATOR_DIVIDE_ASSIGNMENT}, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> right;
        if (!parseAdditionAssignment(tokens, iterator, declarations, right))
        {
            return false;
        }

        expression->children.push_back(std::move(result)); // left
        expression->children.push_back(std::move(right)); // right

        result = std::move(expression);
    }

    return true;
}

bool ASTContext::parseAdditionAssignment(const std::vector<Token>& tokens,
                                         std::vector<Token>::const_iterator& iterator,
                                         std::vector<std::vector<ASTNode*>>& declarations,
                                         std::unique_ptr<ASTNode>& result)
{
    if (!parseAssignment(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (checkTokens({Token::Type::OPERATOR_PLUS_ASSIGNMENT, Token::Type::OPERATOR_MINUS_ASSIGNMENT}, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> right;
        if (!parseAssignment(tokens, iterator, declarations, right))
        {
            return false;
        }

        expression->children.push_back(std::move(result)); // left
        expression->children.push_back(std::move(right)); // right

        result = std::move(expression);
    }

    return true;
}

bool ASTContext::parseAssignment(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::vector<ASTNode*>>& declarations,
                                 std::unique_ptr<ASTNode>& result)
{
    if (!parseTernary(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (checkToken(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> right;
        if (!parseTernary(tokens, iterator, declarations, right))
        {
            return false;
        }

        expression->children.push_back(std::move(result)); // left
        expression->children.push_back(std::move(right)); // right

        result = std::move(expression);
    }

    return true;
}

bool ASTContext::parseTernary(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<ASTNode*>>& declarations,
                              std::unique_ptr<ASTNode>& result)
{
    if (!parseEquality(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (checkToken(Token::Type::OPERATOR_CONDITIONAL, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::OPERATOR_TERNARY;
        expression->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> left;
        if (!parseTernary(tokens, iterator, declarations, left))
        {
            return false;
        }

        if (!checkToken(Token::Type::COLON, tokens, iterator))
        {
            std::cerr << "Expected a colon" << std::endl;
            return false;
        }

        std::unique_ptr<ASTNode> right;
        if (!parseTernary(tokens, iterator, declarations, right))
        {
            return false;
        }

        expression->children.push_back(std::move(result));
        expression->children.push_back(std::move(left));
        expression->children.push_back(std::move(right));

        result = std::move(expression);
    }

    return true;
}

bool ASTContext::parseEquality(const std::vector<Token>& tokens,
                               std::vector<Token>::const_iterator& iterator,
                               std::vector<std::vector<ASTNode*>>& declarations,
                               std::unique_ptr<ASTNode>& result)
{
    if (!parseGreaterThan(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (checkTokens({Token::Type::OPERATOR_EQUAL, Token::Type::OPERATOR_NOT_EQUAL}, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> right;
        if (!parseGreaterThan(tokens, iterator, declarations, right))
        {
            return false;
        }

        expression->children.push_back(std::move(result)); // left
        expression->children.push_back(std::move(right)); // right

        result = std::move(expression);
    }
    
    return true;
}

bool ASTContext::parseGreaterThan(const std::vector<Token>& tokens,
                                  std::vector<Token>::const_iterator& iterator,
                                  std::vector<std::vector<ASTNode*>>& declarations,
                                  std::unique_ptr<ASTNode>& result)
{
    if (!parseLessThan(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (checkTokens({Token::Type::OPERATOR_GREATER_THAN, Token::Type::OPERATOR_GREATER_THAN_EQUAL}, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> right;
        if (!parseLessThan(tokens, iterator, declarations, right))
        {
            return false;
        }

        expression->children.push_back(std::move(result)); // left
        expression->children.push_back(std::move(right)); // right

        result = std::move(expression);
    }
    
    return true;
}

bool ASTContext::parseLessThan(const std::vector<Token>& tokens,
                               std::vector<Token>::const_iterator& iterator,
                               std::vector<std::vector<ASTNode*>>& declarations,
                               std::unique_ptr<ASTNode>& result)
{
    if (!parseAddition(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (checkTokens({Token::Type::OPERATOR_LESS_THAN, Token::Type::OPERATOR_LESS_THAN_EQUAL}, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> right;
        if (!parseAddition(tokens, iterator, declarations, right))
        {
            return false;
        }

        expression->children.push_back(std::move(result)); // left
        expression->children.push_back(std::move(right)); // right

        result = std::move(expression);
    }
    
    return true;
}

bool ASTContext::parseAddition(const std::vector<Token>& tokens,
                               std::vector<Token>::const_iterator& iterator,
                               std::vector<std::vector<ASTNode*>>& declarations,
                               std::unique_ptr<ASTNode>& result)
{
    if (!parseMultiplication(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (checkTokens({Token::Type::OPERATOR_PLUS, Token::Type::OPERATOR_MINUS}, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> right;
        if (!parseMultiplication(tokens, iterator, declarations, right))
        {
            return false;
        }

        expression->children.push_back(std::move(result)); // left
        expression->children.push_back(std::move(right)); // right

        result = std::move(expression);
    }
    
    return true;
}

bool ASTContext::parseMultiplication(const std::vector<Token>& tokens,
                                     std::vector<Token>::const_iterator& iterator,
                                     std::vector<std::vector<ASTNode*>>& declarations,
                                     std::unique_ptr<ASTNode>& result)
{
    if (!parseUnary(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (checkTokens({Token::Type::OPERATOR_MULTIPLY, Token::Type::OPERATOR_DIVIDE}, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> right;
        if (!parseUnary(tokens, iterator, declarations, right))
        {
            return false;
        }

        expression->children.push_back(std::move(result)); // left
        expression->children.push_back(std::move(right)); // right

        result = std::move(expression);
    }

    return true;
}

bool ASTContext::parseUnary(const std::vector<Token>& tokens,
                            std::vector<Token>::const_iterator& iterator,
                            std::vector<std::vector<ASTNode*>>& declarations,
                            std::unique_ptr<ASTNode>& result)
{
    if (checkTokens({Token::Type::OPERATOR_PLUS, Token::Type::OPERATOR_MINUS, Token::Type::OPERATOR_NOT}, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::OPERATOR_UNARY;
        result->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> right;
        if (!parseMember(tokens, iterator, declarations, right))
        {
            return false;
        }

        result->children.push_back(std::move(right));
    }
    else
    {
        if (!parseMember(tokens, iterator, declarations, result))
        {
            return false;
        }
    }

    return true;
}

bool ASTContext::parseMember(const std::vector<Token>& tokens,
                             std::vector<Token>::const_iterator& iterator,
                             std::vector<std::vector<ASTNode*>>& declarations,
                             std::unique_ptr<ASTNode>& result)
{
    if (!parsePrimary(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (checkToken(Token::Type::OPERATOR_DOT, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::EXPRESSION_MEMBER;

        ASTNode* declaration = findDeclaration((iterator - 2)->value, declarations);

        if (!declaration)
        {
            std::cerr << "Invalid declaration reference: " << (iterator - 2)->value << std::endl;
            return false;
        }

        std::unique_ptr<ASTNode> declRefExpression(new ASTNode());
        declRefExpression->type = ASTNode::Type::EXPRESSION_DECLARATION_REFERENCE;
        declRefExpression->reference = declaration;

        result->children.push_back(std::move(declRefExpression));

        if (!checkToken(Token::Type::IDENTIFIER, tokens, iterator))
        {
            std::cerr << "Expected an identifier" << std::endl;
            return false;
        }

        std::unique_ptr<ASTNode> fieldRefExpression(new ASTNode());
        fieldRefExpression->type = ASTNode::Type::EXPRESSION_DECLARATION_REFERENCE;
        fieldRefExpression->reference = findField((iterator - 1)->value, declaration->reference);

        if (!fieldRefExpression->reference)
        {
            std::cerr << "Invalid member reference: " << (iterator - 1)->value << std::endl;
            return false;
        }


        expression->children.push_back(std::move(result)); // left
        expression->children.push_back(std::move(fieldRefExpression)); // right

        result = std::move(expression);
    }
    
    return true;
}

bool ASTContext::parsePrimary(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<ASTNode*>>& declarations,
                              std::unique_ptr<ASTNode>& result)
{
    if (checkToken(Token::Type::LITERAL_INT, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::EXPRESSION_LITERAL;
        result->reference = findDeclaration("int", declarations);
        result->value = (iterator - 1)->value;
    }
    else if (checkToken(Token::Type::LITERAL_FLOAT, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::EXPRESSION_LITERAL;
        result->reference = findDeclaration("float", declarations);
        result->value = (iterator - 1)->value;
    }
    else if (checkToken(Token::Type::LITERAL_STRING, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::EXPRESSION_LITERAL;
        result->reference = findDeclaration("string", declarations);
        result->value = (iterator - 1)->value;
    }
    else if (checkTokens({Token::Type::KEYWORD_TRUE, Token::Type::KEYWORD_FALSE}, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::EXPRESSION_LITERAL;
        result->reference = findDeclaration("bool", declarations);
        result->value = (iterator - 1)->value;
    }
    else if (checkToken(Token::Type::IDENTIFIER, tokens, iterator))
    {
        if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            result.reset(new ASTNode());
            result->type = ASTNode::Type::EXPRESSION_CALL;

            std::unique_ptr<ASTNode> declRefExpression(new ASTNode());
            declRefExpression->type = ASTNode::Type::EXPRESSION_DECLARATION_REFERENCE;
            declRefExpression->reference = findDeclaration((iterator - 2)->value, declarations);

            if (!declRefExpression->reference)
            {
                std::cerr << "Invalid declaration reference: " << (iterator - 2)->value << std::endl;
                return false;
            }

            result->children.push_back(std::move(declRefExpression));

            bool firstParameter = true;
            std::unique_ptr<ASTNode> parameter;

            for (;;)
            {
                if (checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
                {
                    break;
                }
                else if ((firstParameter || checkToken(Token::Type::COMMA, tokens, iterator)) &&
                         parseExpression(tokens, iterator, declarations, parameter))
                {
                    firstParameter = false;

                    result->children.push_back(std::move(parameter));
                }
                else
                {
                    std::cerr << "Expected a comma, keyword or a right parenthesis" << std::endl;
                    return false;
                }
            }
        }
        else if (checkToken(Token::Type::LEFT_BRACKET, tokens, iterator))
        {
            result.reset(new ASTNode());
            result->type = ASTNode::Type::EXPRESSION_ARRAY_SUBSCRIPT;

            std::unique_ptr<ASTNode> declRefExpression(new ASTNode());
            declRefExpression->type = ASTNode::Type::EXPRESSION_DECLARATION_REFERENCE;
            declRefExpression->reference = findDeclaration((iterator - 2)->value, declarations);

            if (!declRefExpression->reference)
            {
                std::cerr << "Invalid declaration reference: " << (iterator - 2)->value << std::endl;
                return false;
            }

            result->children.push_back(std::move(declRefExpression));

            std::unique_ptr<ASTNode> expression;

            if (!parseExpression(tokens, iterator, declarations, expression))
            {
                return false;
            }

            if (!checkToken(Token::Type::RIGHT_BRACKET, tokens, iterator))
            {
                std::cerr << "Expected a right brace" << std::endl;
                return false;
            }
            
            result->children.push_back(std::move(expression));
        }
        else
        {
            result.reset(new ASTNode());
            result->type = ASTNode::Type::EXPRESSION_DECLARATION_REFERENCE;
            result->reference = findDeclaration((iterator - 1)->value, declarations);

            if (!result->reference)
            {
                std::cerr << "Invalid declaration reference: " << (iterator - 1)->value << std::endl;
                return false;
            }
        }
    }
    else if (checkToken(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::EXPRESSION_PAREN;

        std::unique_ptr<ASTNode> expression;

        if (!parseExpression(tokens, iterator, declarations, expression))
        {
            return false;
        }

        if (!checkToken(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return false;
        }

        result->children.push_back(std::move(expression));
    }
    else
    {
        std::cerr << "Expected an expression" << std::endl;
        return false;
    }

    return true;
}

void ASTContext::dump()
{
    if (translationUnit)
    {
        ASTContext::dumpNode(translationUnit);
    }
}

void ASTContext::dumpNode(const std::unique_ptr<ASTNode>& node, std::string indent)
{
    std::cout << indent << node.get() << " " << nodeTypeToString(node->type);

    if (!node->name.empty()) std::cout << ", name: " << node->name;
    if (node->reference)
    {
        std::cout << ", reference: ";
        if (node->isStatic) std::cout << "static ";
        if (node->isConst) std::cout << "const ";
        std::cout << node->reference->name;

    }
    if (!node->value.empty()) std::cout << ", value: " << node->value;
    if (node->semantic != ASTNode::Semantic::NONE) std::cout << ", semantic: " << semanticToString(node->semantic);

    std::cout << std::endl;

    for (const std::unique_ptr<ASTNode>& child : node->children)
    {
        ASTContext::dumpNode(child, indent + "  ");
    }
}
