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

bool ASTContext::parse(const std::vector<Token>& tokens)
{
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
        if (check(Token::Type::KEYWORD_STRUCT, tokens, iterator))
        {
            std::unique_ptr<ASTNode> decl;
            if (!parseStructDecl(tokens, iterator, declarations, decl))
            {
                std::cerr << "Failed to parse a structure declaration" << std::endl;
                return false;
            }

            result->children.push_back(std::move(decl));
        }
        else if (check(Token::Type::KEYWORD_TYPEDEF, tokens, iterator))
        {
            std::unique_ptr<ASTNode> decl;
            if (!parseTypedefDecl(tokens, iterator, declarations, decl))
            {
                std::cerr << "Failed to parse a type definition declaration" << std::endl;
                return false;
            }

            result->children.push_back(std::move(decl));
        }
        else if (check(Token::Type::KEYWORD_FUNCTION, tokens, iterator))
        {
            std::unique_ptr<ASTNode> decl;
            if (!parseFunctionDecl(tokens, iterator, declarations, decl))
            {
                std::cerr << "Failed to parse a function declaration" << std::endl;
                return false;
            }

            result->children.push_back(std::move(decl));
        }
        else if (check({Token::Type::KEYWORD_STATIC, Token::Type::KEYWORD_CONST, Token::Type::KEYWORD_VAR}, tokens, iterator))
        {
            std::unique_ptr<ASTNode> decl;
            if (!parseVariableDecl(tokens, iterator, declarations, decl))
            {
                std::cerr << "Failed to parse a variable declaration" << std::endl;
                return false;
            }

            result->children.push_back(std::move(decl));
        }
        else if (check(Token::Type::SEMICOLON, tokens, iterator))
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
    if (check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::DECLARATION_STRUCT;
        result->name = (iterator - 1)->value;

        if (check(Token::Type::LEFT_BRACE, tokens, iterator))
        {
            for (;;)
            {
                if (check(Token::Type::RIGHT_BRACE, tokens, iterator))
                {
                    if (result->children.empty())
                    {
                        std::cerr << "Structure must have at least one member" << std::endl;
                        return false;
                    }

                    declarations.back().push_back(result.get());
                    break;
                }
                else if (check(Token::Type::KEYWORD_VAR, tokens, iterator))
                {
                    std::unique_ptr<ASTNode> field(new ASTNode());
                    field->type = ASTNode::Type::DECLARATION_FIELD;

                    if (check(Token::Type::LEFT_BRACKET, tokens, iterator)) // parse attributes
                    {
                        bool firstAttribute = true;

                        for (;;)
                        {
                            if (check(Token::Type::RIGHT_BRACKET, tokens, iterator))
                            {
                                break;
                            }
                            else if ((firstAttribute || check(Token::Type::COMMA, tokens, iterator)) &&
                                     check(Token::Type::IDENTIFIER, tokens, iterator))
                            {
                                firstAttribute = false;

                                std::string attribute = (iterator - 1)->value;

                                if (!check(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
                                {
                                    std::cerr << "Expected an equality sign" << std::endl;
                                    return false;
                                }

                                if (!check(Token::Type::IDENTIFIER, tokens, iterator))
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

                    if (!check(Token::Type::IDENTIFIER, tokens, iterator))
                    {
                        std::cerr << "Expected an identifier" << std::endl;
                        return false;
                    }

                    field->name = (iterator - 1)->value;

                    if (!check(Token::Type::COLON, tokens, iterator))
                    {
                        std::cerr << "Expected a colon" << std::endl;
                        return false;
                    }

                    if (!check(Token::Type::IDENTIFIER, tokens, iterator))
                    {
                        std::cerr << "Expected a type name" << std::endl;
                        return false;
                    }

                    field->typeName = (iterator - 1)->value;

                    if (!check(Token::Type::SEMICOLON, tokens, iterator))
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
        else if (check(Token::Type::SEMICOLON, tokens, iterator))
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
    if (!check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return false;
    }

    result.reset(new ASTNode());
    result->type = ASTNode::Type::DECLARATION_TYPE_DEFINITION;
    result->typeName = (iterator - 1)->value;

    if (!check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return false;
    }

    result->name = (iterator - 1)->value;

    if (!check(Token::Type::SEMICOLON, tokens, iterator))
    {
        std::cerr << "Expected a semicolon" << std::endl;
        return false;
    }

    return true;
}

bool ASTContext::parseFunctionDecl(const std::vector<Token>& tokens,
                                   std::vector<Token>::const_iterator& iterator,
                                   std::vector<std::vector<ASTNode*>>& declarations,
                                   std::unique_ptr<ASTNode>& result)
{
    if (!check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a function name" << std::endl;
        return false;
    }

    result.reset(new ASTNode());
    result->type = ASTNode::Type::DECLARATION_FUNCTION;
    result->name = (iterator - 1)->value;

    if (!check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::cerr << "Unexpected end of function declaration" << std::endl;
        return false;
    }

    bool firstParameter = true;

    for (;;)
    {
        if (check(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            break;
        }
        else if ((firstParameter || check(Token::Type::COMMA, tokens, iterator)) &&
                 check(Token::Type::IDENTIFIER, tokens, iterator))
        {
            firstParameter = false;

            std::unique_ptr<ASTNode> parameter(new ASTNode());
            parameter->type = ASTNode::Type::DECLARATION_PARAMETER;
            parameter->name = (iterator - 1)->value;

            if (!check(Token::Type::COLON, tokens, iterator))
            {
                std::cerr << "Expected a colon" << std::endl;
                return false;
            }

            if (!check(Token::Type::IDENTIFIER, tokens, iterator))
            {
                std::cerr << "Expected a type name" << std::endl;
                return false;
            }

            parameter->typeName = (iterator - 1)->value;
            result->children.push_back(std::move(parameter));
        }
        else
        {
            std::cerr << "Expected a comma, keyword or a right parenthesis" << std::endl;
            return false;
        }
    }

    if (!check(Token::Type::COLON, tokens, iterator))
    {
        std::cerr << "Expected a colon" << std::endl;
        return false;
    }

    if (!check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return false;
    }

    result->typeName = (iterator - 1)->value;

    if (check(Token::Type::LEFT_BRACE, tokens, iterator))
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
    else if (check(Token::Type::SEMICOLON, tokens, iterator))
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

    if (check(Token::Type::KEYWORD_CONST, tokens, iterator))
    {
        result->isConst = true;
    }
    else if (check(Token::Type::KEYWORD_VAR, tokens, iterator))
    {
        result->isConst = false;
    }
    else
    {
        std::cerr << "Expected const or var" << std::endl;
        return false;
    }

    if (!check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Unexpected end of variable declaration" << std::endl;
        return false;
    }

    result->name = (iterator - 1)->value;

    if (!check(Token::Type::COLON, tokens, iterator))
    {
        std::cerr << "Expected a colon" << std::endl;
        return false;
    }

    if (!check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::cerr << "Expected a type name" << std::endl;
        return false;
    }

    result->typeName = (iterator - 1)->value;

    if (check(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression;
        if (!parseExpression(tokens, iterator, declarations, expression))
        {
            return false;
        }

        result->children.push_back(std::move(expression));
    }
    else if (check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression;
        if (!parseExpression(tokens, iterator, declarations, expression))
        {
            return false;
        }

        if (!check(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a right parenthesis" << std::endl;
            return false;
        }

        result->children.push_back(std::move(expression));

    }

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
        if (check(Token::Type::RIGHT_BRACE, tokens, iterator))
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
    if (check(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        return parseCompoundStatement(tokens, iterator, declarations, result);
    }
    else if (check(Token::Type::KEYWORD_IF, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_IF;

        if (!check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a left parenthesis" << std::endl;
            return false;
        }

        if (check(Token::Type::KEYWORD_VAR, tokens, iterator))
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

        if (!check(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
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
    else if (check(Token::Type::KEYWORD_FOR, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_FOR;

        if (!check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a left parenthesis" << std::endl;
            return false;
        }

        std::unique_ptr<ASTNode> node;

        if (check(Token::Type::KEYWORD_VAR, tokens, iterator))
        {
            if (!parseVariableDecl(tokens, iterator, declarations, node))
            {
                return false;
            }

            if (!check(Token::Type::SEMICOLON, tokens, iterator))
            {
                std::cerr << "Expected a semicolon" << std::endl;
                return false;
            }
        }
        else if (check(Token::Type::SEMICOLON, tokens, iterator))
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

            if (!check(Token::Type::SEMICOLON, tokens, iterator))
            {
                std::cerr << "Expected a semicolon" << std::endl;
                return false;
            }
        }

        result->children.push_back(std::move(node));

        if (check(Token::Type::KEYWORD_VAR, tokens, iterator))
        {
            if (!parseVariableDecl(tokens, iterator, declarations, node))
            {
                return false;
            }

            if (!check(Token::Type::SEMICOLON, tokens, iterator))
            {
                std::cerr << "Expected a semicolon" << std::endl;
                return false;
            }
        }
        else if (check(Token::Type::SEMICOLON, tokens, iterator))
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

            if (!check(Token::Type::SEMICOLON, tokens, iterator))
            {
                std::cerr << "Expected a semicolon" << std::endl;
                return false;
            }
        }

        result->children.push_back(std::move(node));

        if (check(Token::Type::KEYWORD_VAR, tokens, iterator))
        {
            if (!parseVariableDecl(tokens, iterator, declarations, node))
            {
                return false;
            }

            if (!check(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
            {
                std::cerr << "Expected a right parenthesis" << std::endl;
                return false;
            }
        }
        else if (check(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
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

            if (!check(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
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
    else if (check(Token::Type::KEYWORD_SWITCH, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_SWITCH;

        if (!check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a left parenthesis" << std::endl;
            return false;
        }

        if (check(Token::Type::KEYWORD_VAR, tokens, iterator))
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

        if (!check(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
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
    else if (check(Token::Type::KEYWORD_CASE, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_CASE;

        if (!check(Token::Type::LITERAL_INT, tokens, iterator))
        {
            std::cerr << "Expected an integer literal" << std::endl;
            return false;
        }

        result->value = (iterator - 1)->value;

        if (!check(Token::Type::COLON, tokens, iterator))
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
    else if (check(Token::Type::KEYWORD_WHILE, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_WHILE;

        if (!check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            std::cerr << "Expected a left parenthesis" << std::endl;
            return false;
        }

        if (check(Token::Type::KEYWORD_VAR, tokens, iterator))
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

        if (!check(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
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
    else if (check(Token::Type::KEYWORD_DO, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_DO;
    }
    else if (check(Token::Type::KEYWORD_BREAK, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_BREAK;

        if (!check(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return false;
        }
    }
    else if (check(Token::Type::KEYWORD_CONTINUE, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_CONTINUE;

        if (!check(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return false;
        }
    }
    else if (check(Token::Type::KEYWORD_RETURN, tokens, iterator))
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

        if (!check(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return false;
        }
    }
    else if (check({Token::Type::KEYWORD_STATIC, Token::Type::KEYWORD_CONST, Token::Type::KEYWORD_VAR}, tokens, iterator))
    {
        if (!parseVariableDecl(tokens, iterator, declarations, result))
        {
            return false;
        }

        if (!check(Token::Type::SEMICOLON, tokens, iterator))
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

        if (!check(Token::Type::SEMICOLON, tokens, iterator))
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
    return parseAssignment(tokens, iterator, declarations, result);
}

bool ASTContext::parseAssignment(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::vector<ASTNode*>>& declarations,
                                 std::unique_ptr<ASTNode>& result)
{
    if (!parseEquality(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (check(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> right;
        if (!parseEquality(tokens, iterator, declarations, right))
        {
            return false;
        }

        expression->children.push_back(std::move(result)); // left
        expression->children.push_back(std::move(right)); // right

        result = std::move(expression);
    }
    
    return true;
}

bool ASTContext::parseEquality(const std::vector<Token>& tokens,
                               std::vector<Token>::const_iterator& iterator,
                               std::vector<std::vector<ASTNode*>>& declarations,
                               std::unique_ptr<ASTNode>& result)
{
    if (!parseComparison(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (check({Token::Type::OPERATOR_EQUAL, Token::Type::OPERATOR_NOT_EQUAL}, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> right;
        if (!parseComparison(tokens, iterator, declarations, right))
        {
            return false;
        }

        expression->children.push_back(std::move(result)); // left
        expression->children.push_back(std::move(right)); // right

        result = std::move(expression);
    }
    
    return true;
}

bool ASTContext::parseComparison(const std::vector<Token>& tokens,
                                 std::vector<Token>::const_iterator& iterator,
                                 std::vector<std::vector<ASTNode*>>& declarations,
                                 std::unique_ptr<ASTNode>& result)
{
    if (!parseAddition(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (check({Token::Type::OPERATOR_GREATER_THAN, Token::Type::OPERATOR_GREATER_THAN_EQUAL, Token::Type::OPERATOR_LESS_THAN, Token::Type::OPERATOR_LESS_THAN_EQUAL}, tokens, iterator))
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

    while (check({Token::Type::OPERATOR_PLUS, Token::Type::OPERATOR_MINUS}, tokens, iterator))
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
    if (!parseMember(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (check({Token::Type::OPERATOR_MULTIPLY, Token::Type::OPERATOR_DIVIDE}, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::OPERATOR_BINARY;
        expression->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> right;
        if (!parseMember(tokens, iterator, declarations, right))
        {
            return false;
        }

        expression->children.push_back(std::move(result)); // left
        expression->children.push_back(std::move(right)); // right

        result = std::move(expression);
    }

    return true;
}

bool ASTContext::parseMember(const std::vector<Token>& tokens,
                             std::vector<Token>::const_iterator& iterator,
                             std::vector<std::vector<ASTNode*>>& declarations,
                             std::unique_ptr<ASTNode>& result)
{
    if (!parseUnary(tokens, iterator, declarations, result))
    {
        return false;
    }

    while (check(Token::Type::OPERATOR_DOT, tokens, iterator))
    {
        std::unique_ptr<ASTNode> expression(new ASTNode());
        expression->type = ASTNode::Type::EXPRESSION_MEMBER;
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
    if (check({Token::Type::OPERATOR_PLUS, Token::Type::OPERATOR_MINUS, Token::Type::OPERATOR_NOT}, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::OPERATOR_UNARY;
        result->value = (iterator - 1)->value;

        std::unique_ptr<ASTNode> right;
        if (!parseUnary(tokens, iterator, declarations, right))
        {
            return false;
        }

        result->children.push_back(std::move(right));
    }
    else
    {
        if (!parsePrimary(tokens, iterator, declarations, result))
        {
            return false;
        }
    }

    return true;
}

bool ASTContext::parsePrimary(const std::vector<Token>& tokens,
                              std::vector<Token>::const_iterator& iterator,
                              std::vector<std::vector<ASTNode*>>& declarations,
                              std::unique_ptr<ASTNode>& result)
{
    if (check(Token::Type::LITERAL_INT, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::EXPRESSION_LITERAL;
        result->typeName = "int";
        result->value = (iterator - 1)->value;
    }
    else if (check(Token::Type::LITERAL_FLOAT, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::EXPRESSION_LITERAL;
        result->typeName = "float";
        result->value = (iterator - 1)->value;
    }
    else if (check(Token::Type::LITERAL_STRING, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::EXPRESSION_LITERAL;
        result->typeName = "string";
        result->value = (iterator - 1)->value;
    }
    else if (check({Token::Type::KEYWORD_TRUE, Token::Type::KEYWORD_FALSE}, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::EXPRESSION_LITERAL;
        result->typeName = "bool";
        result->value = (iterator - 1)->value;
    }
    else if (check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        if (check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            result.reset(new ASTNode());
            result->type = ASTNode::Type::EXPRESSION_CALL;
            result->name = (iterator - 2)->value;

            bool firstParameter = true;
            std::unique_ptr<ASTNode> parameter;

            for (;;)
            {
                if (check(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
                {
                    break;
                }
                else if ((firstParameter || check(Token::Type::COMMA, tokens, iterator)) &&
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
        else if (check(Token::Type::LEFT_BRACKET, tokens, iterator))
        {
            result.reset(new ASTNode());
            result->type = ASTNode::Type::EXPRESSION_ARRAY_SUBSCRIPT;
            result->name = (iterator - 2)->value;

            std::unique_ptr<ASTNode> expression;

            if (!parseExpression(tokens, iterator, declarations, expression))
            {
                return false;
            }

            if (!check(Token::Type::RIGHT_BRACKET, tokens, iterator))
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
            result->name = (iterator - 1)->value;
        }
    }
    else if (check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
    {
        result.reset(new ASTNode());
        result->type = ASTNode::Type::EXPRESSION_PAREN;

        std::unique_ptr<ASTNode> expression;

        if (!parseExpression(tokens, iterator, declarations, expression))
        {
            return false;
        }

        if (!check(Token::Type::RIGHT_PARENTHESIS, tokens, iterator))
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
    if (!node->typeName.empty())
    {
        std::cout << ", type: ";
        if (node->isStatic) std::cout << "static ";
        if (node->isConst) std::cout << "const ";
        std::cout << node->typeName;

    }
    if (!node->value.empty()) std::cout << ", value: " << node->value;
    if (node->semantic != ASTNode::Semantic::NONE) std::cout << ", semantic: " << semanticToString(node->semantic);

    std::cout << std::endl;

    for (const std::unique_ptr<ASTNode>& child : node->children)
    {
        ASTContext::dumpNode(child, indent + "  ");
    }
}
