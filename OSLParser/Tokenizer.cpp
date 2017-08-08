//
//  Tokenizer.cpp
//  OSLParser
//
//  Created by Elviss Strazdins on 25/04/2017.
//  Copyright (c) 2017 Elviss Strazdins. All rights reserved.
//

#include <iostream>
#include "Tokenizer.hpp"

bool tokenize(const std::vector<char>& code, std::vector<Token>& tokens)
{
    for (std::vector<char>::const_iterator i = code.begin(); i != code.end();)
    {
        Token token;

        if (*i == '(' || *i == ')' ||
            *i == '{' || *i == '}' ||
            *i == '[' || *i == ']' ||
            *i == ',' || *i == ';' ||
            *i == ':') // punctuation
        {
            token.kind = Token::Kind::PUNCTUATOR;
            if (*i == '(') token.type = Token::Type::LEFT_PARENTHESIS;
            if (*i == ')') token.type = Token::Type::RIGHT_PARENTHESIS;
            if (*i == '{') token.type = Token::Type::LEFT_BRACE;
            if (*i == '}') token.type = Token::Type::RIGHT_BRACE;
            if (*i == '[') token.type = Token::Type::LEFT_BRACKET;
            if (*i == ']') token.type = Token::Type::RIGHT_BRACKET;
            if (*i == ',') token.type = Token::Type::COMMA;
            if (*i == ';') token.type = Token::Type::SEMICOLON;
            if (*i == ':') token.type = Token::Type::COLON;
            token.value.push_back(*i);

            ++i;
        }
        else if ((*i >= '0' && *i <= '9') ||  // number
                 (*i == '.' && (i + 1) != code.end() && *(i + 1) >= '0' && *(i + 1) <= '9')) // starts with a dot
        {
            token.kind = Token::Kind::LITERAL;
            token.type = Token::Type::LITERAL_INT;

            bool dot = false;

            while (i != code.end() &&
                   ((*i >= '0' && *i <= '9') || *i == '.'))
            {
                if (*i == '.')
                {
                    if (dot)
                    {
                        std::cerr << "Invalid number" << std::endl;
                        return false;
                    }
                    else
                    {
                        dot = true;
                        token.type = Token::Type::LITERAL_FLOAT;
                    }
                }

                token.value.push_back(*i);
                ++i;
            }
        }
        else if (*i == '"') // string literal
        {
            token.kind = Token::Kind::LITERAL;
            token.type = Token::Type::LITERAL_STRING;

            while (++i != code.end() &&
                   *i != '"')
            {
                if (*i == '\\')
                {
                    if (++i == code.end())
                    {
                        std::cerr << "Unterminated char" << std::endl;
                        return false;
                    }
                    else if (*i == 'a') token.value.push_back('\a');
                    else if (*i == 'b') token.value.push_back('\b');
                    else if (*i == 't') token.value.push_back('\t');
                    else if (*i == 'n') token.value.push_back('\n');
                    else if (*i == 'v') token.value.push_back('\v');
                    else if (*i == 'f') token.value.push_back('\f');
                    else if (*i == 'r') token.value.push_back('\r');
                    else if (*i == 'e') token.value.push_back('\e');
                    else if (*i == '"') token.value.push_back('"');
                    else if (*i == '\?') token.value.push_back('\?');
                    else if (*i == '\\') token.value.push_back('\\');
                    // TODO: handle numeric character references
                }
                else
                {
                    token.value.push_back(*i);
                }
            }

            if (*i == '"')
            {
                ++i;
            }
            else
            {
                std::cerr << "Unterminated string" << std::endl;
                return false;
            }
        }
        else if (*i == '\'') // char literal
        {
            token.kind = Token::Kind::LITERAL;
            token.type = Token::Type::LITERAL_CHAR;

            if (++i == code.end()) // reached end of file
            {
                std::cerr << "Unterminated char" << std::endl;
                return false;
            }

            while (*i != '\'')
            {
                if (*i == '\\')
                {
                    if (++i == code.end())
                    {
                        std::cerr << "Unterminated char" << std::endl;
                        return false;
                    }
                    else if (*i == 'a') token.value.push_back('\a');
                    else if (*i == 'b') token.value.push_back('\b');
                    else if (*i == 't') token.value.push_back('\t');
                    else if (*i == 'n') token.value.push_back('\n');
                    else if (*i == 'v') token.value.push_back('\v');
                    else if (*i == 'f') token.value.push_back('\f');
                    else if (*i == 'r') token.value.push_back('\r');
                    else if (*i == 'e') token.value.push_back('\e');
                    else if (*i == '\'') token.value.push_back('\'');
                    else if (*i == '\?') token.value.push_back('\?');
                    else if (*i == '\\') token.value.push_back('\\');
                    // TODO: handle numeric character references
                }
                else
                {
                    token.value.push_back(*i);
                }

                if (++i == code.end()) // reached end of file
                {
                    std::cerr << "Unterminated char" << std::endl;
                    return false;
                }
            }

            if (token.value.length() == 1)
            {
                ++i;
            }
            else
            {
                std::cerr << "Invalid char literal" << std::endl;
                return false;
            }
        }
        else if ((*i >= 'a' && *i <= 'z') ||
                 (*i >= 'A' && *i <= 'Z') ||
                 *i == '_')
        {
            while (i != code.end() &&
                   ((*i >= 'a' && *i <= 'z') ||
                    (*i >= 'A' && *i <= 'Z') ||
                    *i == '_' ||
                    (*i >= '0' && *i <= '9')))
            {
                token.value.push_back(*i);
                ++i;
            }

            if (token.value == "and_eq") { token.kind = Token::Kind::OPERATOR; token.type = Token::Type::OPERATOR_BITWISE_AND_ASSIGNMENT; }
            else if (token.value == "or_eq") { token.kind = Token::Kind::OPERATOR; token.type = Token::Type::OPERATOR_BITWISE_OR_ASSIGNMENT; }
            else if (token.value == "xor_eq") { token.kind = Token::Kind::OPERATOR; token.type = Token::Type::OPERATOR_BITWISE_XOR_ASSIGNMENT; }
            else if (token.value == "compl") { token.kind = Token::Kind::OPERATOR; token.type = Token::Type::OPERATOR_BITWISE_NOT; }
            else if (token.value == "bitand") { token.kind = Token::Kind::OPERATOR; token.type = Token::Type::OPERATOR_BITWISE_AND; }
            else if (token.value == "bitor") { token.kind = Token::Kind::OPERATOR; token.type = Token::Type::OPERATOR_BITWISE_OR; }
            else if (token.value == "xor") { token.kind = Token::Kind::OPERATOR; token.type = Token::Type::OPERATOR_BITWISE_XOR; }
            else if (token.value == "not_eq") { token.kind = Token::Kind::OPERATOR; token.type = Token::Type::OPERATOR_NOT_EQUAL; }
            else if (token.value == "and") { token.kind = Token::Kind::OPERATOR; token.type = Token::Type::OPERATOR_AND; }
            else if (token.value == "or") { token.kind = Token::Kind::OPERATOR; token.type = Token::Type::OPERATOR_OR; }
            else if (token.value == "not") { token.kind = Token::Kind::OPERATOR; token.type = Token::Type::OPERATOR_NOT; }
            else if (token.value == "if") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_IF; }
            else if (token.value == "else") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_ELSE; }
            else if (token.value == "return") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_RETURN; }
            else if (token.value == "for") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_FOR; }
            else if (token.value == "while") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_WHILE; }
            else if (token.value == "do") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_DO; }
            else if (token.value == "break") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_BREAK; }
            else if (token.value == "continue") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_CONTINUE; }
            else if (token.value == "true") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_TRUE; }
            else if (token.value == "false") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_FALSE; }
            else if (token.value == "inline") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_INLINE; }
            else if (token.value == "goto") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_GOTO; }
            else if (token.value == "switch") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_SWITCH; }
            else if (token.value == "case") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_CASE; }
            else if (token.value == "default") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_DEFAULT; }
            else if (token.value == "static") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_STATIC; }
            else if (token.value == "const") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_CONST; }
            else if (token.value == "extern") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_EXTERN; }
            else if (token.value == "mutable") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_MUTABLE; }
            else if (token.value == "auto") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_AUTO; }
            else if (token.value == "typedef") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_TYPEDEF; }
            else if (token.value == "union") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_UNION; }
            else if (token.value == "enum") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_ENUM; }
            else if (token.value == "template") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_TEMPLATE; }
            else if (token.value == "struct") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_STRUCT; }
            else if (token.value == "class") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_CLASS; }
            else if (token.value == "public") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_PUBLIC; }
            else if (token.value == "protected") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_PROTECTED; }
            else if (token.value == "private") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_PRIVATE; }
            else if (token.value == "new") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_NEW; }
            else if (token.value == "delete") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_DELETE; }
            else if (token.value == "this") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_THIS; }
            else if (token.value == "sizeof") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_SIZEOF; }
            else if (token.value == "namespace") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_NAMESPACE; }
            else if (token.value == "using") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_USING; }
            else if (token.value == "try") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_TRY; }
            else if (token.value == "catch") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_CATCH; }
            else if (token.value == "throw") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_THROW; }
            else if (token.value == "noexcept") { token.kind = Token::Kind::KEYWORD; token.type = Token::Type::KEYWORD_NOEXCEPT; }
            else { token.kind = Token::Kind::IDENTIFIER; token.type = Token::Type::IDENTIFIER; }
        }
        else if (*i == '+' || *i == '-' ||
                 *i == '*' || *i == '/' ||
                 *i == '%' || *i == '=' ||
                 *i == '&' || *i == '|' ||
                 *i == '<' || *i == '>' ||
                 *i == '!' || *i == '.' ||
                 *i == '~' || *i == '^')
        {
            if (*i == '+')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_PLUS;
                token.value.push_back(*i);
                ++i;

                if (i != code.end())
                {
                    if (*i == '=')
                    {
                        token.type = Token::Type::OPERATOR_PLUS_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '+')
                    {
                        token.type = Token::Type::OPERATOR_INCREMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                }
            }
            else if (*i == '-')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_MINUS;
                token.value.push_back(*i);
                ++i;

                if (i != code.end())
                {
                    if (*i == '=')
                    {
                        token.type = Token::Type::OPERATOR_MINUS_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '-')
                    {
                        token.type = Token::Type::OPERATOR_DECREMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '>')
                    {
                        token.type = Token::Type::OPERATOR_ARROW;
                        token.value.push_back(*i);
                        ++i;
                    }
                }
            }
            else if (*i == '*')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_MULTIPLY;
                token.value.push_back(*i);
                ++i;

                if (i != code.end())
                {
                    if (*i == '=')
                    {
                        token.type = Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                }
            }
            else if (*i == '/')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_DIVIDE;
                token.value.push_back(*i);
                ++i;

                if (i != code.end())
                {
                    if (*i == '=')
                    {
                        token.type = Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '/' || // single line comment
                             *i == '*') // multiline comment
                    {
                        if (*i == '/') // single-line comment
                        {
                            ++i;

                            while (i != code.end() && *i != '\n')
                            {
                                if (++i == code.end()) break; // reached end of file
                            }
                        }
                        else if (*i == '*') // multi-line comment
                        {
                            ++i;

                            while (i != code.end() && *i != '*' &&
                                   (i + 1) != code.end() && *(i + 1) != '/')
                            {
                                ++i;
                            }

                            if (*i == '*' && ++i != code.end() &&
                                *i == '/')
                            {
                                ++i;
                            }
                            else
                            {
                                std::cerr << "Unterminated block comment" << *i << *(i + 1) << std::endl;
                                return false;
                            }
                        }

                        continue;
                    }
                }
            }
            else if (*i == '%')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_MODULO;
                token.value.push_back(*i);
                ++i;

                if (i != code.end())
                {
                    if (*i == '=')
                    {
                        token.type = Token::Type::OPERATOR_MODULO_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                }
            }
            else if (*i == '=')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_ASSIGNMENT;
                token.value.push_back(*i);
                ++i;

                if (i != code.end())
                {
                    if (*i == '=')
                    {
                        token.type = Token::Type::OPERATOR_EQUAL;
                        token.value.push_back(*i);
                        ++i;
                    }
                }
            }
            else if (*i == '&')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_BITWISE_AND;
                token.value.push_back(*i);
                ++i;

                if (i != code.end())
                {
                    if (*i == '=')
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_AND_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '&')
                    {
                        token.type = Token::Type::OPERATOR_AND;
                        token.value.push_back(*i);
                        ++i;
                    }
                }
            }
            else if (*i == '~')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_BITWISE_NOT;
                token.value.push_back(*i);
                ++i;

                if (i != code.end())
                {
                    if (*i == '=')
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_NOT_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                }
            }
            else if (*i == '^')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_BITWISE_XOR;
                token.value.push_back(*i);
                ++i;

                if (i != code.end())
                {
                    if (*i == '=')
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_XOR_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                }
            }
            else if (*i == '|')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_BITWISE_OR;
                token.value.push_back(*i);
                ++i;

                if (i != code.end())
                {
                    if (*i == '=')
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_OR_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '|')
                    {
                        token.type = Token::Type::OPERATOR_OR;
                        token.value.push_back(*i);
                        ++i;
                    }
                }
            }
            else if (*i == '<')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_LESS_THAN;
                token.value.push_back(*i);
                ++i;

                if (i != code.end())
                {
                    if (*i == '=')
                    {
                        token.type = Token::Type::OPERATOR_LESS_THAN_EQUAL;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '<')
                    {
                        token.type = Token::Type::OPERATOR_SHIFT_LEFT;
                        token.value.push_back(*i);
                        ++i;

                        if (i != code.end())
                        {
                            if (*i == '=')
                            {
                                token.type = Token::Type::OPERATOR_SHIFT_LEFT_ASSIGNMENT;
                                token.value.push_back(*i);
                                ++i;
                            }
                        }
                    }
                }
            }
            else if (*i == '>')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_GREATER_THAN;
                token.value.push_back(*i);
                ++i;

                if (i != code.end())
                {
                    if (*i == '=')
                    {
                        token.type = Token::Type::OPERATOR_GREATER_THAN_EQUAL;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '>')
                    {
                        token.type = Token::Type::OPERATOR_SHIFT_RIGHT;
                        token.value.push_back(*i);
                        ++i;

                        if (i != code.end())
                        {
                            if (*i == '=')
                            {
                                token.type = Token::Type::OPERATOR_SHIFT_RIGHT_ASSIGNMENT;
                                token.value.push_back(*i);
                                ++i;
                            }
                        }
                    }
                }
            }
            else if (*i == '!')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_NOT;
                token.value.push_back(*i);
                ++i;

                if (i != code.end())
                {
                    if (*i == '=')
                    {
                        token.type = Token::Type::OPERATOR_NOT_EQUAL;
                        token.value.push_back(*i);
                        ++i;
                    }
                }
            }
            else if (*i == '?')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_CONDITIONAL;
                token.value.push_back(*i);
                ++i;
            }
            else if (*i == '.')
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = Token::Type::OPERATOR_DOT;
                token.value.push_back(*i);
                ++i;
            }
        }
        else if (*i == ' ' || *i == '\t' || *i == '\n' || *i == '\r') // whitespace
        {
            ++i;
            continue;
        }
        else
        {
            std::cerr << "Unknown character" << std::endl;
            return false;
        }
        
        tokens.push_back(token);
    }
    
    return true;
}
