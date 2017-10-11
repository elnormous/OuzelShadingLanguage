//
//  Parser.cpp
//  OSLParser
//
//  Created by Elviss Strazdins on 25/04/2017.
//  Copyright (c) 2017 Elviss Strazdins. All rights reserved.
//

#include <iostream>
#include "Parser.hpp"

std::vector<std::string> builtinTypes = {
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
        if (iterator->type == Token::Type::KEYWORD_STRUCT)
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
        else if (iterator->type == Token::Type::KEYWORD_TYPEDEF)
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

        ++iterator;
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
        if (iterator != tokens.end())
        {
            if (iterator->type == Token::Type::KEYWORD_INLINE)
            {
                result->isInline = true;
                ++iterator;
            }
            else if (iterator->type == Token::Type::KEYWORD_CONST)
            {
                result->isConst = true;
                ++iterator;
            }
            else if (iterator->type == Token::Type::KEYWORD_EXTERN)
            {
                result->isExtern = true;
                ++iterator;
            }
            else
            {
                break;
            }
        }
        else
        {
            std::cerr << "Unexpected end of declaration" << std::endl;
            return nullptr;
        }
    }

    if (iterator != tokens.end())
    {
        if (iterator->type == Token::Type::IDENTIFIER)
        {
            result->typeName = iterator->value;

            if (++iterator != tokens.end() &&
                iterator->type == Token::Type::IDENTIFIER)
            {
                result->name = iterator->value;

                if (++iterator != tokens.end())
                {
                    if (iterator->type == Token::Type::SEMICOLON)
                    {
                        declarations.back().push_back(result.get());
                        result->type = ASTNode::Type::DECLARATION_VARIABLE;
                        return result;
                    }
                    else if (iterator->type == Token::Type::LEFT_PARENTHESIS)
                    {
                        result->type = ASTNode::Type::DECLARATION_FUNCTION;

                        if (++iterator != tokens.end())
                        {
                            if (iterator->type == Token::Type::IDENTIFIER)
                            {
                                for (;;)
                                {
                                    if (std::unique_ptr<ASTNode> param = parseParamDecl(tokens, iterator, declarations))
                                    {
                                        result->children.push_back(std::move(param));
                                    }
                                    else
                                    {
                                        std::cerr << "Failed to parse a parameter declaration" << std::endl;
                                        return nullptr;
                                    }

                                    if (++iterator != tokens.end())
                                    {
                                        if (iterator->type == Token::Type::RIGHT_PARENTHESIS)
                                        {
                                            break;
                                        }
                                        else if (iterator->type != Token::Type::COMMA)
                                        {
                                            std::cerr << "Expected a comma or a right parenthesis" << std::endl;
                                            return nullptr;
                                        }
                                    }
                                    else
                                    {
                                        std::cerr << "Unexpected end of function declaration" << std::endl;
                                        return nullptr;
                                    }
                                }
                            }
                            else if (iterator->type != Token::Type::RIGHT_PARENTHESIS)
                            {
                                std::cerr << "Expected a keyword or a right parenthesis" << std::endl;
                                return nullptr;
                            }

                            if (++iterator != tokens.end())
                            {
                                if (iterator->type == Token::Type::LEFT_BRACE)
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
                                else if (iterator->type == Token::Type::SEMICOLON)
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
                            std::cerr << "Unexpected end of function declaration" << std::endl;
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
                    std::cerr << "Unexpected end of declaration" << std::endl;
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
    }
    else
    {
        std::cerr << "Unexpected end of declaration" << std::endl;
        return nullptr;
    }

    return result;
}

std::unique_ptr<ASTNode> ASTContext::parseStructDecl(const std::vector<Token>& tokens,
                                                     std::vector<Token>::const_iterator& iterator,
                                                     std::vector<std::vector<ASTNode*>>& declarations)
{
    if (iterator->type == Token::Type::KEYWORD_STRUCT)
    {
        if (++iterator != tokens.end() && iterator->type == Token::Type::IDENTIFIER)
        {
            std::unique_ptr<ASTNode> result(new ASTNode());
            result->type = ASTNode::Type::DECLARATION_STRUCT;
            result->name = iterator->value;

            if (++iterator != tokens.end())
            {
                if (iterator->type == Token::Type::LEFT_BRACE)
                {
                    for (;;)
                    {
                        if (++iterator != tokens.end())
                        {
                            if (iterator->type == Token::Type::RIGHT_BRACE)
                            {
                                if (++iterator != tokens.end() && iterator->type == Token::Type::SEMICOLON)
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
                            else if (iterator->type == Token::Type::IDENTIFIER)
                            {
                                if (std::unique_ptr<ASTNode> field = parseFieldDecl(tokens, iterator, declarations))
                                {
                                    result->children.push_back(std::move(field));
                                }
                                else
                                {
                                    std::cerr << "Failed to parse a field declaration" << std::endl;
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
                            std::cerr << "Unexpected end of structure declaration" << std::endl;
                            return nullptr;
                        }
                    }
                }
                else if (iterator->type == Token::Type::SEMICOLON)
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
                std::cerr << "Unexpected end of declaration" << std::endl;
                return nullptr;
            }
        }
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseFieldDecl(const std::vector<Token>& tokens,
                                                    std::vector<Token>::const_iterator& iterator,
                                                    std::vector<std::vector<ASTNode*>>& declarations)
{
    if (iterator->type == Token::Type::IDENTIFIER)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::DECLARATION_FIELD;

        result->typeName = iterator->value;

        while (++iterator != tokens.end())
        {
            if (iterator->type == Token::Type::IDENTIFIER)
            {
                result->name = iterator->value;

                if (++iterator != tokens.end() && iterator->type == Token::Type::SEMICOLON)
                {
                    return result;
                }
                else
                {
                    std::cerr << "Expected a semicolon" << std::endl;
                    return nullptr;
                }
            }
            else if (iterator->type == Token::Type::LEFT_BRACKET)
            {
                if (++iterator != tokens.end() && iterator->type == Token::Type::LEFT_BRACKET)
                {
                    if (++iterator != tokens.end() && iterator->type == Token::Type::IDENTIFIER)
                    {
                        ASTNode::Attribute attribute = ASTNode::Attribute::NONE;

                        // TODO: find slot number
                        if (iterator->value == "binormal") attribute = ASTNode::Attribute::BINORMAL;
                        else if (iterator->value == "blend_indices") attribute = ASTNode::Attribute::BLEND_INDICES;
                        else if (iterator->value == "blend_weight") attribute = ASTNode::Attribute::BLEND_WEIGHT;
                        else if (iterator->value == "color") attribute = ASTNode::Attribute::COLOR;
                        else if (iterator->value == "normal") attribute = ASTNode::Attribute::NORMAL;
                        else if (iterator->value == "position") attribute = ASTNode::Attribute::POSITION;
                        else if (iterator->value == "position_transformed") attribute = ASTNode::Attribute::POSITION_TRANSFORMED;
                        else if (iterator->value == "point_size") attribute = ASTNode::Attribute::POINT_SIZE;
                        else if (iterator->value == "tangent") attribute = ASTNode::Attribute::TANGENT;
                        else if (iterator->value == "texture_coordinates") attribute = ASTNode::Attribute::TEXTURE_COORDINATES;
                        else
                        {
                            std::cerr << "Invalid semantic" << std::endl;
                            return nullptr;
                        }

                        if (++iterator != tokens.end() && iterator->type == Token::Type::RIGHT_BRACKET)
                        {
                            if (++iterator != tokens.end() && iterator->type == Token::Type::RIGHT_BRACKET)
                            {
                                result->attribute = attribute;
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
            else
            {
                std::cerr << "Expected a field name" << std::endl;
                return nullptr;
            }
        }
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseTypedefDecl(const std::vector<Token>& tokens,
                                                      std::vector<Token>::const_iterator& iterator,
                                                      std::vector<std::vector<ASTNode*>>& declarations)
{
    if (iterator->type == Token::Type::KEYWORD_TYPEDEF)
    {
        if (++iterator != tokens.end() && iterator->type == Token::Type::IDENTIFIER)
        {
            std::unique_ptr<ASTNode> result(new ASTNode());
            result->type = ASTNode::Type::DECLARATION_TYPE_DEFINITION;
            result->typeName = iterator->value;

            if (++iterator != tokens.end() && iterator->type == Token::Type::IDENTIFIER)
            {
                result->name = iterator->value;

                if (++iterator != tokens.end() && iterator->type == Token::Type::SEMICOLON)
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
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseVarDecl(const std::vector<Token>& tokens,
                                                  std::vector<Token>::const_iterator& iterator,
                                                  std::vector<std::vector<ASTNode*>>& declarations)
{
    if (iterator->type == Token::Type::IDENTIFIER)
    {
    }
    else if (iterator->type == Token::Type::KEYWORD_CONST)
    {
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseParamDecl(const std::vector<Token>& tokens,
                                                    std::vector<Token>::const_iterator& iterator,
                                                    std::vector<std::vector<ASTNode*>>& declarations)
{
    if (iterator->type == Token::Type::IDENTIFIER)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::DECLARATION_PARAMETER;
        result->typeName = iterator->value;

        if (++iterator != tokens.end() && iterator->type == Token::Type::IDENTIFIER)
        {
            result->name = iterator->value;
        }
        else
        {
            std::cerr << "Expected an identifier" << std::endl;
            return nullptr;
        }

        return result;
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseCompoundStatement(const std::vector<Token>& tokens,
                                                            std::vector<Token>::const_iterator& iterator,
                                                            std::vector<std::vector<ASTNode*>>& declarations)
{
    if (iterator->type == Token::Type::LEFT_BRACE)
    {
        declarations.push_back(std::vector<ASTNode*>());

        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_COMPOUND;

        for (;;)
        {
            if (++iterator != tokens.end())
            {
                if (iterator->type == Token::Type::RIGHT_BRACE)
                {
                    declarations.pop_back();
                    return result;
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
            else
            {
                std::cerr << "Unexpected end of compound statement" << std::endl;
                return nullptr;
            }
        }

        return result;
    }
    else
    {
        std::cerr << "Expected a left brace" << std::endl;
        return nullptr;
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseStatement(const std::vector<Token>& tokens,
                                                    std::vector<Token>::const_iterator& iterator,
                                                    std::vector<std::vector<ASTNode*>>& declarations)
{
    if (iterator->type == Token::Type::LEFT_BRACE)
    {
        return parseCompoundStatement(tokens, iterator, declarations);
    }
    else if (iterator->type == Token::Type::IDENTIFIER)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        std::string typeName = iterator->value;

        if (++iterator != tokens.end())
        {
            if (iterator->type == Token::Type::LEFT_PARENTHESIS)
            {
                // function call
                // TODO: parse parameters
            }
            else if (iterator->type == Token::Type::IDENTIFIER)
            {
                result->type = ASTNode::Type::DECLARATION_VARIABLE;
                result->typeName = typeName;
                result->name = iterator->value;
            }

            if (++iterator != tokens.end())
            {
                return result;
            }
            else
            {
                std::cerr << "Unexpected end of variable declaration" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cerr << "Unexpected end of file" << std::endl;
            return nullptr;
        }

        return result;
    }
    else if (iterator->type == Token::Type::KEYWORD_IF)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_IF;

        if (++iterator != tokens.end())
        {
            if (iterator->type == Token::Type::LEFT_PARENTHESIS)
            {
                // TODO: parse expression
            }
            else
            {
                std::cerr << "Expected a left parenthesis" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cerr << "Unexpected end of if statement" << std::endl;
            return nullptr;
        }

        return result;
    }
    else if (iterator->type == Token::Type::KEYWORD_FOR)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_FOR;

        if (++iterator != tokens.end())
        {
            if (iterator->type == Token::Type::LEFT_PARENTHESIS)
            {
                // TODO: parse expression
            }
            else
            {
                std::cerr << "Expected a left parenthesis" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cerr << "Unexpected end of for statement" << std::endl;
            return nullptr;
        }

        return result;
    }
    else if (iterator->type == Token::Type::KEYWORD_SWITCH)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_SWITCH;

        if (++iterator != tokens.end())
        {
            if (iterator->type == Token::Type::LEFT_PARENTHESIS)
            {
                // TODO: parse expression
            }
            else
            {
                std::cerr << "Expected a left parenthesis" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cerr << "Unexpected end of switch statement" << std::endl;
            return nullptr;
        }

        return result;
    }
    else if (iterator->type == Token::Type::KEYWORD_CASE)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_CASE;

        return result;
    }
    else if (iterator->type == Token::Type::KEYWORD_WHILE)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_WHILE;

        if (++iterator != tokens.end())
        {
            if (iterator->type == Token::Type::LEFT_PARENTHESIS)
            {
                // TODO: parse expression
            }
            else
            {
                std::cerr << "Expected a left parenthesis" << std::endl;
                return nullptr;
            }
        }
        else
        {
            std::cerr << "Unexpected end of while statement" << std::endl;
            return nullptr;
        }

        return result;
    }
    else if (iterator->type == Token::Type::KEYWORD_DO)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_DO;

        return result;
    }
    else if (iterator->type == Token::Type::KEYWORD_BREAK)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_BREAK;

        return result;
    }
    else if (iterator->type == Token::Type::KEYWORD_CONTINUE)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_CONTINUE;

        return result;
    }
    else if (iterator->type == Token::Type::KEYWORD_RETURN)
    {
        std::unique_ptr<ASTNode> result(new ASTNode());
        result->type = ASTNode::Type::STATEMENT_RETURN;

        return result;
    }
    else
    {
        std::cerr << "Expected a keyword or an identifier" << std::endl;
        return nullptr;
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseFunctionDecl(const std::vector<Token>& tokens,
                                                       std::vector<Token>::const_iterator& iterator,
                                                       std::vector<std::vector<ASTNode*>>& declarations)
{
    if (iterator->type == Token::Type::IDENTIFIER)
    {
        //node.type = ASTNode::Type::FUNCTION_DECLARATION;
    }

    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseIf(const std::vector<Token>& tokens,
                                             std::vector<Token>::const_iterator& iterator,
                                             std::vector<std::vector<ASTNode*>>& declarations)
{
    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseFor(const std::vector<Token>& tokens,
                                              std::vector<Token>::const_iterator& iterator,
                                              std::vector<std::vector<ASTNode*>>& declarations)
{
    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseWhile(const std::vector<Token>& tokens,
                                                std::vector<Token>::const_iterator& iterator,
                                                std::vector<std::vector<ASTNode*>>& declarations)
{
    return nullptr;
}

std::unique_ptr<ASTNode> ASTContext::parseDo(const std::vector<Token>& tokens,
                                             std::vector<Token>::const_iterator& iterator,
                                             std::vector<std::vector<ASTNode*>>& declarations)
{
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
