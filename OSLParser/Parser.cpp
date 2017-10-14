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
    auto iterator = tokens.begin();

    std::vector<std::vector<ASTNode*>> declarations;
    declarations.push_back(std::vector<ASTNode*>());

    if (std::unique_ptr<ASTNode> node = parseTopLevel(tokens, iterator, declarations))
    {
        translationUnit = std::move(node);
        return true;
    }
    else
    {
        return false;
    }
}

std::unique_ptr<ASTNode> ASTContext::parseTopLevel(const std::vector<Token>& tokens,
                                                   std::vector<Token>::const_iterator& iterator,
                                                   std::vector<std::vector<ASTNode*>>& declarations)
{
    std::unique_ptr<ASTNode> result(new ASTNode());
    result->type = ASTNode::Type::TRANSLATION_UNIT;

    while (iterator != tokens.end())
    {
        if (check(Token::Type::KEYWORD_STRUCT, tokens, iterator))
        {
            if (std::unique_ptr<ASTNode> decl = parseStructDecl(tokens, iterator, declarations))
            {
                result->children.push_back(std::move(decl));
            }
            else
            {
                std::cerr << "Failed to parse a structure declaration" << std::endl;
                return nullptr;
            }
        }
        else if (check(Token::Type::KEYWORD_TYPEDEF, tokens, iterator))
        {
            if (std::unique_ptr<ASTNode> decl = parseTypedefDecl(tokens, iterator, declarations))
            {
                result->children.push_back(std::move(decl));
            }
            else
            {
                std::cerr << "Failed to parse a type definition declaration" << std::endl;
                return nullptr;
            }
        }
        else if (check(Token::Type::KEYWORD_FUNCTION, tokens, iterator))
        {
            if (std::unique_ptr<ASTNode> decl = parseFunctionDecl(tokens, iterator, declarations))
            {
                result->children.push_back(std::move(decl));
            }
            else
            {
                std::cerr << "Failed to parse a function declaration" << std::endl;
                return nullptr;
            }
        }
        else if (check({Token::Type::KEYWORD_STATIC, Token::Type::KEYWORD_CONST, Token::Type::KEYWORD_VAR}, tokens, iterator))
        {
            if (std::unique_ptr<ASTNode> decl = parseVariableDecl(tokens, iterator, declarations))
            {
                result->children.push_back(std::move(decl));
            }
            else
            {
                std::cerr << "Failed to parse a variable declaration" << std::endl;
                return nullptr;
            }
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
            return nullptr;
        }
    }

    return result;
}

std::unique_ptr<ASTNode> ASTContext::parseStructDecl(const std::vector<Token>& tokens,
                                                     std::vector<Token>::const_iterator& iterator,
                                                     std::vector<std::vector<ASTNode*>>& declarations)
{
    if (check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::DECLARATION_STRUCT;
        result->name = (iterator - 1)->value;

        if (check(Token::Type::LEFT_BRACE, tokens, iterator))
        {
            for (;;)
            {
                if (check(Token::Type::RIGHT_BRACE, tokens, iterator))
                {
                    if (!result->children.empty())
                    {
                        declarations.back().push_back(result.get());
                        return result;
                    }
                    else
                    {
                        std::cerr << "Structure must have at least one member" << std::endl;
                        return nullptr;
                    }
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

                                if (check(Token::Type::OPERATOR_ASSIGNMENT, tokens, iterator))
                                {
                                    if (check(Token::Type::IDENTIFIER, tokens, iterator))
                                    {
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
                                else
                                {
                                    std::cerr << "Expected an equality sign" << std::endl;
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

                    if (check(Token::Type::IDENTIFIER, tokens, iterator))
                    {
                        field->name = (iterator - 1)->value;

                        if (check(Token::Type::COLON, tokens, iterator))
                        {
                            if (check(Token::Type::IDENTIFIER, tokens, iterator))
                            {
                                field->typeName = (iterator - 1)->value;

                                if (check(Token::Type::SEMICOLON, tokens, iterator))
                                {
                                    result->children.push_back(std::move(field));
                                }
                                else
                                {
                                    std::cerr << "Expected a semicolon" << std::endl;
                                    return nullptr;
                                }
                            }
                            else
                            {
                                std::cerr << "Expected a type name" << std::endl;
                                return nullptr;
                            }
                        }
                        else
                        {
                            std::cerr << "Expected a colon" << std::endl;
                            return nullptr;
                        }
                    }
                    else
                    {
                        std::cerr << "Expected an identifier" << std::endl;
                        return nullptr;
                    }
                }
                else
                {
                    std::cerr << "Expected a type name" << std::endl;
                    return nullptr;
                }
            }
        }
        else if (check(Token::Type::SEMICOLON, tokens, iterator))
        {
            declarations.back().push_back(result.get());
            return result;
        }
        else
        {
            std::cerr << "Expected a left brace or a semicolon" << std::endl;
            return nullptr;
        }
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseTypedefDecl(const std::vector<Token>& tokens,
                                                      std::vector<Token>::const_iterator& iterator,
                                                      std::vector<std::vector<ASTNode*>>& declarations)
{
    if (check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::DECLARATION_TYPE_DEFINITION;
        result->typeName = (iterator - 1)->value;

        if (check(Token::Type::IDENTIFIER, tokens, iterator))
        {
            result->name = (iterator - 1)->value;

            if (check(Token::Type::SEMICOLON, tokens, iterator))
            {
                return result;
            }
            else
            {
                std::cerr << "Expected a semicolon" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cerr << "Expected a type name" << std::endl;
            return nullptr;
        }
    }
    else
    {
        std::cerr << "Expected a type name" << std::endl;
        return nullptr;
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseFunctionDecl(const std::vector<Token>& tokens,
                                                       std::vector<Token>::const_iterator& iterator,
                                                       std::vector<std::vector<ASTNode*>>& declarations)
{
    if (check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::DECLARATION_FUNCTION;
        result->name = (iterator - 1)->value;

        if (check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
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

                    if (check(Token::Type::COLON, tokens, iterator))
                    {
                        if (check(Token::Type::IDENTIFIER, tokens, iterator))
                        {
                            parameter->typeName = (iterator - 1)->value;
                            result->children.push_back(std::move(parameter));
                        }
                        else
                        {
                            std::cerr << "Expected a type name" << std::endl;
                            return nullptr;
                        }
                    }
                    else
                    {
                        std::cerr << "Expected a colon" << std::endl;
                        return nullptr;
                    }
                }
                else
                {
                    std::cerr << "Expected a comma, keyword or a right parenthesis" << std::endl;
                    return nullptr;
                }
            }

            if (check(Token::Type::COLON, tokens, iterator))
            {
                if (check(Token::Type::IDENTIFIER, tokens, iterator))
                {
                    result->typeName = (iterator - 1)->value;
                }
                else
                {
                    std::cerr << "Expected a type name" << std::endl;
                    return nullptr;
                }
            }
            else
            {
                std::cerr << "Expected a colon" << std::endl;
                return nullptr;
            }

            if (check(Token::Type::LEFT_BRACE, tokens, iterator))
            {
                declarations.back().push_back(result.get());

                // parse body
                if (std::unique_ptr<ASTNode> compound = parseCompoundStatement(tokens, iterator, declarations))
                {
                    result->children.push_back(std::move(compound));
                    return result;
                }
                else
                {
                    std::cerr << "Failed to parse a compound statement" << std::endl;
                    return nullptr;
                }
            }
            else if (check(Token::Type::SEMICOLON, tokens, iterator))
            {
                declarations.back().push_back(result.get());
                return result;
            }
            else
            {
                std::cerr << "Expected a left brace or a semicolon" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cerr << "Unexpected end of function declaration" << std::endl;
            return nullptr;
        }
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseVariableDecl(const std::vector<Token>& tokens,
                                                       std::vector<Token>::const_iterator& iterator,
                                                       std::vector<std::vector<ASTNode*>>& declarations)
{
    std::unique_ptr<ASTNode> result(new ASTNode());
    result->type = ASTNode::Type::DECLARATION_VARIABLE;

    --iterator;

    for (;;)
    {
        if (check(Token::Type::KEYWORD_STATIC, tokens, iterator))
        {
            result->isStatic = true;
        }
        else if (check(Token::Type::KEYWORD_CONST, tokens, iterator))
        {
            result->isConst = true;
        }
        else if (check(Token::Type::KEYWORD_VAR, tokens, iterator))
        {
            result->isConst = false;
        }
        else
        {
            break;
        }
    }

    if (check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        result->name = (iterator - 1)->value;

        if (check(Token::Type::COLON, tokens, iterator))
        {
            if (check(Token::Type::IDENTIFIER, tokens, iterator))
            {
                result->typeName = (iterator - 1)->value;

                if (check(Token::Type::SEMICOLON, tokens, iterator))
                {
                    return result;
                }
                else
                {
                    std::cerr << "Expected a semicolon" << std::endl;
                    return nullptr;
                }
            }
            else
            {
                std::cerr << "Expected a type name" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cerr << "Expected a colon" << std::endl;
            return nullptr;
        }
    }
    else
    {
        std::cerr << "Unexpected end of variable declaration" << std::endl;
        return nullptr;
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseCompoundStatement(const std::vector<Token>& tokens,
                                                            std::vector<Token>::const_iterator& iterator,
                                                            std::vector<std::vector<ASTNode*>>& declarations)
{
    declarations.push_back(std::vector<ASTNode*>());

    std::unique_ptr<ASTNode> result(new ASTNode());
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
            if (std::unique_ptr<ASTNode> statement = parseStatement(tokens, iterator, declarations))
            {
                result->children.push_back(std::move(statement));
            }
            else
            {
                std::cerr << "Failed to parse a statement" << std::endl;
                return nullptr;
            }
        }
    }

    return result;
}

std::unique_ptr<ASTNode> ASTContext::parseStatement(const std::vector<Token>& tokens,
                                                    std::vector<Token>::const_iterator& iterator,
                                                    std::vector<std::vector<ASTNode*>>& declarations)
{
    if (check(Token::Type::LEFT_BRACE, tokens, iterator))
    {
        return parseCompoundStatement(tokens, iterator, declarations);
    }
    else if (check(Token::Type::KEYWORD_IF, tokens, iterator))
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_IF;

        if (check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            // TODO: parse expression
        }
        else
        {
            std::cerr << "Expected a left parenthesis" << std::endl;
            return nullptr;
        }

        return result;
    }
    else if (check(Token::Type::KEYWORD_FOR, tokens, iterator))
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_FOR;

        if (check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            // TODO: parse expression
        }
        else
        {
            std::cerr << "Expected a left parenthesis" << std::endl;
            return nullptr;
        }

        return result;
    }
    else if (check(Token::Type::KEYWORD_SWITCH, tokens, iterator))
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_SWITCH;

        if (check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            // TODO: parse expression
        }
        else
        {
            std::cerr << "Expected a left parenthesis" << std::endl;
            return nullptr;
        }

        return result;
    }
    else if (check(Token::Type::KEYWORD_CASE, tokens, iterator))
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_CASE;

        return result;
    }
    else if (check(Token::Type::KEYWORD_WHILE, tokens, iterator))
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_WHILE;

        if (check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            // TODO: parse expression
        }
        else
        {
            std::cerr << "Expected a left parenthesis" << std::endl;
            return nullptr;
        }

        return result;
    }
    else if (check(Token::Type::KEYWORD_DO, tokens, iterator))
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_DO;

        return result;
    }
    else if (check(Token::Type::KEYWORD_BREAK, tokens, iterator))
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_BREAK;

        return result;
    }
    else if (check(Token::Type::KEYWORD_CONTINUE, tokens, iterator))
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_CONTINUE;

        return result;
    }
    else if (check(Token::Type::KEYWORD_RETURN, tokens, iterator))
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_RETURN;
        
        return result;
    }
    else if (check({Token::Type::KEYWORD_STATIC, Token::Type::KEYWORD_CONST, Token::Type::KEYWORD_VAR}, tokens, iterator))
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::DECLARATION_VARIABLE;

        --iterator;

        for (;;)
        {
            if (check(Token::Type::KEYWORD_STATIC, tokens, iterator))
            {
                result->isStatic = true;
            }
            else if (check(Token::Type::KEYWORD_CONST, tokens, iterator))
            {
                result->isConst = true;
            }
            else if (check(Token::Type::KEYWORD_VAR, tokens, iterator))
            {
                result->isConst = false;
            }
            else
            {
                break;
            }
        }

        if (check(Token::Type::IDENTIFIER, tokens, iterator))
        {
            result->name = (iterator - 1)->value;

            if (check(Token::Type::COLON, tokens, iterator))
            {
                if (check(Token::Type::IDENTIFIER, tokens, iterator))
                {
                    result->typeName = (iterator - 1)->value;

                    if (check(Token::Type::SEMICOLON, tokens, iterator))
                    {
                        return result;
                    }
                    else
                    {
                        std::cerr << "Expected a semicolon" << std::endl;
                        return nullptr;
                    }
                }
                else
                {
                    std::cerr << "Expected a type name" << std::endl;
                    return nullptr;
                }
            }
            else
            {
                std::cerr << "Expected a colon" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cerr << "Unexpected end of variable declaration" << std::endl;
            return nullptr;
        }

        return result;
    }
    else
    {
        // TODO: parse expression
    }

    return nullptr;
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
    if (node->semantic != ASTNode::Semantic::NONE) std::cout << ", semantic: " << semanticToString(node->semantic);

    std::cout << std::endl;

    for (const std::unique_ptr<ASTNode>& child : node->children)
    {
        ASTContext::dumpNode(child, indent + "  ");
    }
}
