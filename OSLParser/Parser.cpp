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
        else if (check(Token::Type::SEMICOLON, tokens, iterator))
        {
            std::unique_ptr<ASTNode> decl(new ASTNode());
            decl->type = ASTNode::Type::DECLARATION_EMPTY;
            result->children.push_back(std::move(decl));
        }
        else
        {
            if (std::unique_ptr<ASTNode> decl = parseDecl(tokens, iterator, declarations))
            {
                result->children.push_back(std::move(decl));
            }
            else
            {
                std::cerr << "Failed to parse a declaration" << std::endl;
                return nullptr;
            }
        }
    }

    return result;
}

std::unique_ptr<ASTNode> ASTContext::parseDecl(const std::vector<Token>& tokens,
                                               std::vector<Token>::const_iterator& iterator,
                                               std::vector<std::vector<ASTNode*>>& declarations)
{
    std::unique_ptr<ASTNode> result(new ASTNode());

    for (;;)
    {
        if (check(Token::Type::KEYWORD_INLINE, tokens, iterator))
        {
            result->isInline = true;
        }
        else if (check(Token::Type::KEYWORD_CONST, tokens, iterator))
        {
            result->isConst = true;
        }
        else if (check(Token::Type::KEYWORD_EXTERN, tokens, iterator))
        {
            result->isExtern = true;
        }
        else
        {
            break;
        }
    }

    if (check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        result->typeName = (iterator - 1)->value;

        if (check(Token::Type::IDENTIFIER, tokens, iterator))
        {
            result->name = (iterator - 1)->value;

            if (check(Token::Type::SEMICOLON, tokens, iterator))
            {
                declarations.back().push_back(result.get());
                result->type = ASTNode::Type::DECLARATION_VARIABLE;
                return result;
            }
            else if (check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
            {
                result->type = ASTNode::Type::DECLARATION_FUNCTION;

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
                        parameter->typeName = (iterator - 1)->value;

                        if (check(Token::Type::IDENTIFIER, tokens, iterator))
                        {
                            parameter->name = (iterator - 1)->value;
                            result->children.push_back(std::move(parameter));
                        }
                        else
                        {
                            std::cerr << "Expected an identifier" << std::endl;
                            return nullptr;
                        }
                    }
                    else
                    {
                        std::cerr << "Expected a comma, keyword or a right parenthesis" << std::endl;
                        return nullptr;
                    }
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
        else
        {
            std::cerr << "Expected a name" << std::endl;
            return nullptr;
        }
    }
    else
    {
        std::cerr << "Expected a type name" << std::endl;
        return nullptr;
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
                    if (check(Token::Type::SEMICOLON, tokens, iterator))
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
                    else
                    {
                        std::cerr << "Expected a semicolon" << std::endl;
                        return nullptr;
                    }
                }
                else if (check(Token::Type::IDENTIFIER, tokens, iterator))
                {
                    std::unique_ptr<ASTNode> field(new ASTNode());
                    field->type = ASTNode::Type::DECLARATION_FIELD;

                    field->typeName = (iterator - 1)->value;

                    if (check(Token::Type::LEFT_BRACKET, tokens, iterator)) // parse attributes
                    {
                        if (check(Token::Type::LEFT_BRACKET, tokens, iterator))
                        {
                            if (check(Token::Type::IDENTIFIER, tokens, iterator))
                            {
                                ASTNode::Attribute attribute = ASTNode::Attribute::NONE;

                                // TODO: find slot number
                                if ((iterator - 1)->value == "binormal") attribute = ASTNode::Attribute::BINORMAL;
                                else if ((iterator - 1)->value == "blend_indices") attribute = ASTNode::Attribute::BLEND_INDICES;
                                else if ((iterator - 1)->value == "blend_weight") attribute = ASTNode::Attribute::BLEND_WEIGHT;
                                else if ((iterator - 1)->value == "color") attribute = ASTNode::Attribute::COLOR;
                                else if ((iterator - 1)->value == "normal") attribute = ASTNode::Attribute::NORMAL;
                                else if ((iterator - 1)->value == "position") attribute = ASTNode::Attribute::POSITION;
                                else if ((iterator - 1)->value == "position_transformed") attribute = ASTNode::Attribute::POSITION_TRANSFORMED;
                                else if ((iterator - 1)->value == "point_size") attribute = ASTNode::Attribute::POINT_SIZE;
                                else if ((iterator - 1)->value == "tangent") attribute = ASTNode::Attribute::TANGENT;
                                else if ((iterator - 1)->value == "texture_coordinates") attribute = ASTNode::Attribute::TEXTURE_COORDINATES;
                                else
                                {
                                    std::cerr << "Invalid semantic" << std::endl;
                                    return nullptr;
                                }

                                if (check(Token::Type::RIGHT_BRACKET, tokens, iterator))
                                {
                                    if (check(Token::Type::RIGHT_BRACKET, tokens, iterator))
                                    {
                                        field->attribute = attribute;
                                    }
                                    else
                                    {
                                        std::cerr << "Expected a right bracket" << std::endl;
                                        return nullptr;
                                    }
                                }
                                else
                                {
                                    std::cerr << "Expected a right bracket" << std::endl;
                                    return nullptr;
                                }
                            }
                        }
                        else
                        {
                            std::cerr << "Expected a left bracket" << std::endl;
                            return nullptr;
                        }
                    }

                    if (check(Token::Type::IDENTIFIER, tokens, iterator))
                    {
                        field->name = (iterator - 1)->value;

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
                        std::cerr << "Expected a field name" << std::endl;
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
    else if (check(Token::Type::IDENTIFIER, tokens, iterator))
    {
        // TODO: add expression statement
        // TODO: parse exprtession

        std::unique_ptr<ASTNode> result(new ASTNode());
        std::string typeName = (iterator - 1)->value;

        if (check(Token::Type::LEFT_PARENTHESIS, tokens, iterator))
        {
            // function call
            // TODO: parse parameters
        }
        else if (check(Token::Type::IDENTIFIER, tokens, iterator))
        {
            result->type = ASTNode::Type::DECLARATION_VARIABLE;
            result->typeName = typeName;
            result->name = (iterator - 1)->value;
        }

        if (check(Token::Type::SEMICOLON, tokens, iterator))
        {
            declarations.back().push_back(result.get());
            return result;
        }
        else
        {
            std::cerr << "Expected a semicolon" << std::endl;
            return nullptr;
        }

        return result;
    }
    else
    {
        std::cerr << "Expected a keyword or an identifier" << std::endl;
        return nullptr;
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
        if (node->isInline) std::cout << "inline ";
        if (node->isConst) std::cout << "const ";
        if (node->isExtern) std::cout << "extern ";
        std::cout << node->typeName;

    }
    if (node->attribute != ASTNode::Attribute::NONE) std::cout << ", attribute: " << attributeToString(node->attribute);

    std::cout << std::endl;

    for (const std::unique_ptr<ASTNode>& child : node->children)
    {
        ASTContext::dumpNode(child, indent + "  ");
    }
}
