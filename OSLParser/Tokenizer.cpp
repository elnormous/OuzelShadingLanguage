//
//  Tokenizer.cpp
//  OSLParser
//
//  Created by Elviss Strazdins on 25/04/2017.
//  Copyright (c) 2017 Elviss Strazdins. All rights reserved.
//

#include <iostream>
#include "Tokenizer.h"

bool tokenize(const std::vector<char>& code, std::vector<Token>& tokens)
{
    for (std::vector<char>::const_iterator i = code.begin(); i != code.end(); ++i)
    {
        Token token;

        if (*i == '(' || *i == ')' ||
            *i == '{' || *i == '}' ||
            *i == '[' || *i == ']' ||
            *i == ',' || *i == ';' ||
            *i == ':') // punctuation
        {
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
        }
        else if (*i >= '0' && *i <= '9') // number
        {
            token.type = Token::Type::INT_LITERAL;

            bool dot = false;

            token.value.push_back(*i);

            while ((i + 1) != code.end() &&
                   ((*(i + 1) >= '0' && *(i + 1) <= '9') || *(i + 1) == '.'))
            {
                ++i;
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
                        token.type = Token::Type::FLOAT_LITERAL;
                    }
                }

                token.value.push_back(*i);
            }
        }
        else if (*i == '"') // string literal
        {
            token.type = Token::Type::STRING_LITERAL;

            if (++i == code.end()) // reached end of file
            {
                std::cerr << "Unterminated string" << std::endl;
                return false;
            }

            // TODO: handle escaping
            while (*i != '"')
            {
                token.value.push_back(*i);

                if (++i == code.end()) // reached end of file
                {
                    std::cerr << "Unterminated string" << std::endl;
                    return false;
                }
            }
        }
        else if (*i == '\'')
        {
            token.type = Token::Type::CHAR_LITERAL;

            if (++i == code.end()) // reached end of file
            {
                std::cerr << "Unterminated char" << std::endl;
                return false;
            }

            // TODO: handle escaping
            while (*i != '\'')
            {
                token.value.push_back(*i);

                if (++i == code.end()) // reached end of file
                {
                    std::cerr << "Unterminated char" << std::endl;
                    return false;
                }
            }

            if (token.value.length() != 1)
            {
                std::cerr << "Invalid char literal" << std::endl;
                return false;
            }
        }
        else if ((*i >= 'a' && *i <= 'z') ||
                 (*i >= 'A' && *i <= 'Z') ||
                 *i == '_')
        {
            token.value.push_back(*i);

            while ((i + 1) != code.end() &&
                   ((*(i + 1) >= 'a' && *(i + 1) <= 'z') ||
                    (*(i + 1) >= 'A' && *(i + 1) <= 'Z') ||
                    *(i + 1) == '_' ||
                    (*(i + 1) >= '0' && *(i + 1) <= '9')))
            {
                ++i;
                token.value.push_back(*i);
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
            else if (token.value == "const") token.type = Token::Type::KEYWORD_CONST;
            else token.type = Token::Type::IDENTIFIER;
        }
        else if (*i == '+' || *i == '-' ||
                 *i == '*' || *i == '/' ||
                 *i == '%' || *i == '=' ||
                 *i == '&' || *i == '|' ||
                 *i == '<' || *i == '>' ||
                 *i == '!' || *i == '.' ||
                 *i == '~' || *i == '^')
        {
            if (*i == '/' && (i + 1) != code.end() && // comment
                (*(i + 1) == '/' || *(i + 1) == '*'))
            {
                ++i;

                if (*i == '/') // single-line comment
                {
                    if (++i == code.end()) break; // reached end of file

                    while (*i != '\n')
                    {
                        if (++i == code.end()) break; // reached end of file
                    }
                }
                else if (*i == '*') // multi-line comment
                {
                    if (++i == code.end()) break; // reached end of file

                    while (*i != '*' && (i + 1) != code.end() &&
                           *(i + 1) != '/')
                    {
                        if (++i == code.end()) break; // reached end of file
                    }

                    if (*i != '*' || (i + 1) == code.end() ||
                        *(i + 1) != '/')
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
            else if (*i == '+')
            {
                token.type = Token::Type::OPERATOR_PLUS;
                token.value.push_back(*i);

                if ((i + 1) != code.end())
                {
                    if (*(i + 1) == '=')
                    {
                        token.type = Token::Type::OPERATOR_PLUS_ASSIGNMENT;
                        ++i;
                        token.value.push_back(*i);
                    }
                    else if (*(i + 1) == '+')
                    {
                        token.type = Token::Type::OPERATOR_INCREMENT;
                        ++i;
                        token.value.push_back(*i);
                    }
                }
            }
            else if (*i == '-')
            {
                token.type = Token::Type::OPERATOR_MINUS;
                token.value.push_back(*i);

                if ((i + 1) != code.end())
                {
                    if (*(i + 1) == '=')
                    {
                        token.type = Token::Type::OPERATOR_MINUS_ASSIGNMENT;
                        ++i;
                        token.value.push_back(*i);
                    }
                    else if (*(i + 1) == '-')
                    {
                        token.type = Token::Type::OPERATOR_DECREMENT;
                        ++i;
                        token.value.push_back(*i);
                    }
                }
            }
            else if (*i == '*')
            {
                token.type = Token::Type::OPERATOR_MULTIPLY;
                token.value.push_back(*i);

                if ((i + 1) != code.end())
                {
                    if (*(i + 1) == '=')
                    {
                        token.type = Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT;
                        ++i;
                        token.value.push_back(*i);
                    }
                }
            }
            else if (*i == '/')
            {
                token.type = Token::Type::OPERATOR_DIVIDE;
                token.value.push_back(*i);

                if ((i + 1) != code.end())
                {
                    if (*(i + 1) == '=')
                    {
                        token.type = Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT;
                        ++i;
                        token.value.push_back(*i);
                    }
                    else if (*(i + 1) == '/' || // single line comment
                             *(i + 1) == '*') // multiline comment
                    {
                        ++i;

                        if (*i == '/') // single-line comment
                        {
                            if (++i == code.end()) break; // reached end of file

                            while (*i != '\n')
                            {
                                if (++i == code.end()) break; // reached end of file
                            }
                        }
                        else if (*i == '*') // multi-line comment
                        {
                            if (++i == code.end()) break; // reached end of file

                            while (*i != '*' && (i + 1) != code.end() &&
                                   *(i + 1) != '/')
                            {
                                if (++i == code.end()) break; // reached end of file
                            }

                            if (*i != '*' || (i + 1) == code.end() ||
                                *(i + 1) != '/')
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
            else if (*i == '%')
            {
                token.type = Token::Type::OPERATOR_MODULO;
                token.value.push_back(*i);

                if ((i + 1) != code.end())
                {
                    if (*(i + 1) == '=')
                    {
                        token.type = Token::Type::OPERATOR_MODULO_ASSIGNMENT;
                        ++i;
                        token.value.push_back(*i);
                    }
                }
            }
            else if (*i == '=')
            {
                token.type = Token::Type::OPERATOR_ASSIGNMENT;
                token.value.push_back(*i);

                if ((i + 1) != code.end())
                {
                    if (*(i + 1) == '=')
                    {
                        token.type = Token::Type::OPERATOR_EQUAL;
                        ++i;
                        token.value.push_back(*i);
                    }
                }
            }
            else if (*i == '&')
            {
                token.type = Token::Type::OPERATOR_BITWISE_AND;
                token.value.push_back(*i);

                if ((i + 1) != code.end())
                {
                    if (*(i + 1) == '=')
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_AND_ASSIGNMENT;
                        ++i;
                        token.value.push_back(*i);
                    }
                    else if (*(i + 1) == '&')
                    {
                        token.type = Token::Type::OPERATOR_AND;
                        ++i;
                        token.value.push_back(*i);
                    }
                }
            }
            else if (*i == '~')
            {
                token.type = Token::Type::OPERATOR_BITWISE_NOT;
                token.value.push_back(*i);

                if ((i + 1) != code.end())
                {
                    if (*(i + 1) == '=')
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_NOT_ASSIGNMENT;
                        ++i;
                        token.value.push_back(*i);
                    }
                }
            }
            else if (*i == '^')
            {
                token.type = Token::Type::OPERATOR_BITWISE_XOR;
                token.value.push_back(*i);

                if ((i + 1) != code.end())
                {
                    if (*(i + 1) == '=')
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_XOR_ASSIGNMENT;
                        ++i;
                        token.value.push_back(*i);
                    }
                }
            }
            else if (*i == '|')
            {
                token.type = Token::Type::OPERATOR_BITWISE_OR;
                token.value.push_back(*i);

                if ((i + 1) != code.end())
                {
                    if (*(i + 1) == '=')
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_OR_ASSIGNMENT;
                        ++i;
                        token.value.push_back(*i);
                    }
                    else if (*(i + 1) == '|')
                    {
                        token.type = Token::Type::OPERATOR_OR;
                        ++i;
                        token.value.push_back(*i);
                    }
                }
            }
            else if (*i == '<')
            {
                token.type = Token::Type::OPERATOR_LESS_THAN;
                token.value.push_back(*i);

                if ((i + 1) != code.end())
                {
                    if (*(i + 1) == '=')
                    {
                        token.type = Token::Type::OPERATOR_LESS_THAN_EQUAL;
                        ++i;
                        token.value.push_back(*i);
                    }
                    else if (*(i + 1) == '<')
                    {
                        token.type = Token::Type::OPERATOR_SHIFT_LEFT;
                        ++i;
                        token.value.push_back(*i);

                        if ((i + 1) != code.end())
                        {
                            if (*(i + 1) == '=')
                            {
                                token.type = Token::Type::OPERATOR_SHIFT_LEFT_ASSIGNMENT;
                                ++i;
                                token.value.push_back(*i);
                            }
                        }
                    }
                }
            }
            else if (*i == '>')
            {
                token.type = Token::Type::OPERATOR_GREATER_THAN;
                token.value.push_back(*i);

                if ((i + 1) != code.end())
                {
                    if (*(i + 1) == '=')
                    {
                        token.type = Token::Type::OPERATOR_GREATER_THAN_EQUAL;
                        ++i;
                        token.value.push_back(*i);
                    }
                    else if (*(i + 1) == '>')
                    {
                        token.type = Token::Type::OPERATOR_SHIFT_RIGHT;
                        ++i;
                        token.value.push_back(*i);

                        if ((i + 1) != code.end())
                        {
                            if (*(i + 1) == '=')
                            {
                                token.type = Token::Type::OPERATOR_SHIFT_RIGHT_ASSIGNMENT;
                                ++i;
                                token.value.push_back(*i);
                            }
                        }
                    }
                }
            }
            else if (*i == '!')
            {
                token.type = Token::Type::OPERATOR_NOT;
                token.value.push_back(*i);

                if ((i + 1) != code.end())
                {
                    if (*(i + 1) == '=')
                    {
                        token.type = Token::Type::OPERATOR_NOT_EQUAL;
                        ++i;
                        token.value.push_back(*i);
                    }
                }
            }
            else if (*i == '?')
            {
                token.type = Token::Type::OPERATOR_CONDITIONAL;
                token.value.push_back(*i);
            }
            else if (*i == '.')
            {
                token.type = Token::Type::OPERATOR_DOT;
                token.value.push_back(*i);
            }
        }
        else if (*i == ' ' || *i == '\t' || *i == '\n' || *i == '\r') // whitespace
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
