//
//  Tokenizer.cpp
//  OSLParser
//
//  Created by Elviss Strazdins on 25/04/2017.
//  Copyright (c) 2017 Elviss Strazdins. All rights reserved.
//

#include <iostream>
#include "Tokenizer.h"

bool tokenize(const std::vector<uint8_t>& buffer, std::vector<Token>& tokens)
{
    for (std::vector<uint8_t>::const_iterator i = buffer.begin(); i != buffer.end(); ++i)
    {
        char c = static_cast<char>(*i);

        Token token;

        if (c == '(' || c == ')' ||
            c == '{' || c == '}' ||
            c == '[' || c == ']' ||
            c == ',' || c == ';' || c == ':') // punctuation
        {
            if (c == '(') token.type = Token::Type::LEFT_PARENTHESIS;
            if (c == ')') token.type = Token::Type::RIGHT_PARENTHESIS;
            if (c == '{') token.type = Token::Type::LEFT_BRACE;
            if (c == '}') token.type = Token::Type::RIGHT_BRACE;
            if (c == '[') token.type = Token::Type::LEFT_BRACKET;
            if (c == ']') token.type = Token::Type::RIGHT_BRACKET;
            if (c == ',') token.type = Token::Type::COMMA;
            if (c == ';') token.type = Token::Type::SEMICOLON;
            if (c == ':') token.type = Token::Type::COLON;
            token.value.push_back(c);
        }
        else if (c >= '0' && c <= '9') // number
        {
            token.type = Token::Type::INT_LITERAL;

            bool dot = false;

            while ((c >= '0' && c <= '9') || c == '.')
            {
                if (c == '.')
                {
                    if (dot)
                    {
                        std::cerr << "Invalid number" << std::endl;
                        return false;
                    }
                    else
                    {
                        dot = true;
                        token.type = Token::Type::FLOAT_LITERAL;
                    }
                }

                token.value.push_back(c);

                if (++i == buffer.end()) break; // reached end of file
                c = static_cast<char>(*i);
            }
        }
        else if (c == '"') // string literal
        {
            token.type = Token::Type::STRING_LITERAL;

            if (++i == buffer.end()) break; // reached end of file
            c = static_cast<char>(*i);

            while (c != '"')
            {
                token.value.push_back(c);

                if (++i == buffer.end()) break; // reached end of file
                c = static_cast<char>(*i);
            }

            if (c != '"')
            {
                std::cerr << "Unterminated string" << std::endl;
                return false;
            }
        }
        else if ((c >= 'a' && c <= 'z') ||
                 (c >= 'A' && c <= 'Z') ||
                 c == '_')
        {
            while ((c >= 'a' && c <= 'z') ||
                   (c >= 'A' && c <= 'Z') ||
                   c == '_' ||
                   (c >= '0' && c <= '9'))
            {
                token.value.push_back(c);

                if (++i == buffer.end()) break; // reached end of file
                c = static_cast<char>(*i);
            }

            if (token.value == "if") token.type = Token::Type::KEYWORD_IF;
            else if (token.value == "else") token.type = Token::Type::KEYWORD_ELSE;
            else if (token.value == "return") token.type = Token::Type::KEYWORD_RETURN;
            else if (token.value == "for") token.type = Token::Type::KEYWORD_FOR;
            else if (token.value == "while") token.type = Token::Type::KEYWORD_WHILE;
            else if (token.value == "do") token.type = Token::Type::KEYWORD_DO;
            else if (token.value == "break") token.type = Token::Type::KEYWORD_BREAK;
            else if (token.value == "continue") token.type = Token::Type::KEYWORD_CONTINUE;
            else if (token.value == "true") token.type = Token::Type::KEYWORD_TRUE;
            else if (token.value == "false") token.type = Token::Type::KEYWORD_FALSE;
            else if (token.value == "inline") token.type = Token::Type::KEYWORD_INLINE;
            else if (token.value == "struct") token.type = Token::Type::KEYWORD_STRUCT;
            else if (token.value == "typedef") token.type = Token::Type::KEYWORD_TYPEDEF;
            else token.type = Token::Type::IDENTIFIER;
        }
        else if (c == '+' || c == '-' ||
                 c == '*' || c == '/' ||
                 c == '%' || c == '=' ||
                 c == '&' || c == '|' ||
                 c == '<' || c == '>' ||
                 c == '!' || c == '.' ||
                 c == '~' || c == '^')
        {
            if (c == '/' && (i + 1) != buffer.end() && // comment
                (static_cast<char>(*(i + 1)) == '/' || static_cast<char>(*(i + 1)) == '*'))
            {
                ++i;
                c = static_cast<char>(*i);

                if (c == '/') // single-line comment
                {
                    if (++i == buffer.end()) break; // reached end of file
                    c = static_cast<char>(*i);

                    while (c != '\n')
                    {
                        if (++i == buffer.end()) break; // reached end of file
                        c = static_cast<char>(*i);
                    }
                }
                else if (c == '*') // multi-line comment
                {
                    if (++i == buffer.end()) break; // reached end of file
                    c = static_cast<char>(*i);

                    while (c != '*' && (i + 1) != buffer.end() &&
                           static_cast<char>(*(i + 1)) != '/')
                    {
                        if (++i == buffer.end()) break; // reached end of file
                        c = static_cast<char>(*i);
                    }

                    if (c != '*' || (i + 1) == buffer.end() ||
                        static_cast<char>(*(i + 1)) != '/')
                    {
                        std::cerr << "Unterminated block comment" << std::endl;
                        return false;
                    }
                    else
                    {
                        ++i;
                    }
                }

                continue;
            }
            else if (c == '+')
            {
                token.type = Token::Type::OPERATOR_PLUS;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_PLUS_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '+')
                    {
                        token.type = Token::Type::OPERATOR_INCREMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '-')
            {
                token.type = Token::Type::OPERATOR_MINUS;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_MINUS_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '-')
                    {
                        token.type = Token::Type::OPERATOR_DECREMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '*')
            {
                token.type = Token::Type::OPERATOR_MULTIPLY;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '/')
            {
                token.type = Token::Type::OPERATOR_DIVIDE;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '/' || // single line comment
                             static_cast<char>(*(i + 1)) == '*') // multiline comment
                    {
                        ++i;
                        c = static_cast<char>(*i);

                        if (c == '/') // single-line comment
                        {
                            if (++i == buffer.end()) break; // reached end of file
                            c = static_cast<char>(*i);

                            while (c != '\n')
                            {
                                if (++i == buffer.end()) break; // reached end of file
                                c = static_cast<char>(*i);
                            }
                        }
                        else if (c == '*') // multi-line comment
                        {
                            if (++i == buffer.end()) break; // reached end of file
                            c = static_cast<char>(*i);

                            while (c != '*' && (i + 1) != buffer.end() &&
                                   static_cast<char>(*(i + 1)) != '/')
                            {
                                if (++i == buffer.end()) break; // reached end of file
                                c = static_cast<char>(*i);
                            }

                            if (c != '*' || (i + 1) == buffer.end() ||
                                static_cast<char>(*(i + 1)) != '/')
                            {
                                std::cerr << "Unterminated block comment" << std::endl;
                                return false;
                            }
                            else
                            {
                                ++i;
                            }
                        }

                        continue;

                    }
                }
            }
            else if (c == '%')
            {
                token.type = Token::Type::OPERATOR_MODULO;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_MODULO_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '=')
            {
                token.type = Token::Type::OPERATOR_ASSIGNMENT;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_EQUAL;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '&')
            {
                token.type = Token::Type::OPERATOR_BITWISE_AND;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_AND_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '&')
                    {
                        token.type = Token::Type::OPERATOR_AND;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '|')
            {
                token.type = Token::Type::OPERATOR_BITWISE_OR;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_OR_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '|')
                    {
                        token.type = Token::Type::OPERATOR_OR;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '<')
            {
                token.type = Token::Type::OPERATOR_LESS_THAN;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_LESS_THAN_EQUAL;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '<')
                    {
                        token.type = Token::Type::OPERATOR_SHIFT_LEFT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);

                        if ((i + 1) != buffer.end())
                        {
                            if (static_cast<char>(*(i + 1)) == '=')
                            {
                                token.type = Token::Type::OPERATOR_SHIFT_LEFT_ASSIGNMENT;
                                ++i;
                                c = static_cast<char>(*i);
                                token.value.push_back(c);
                            }
                        }
                    }
                }
            }
            else if (c == '>')
            {
                token.type = Token::Type::OPERATOR_GREATER_THAN;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_GREATER_THAN_EQUAL;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '>')
                    {
                        token.type = Token::Type::OPERATOR_SHIFT_RIGHT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);

                        if ((i + 1) != buffer.end())
                        {
                            if (static_cast<char>(*(i + 1)) == '=')
                            {
                                token.type = Token::Type::OPERATOR_SHIFT_RIGHT_ASSIGNMENT;
                                ++i;
                                c = static_cast<char>(*i);
                                token.value.push_back(c);
                            }
                        }
                    }
                }
            }
            else if (c == '!')
            {
                token.type = Token::Type::OPERATOR_NOT;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_NOT_EQUAL;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '?')
            {
                token.type = Token::Type::OPERATOR_CONDITIONAL;
                token.value.push_back(c);
            }
            else if (c == '.')
            {
                token.type = Token::Type::OPERATOR_DOT;
                token.value.push_back(c);
            }
        }
        else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') // whitespace
        {
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
