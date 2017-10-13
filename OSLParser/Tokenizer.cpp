//
//  Tokenizer.cpp
//  OSLParser
//
//  Created by Elviss Strazdins on 25/04/2017.
//  Copyright (c) 2017 Elviss Strazdins. All rights reserved.
//

#include <iostream>
#include <map>
#include "Tokenizer.hpp"

static const std::map<std::string, Token::Type> operatorMap = {
    {"and_eq", Token::Type::OPERATOR_BITWISE_AND_ASSIGNMENT},
    {"or_eq", Token::Type::OPERATOR_BITWISE_OR_ASSIGNMENT},
    {"xor_eq", Token::Type::OPERATOR_BITWISE_XOR_ASSIGNMENT},
    {"compl", Token::Type::OPERATOR_BITWISE_NOT},
    {"bitand", Token::Type::OPERATOR_BITWISE_AND},
    {"bitor", Token::Type::OPERATOR_BITWISE_OR},
    {"xor", Token::Type::OPERATOR_BITWISE_XOR},
    {"not_eq", Token::Type::OPERATOR_NOT_EQUAL},
    {"and", Token::Type::OPERATOR_AND},
    {"or", Token::Type::OPERATOR_OR},
    {"not", Token::Type::OPERATOR_NOT}
};

static const std::map<std::string, Token::Type> keywordMap = {
    {"if", Token::Type::KEYWORD_IF},
    {"else", Token::Type::KEYWORD_ELSE},
    {"return", Token::Type::KEYWORD_RETURN},
    {"for", Token::Type::KEYWORD_FOR},
    {"while", Token::Type::KEYWORD_WHILE},
    {"do", Token::Type::KEYWORD_DO},
    {"break", Token::Type::KEYWORD_BREAK},
    {"continue", Token::Type::KEYWORD_CONTINUE},
    {"true", Token::Type::KEYWORD_TRUE},
    {"false", Token::Type::KEYWORD_FALSE},
    {"inline", Token::Type::KEYWORD_INLINE},
    {"goto", Token::Type::KEYWORD_GOTO},
    {"switch", Token::Type::KEYWORD_SWITCH},
    {"case", Token::Type::KEYWORD_CASE},
    {"default", Token::Type::KEYWORD_DEFAULT},
    {"static", Token::Type::KEYWORD_STATIC},
    {"const", Token::Type::KEYWORD_CONST},
    {"extern", Token::Type::KEYWORD_EXTERN},
    {"mutable", Token::Type::KEYWORD_MUTABLE},
    {"auto", Token::Type::KEYWORD_AUTO},
    {"typedef", Token::Type::KEYWORD_TYPEDEF},
    {"union", Token::Type::KEYWORD_UNION},
    {"enum", Token::Type::KEYWORD_ENUM},
    {"template", Token::Type::KEYWORD_TEMPLATE},
    {"struct", Token::Type::KEYWORD_STRUCT},
    {"class", Token::Type::KEYWORD_CLASS},
    {"public", Token::Type::KEYWORD_PUBLIC},
    {"protected", Token::Type::KEYWORD_PROTECTED},
    {"private", Token::Type::KEYWORD_PRIVATE},
    {"new", Token::Type::KEYWORD_NEW},
    {"delete", Token::Type::KEYWORD_DELETE},
    {"this", Token::Type::KEYWORD_THIS},
    {"sizeof", Token::Type::KEYWORD_SIZEOF},
    {"namespace", Token::Type::KEYWORD_NAMESPACE},
    {"using", Token::Type::KEYWORD_USING},
    {"try", Token::Type::KEYWORD_TRY},
    {"catch", Token::Type::KEYWORD_CATCH},
    {"throw", Token::Type::KEYWORD_THROW},
    {"noexcept", Token::Type::KEYWORD_NOEXCEPT}
};

bool tokenize(const std::vector<char>& code, std::vector<Token>& tokens)
{
    for (std::vector<char>::const_iterator i = code.begin(); i != code.end();)
    {
        Token token;

        if (*i == '(' || *i == ')' ||
            *i == '{' || *i == '}' ||
            *i == '[' || *i == ']' ||
            *i == ',' || *i == ';') // punctuation
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
            token.value.push_back(*i);

            ++i;
        }
        else if (*i == ':')
        {
            token.kind = Token::Kind::PUNCTUATOR;
            token.type = Token::Type::COLON;
            token.value.push_back(*i);
            ++i;

            if (i != code.end())
            {
                if (*i == '>') // :>
                {
                    token.type = Token::Type::RIGHT_BRACKET;
                    token.value.push_back(*i);
                    ++i;
                }
            }
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

            std::map<std::string, Token::Type>::const_iterator keywordIterator;

            if ((keywordIterator = operatorMap.find(token.value)) != operatorMap.end())
            {
                token.kind = Token::Kind::OPERATOR;
                token.type = keywordIterator->second;
            }
            else if ((keywordIterator = keywordMap.find(token.value)) != keywordMap.end())
            {
                token.kind = Token::Kind::KEYWORD;
                token.type = keywordIterator->second;
            }
            else
            {
                token.kind = Token::Kind::IDENTIFIER;
                token.type = Token::Type::IDENTIFIER;
            }
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
                    if (*i == '=') // +=
                    {
                        token.type = Token::Type::OPERATOR_PLUS_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '+') // ++
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
                    if (*i == '=') // -=
                    {
                        token.type = Token::Type::OPERATOR_MINUS_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '-') // --
                    {
                        token.type = Token::Type::OPERATOR_DECREMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '>') // ->
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
                    if (*i == '=') // *=
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
                    if (*i == '=') // /=
                    {
                        token.type = Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '/') // single-line comment
                    {
                        ++i;

                        for (; i != code.end(); ++i)
                        {
                            if (*i == '\n') // reached newline
                            {
                                ++i; // skip the newline
                                break;
                            }
                        }

                        continue; // skip this token
                    }
                    else if (*i == '*') // multi-line comment
                    {
                        ++i;

                        bool terminated = false;
                        for (; i != code.end(); ++i)
                        {
                            if (*i == '*' && ++i != code.end() && *i == '/')
                            {
                                terminated = true;
                                ++i;
                                break;
                            }
                        }

                        if (!terminated)
                        {
                            std::cerr << "Unterminated block comment" << std::endl;
                            return false;
                        }

                        continue; // skip this token
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
                    if (*i == '=') // %=
                    {
                        token.type = Token::Type::OPERATOR_MODULO_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '>') // %>
                    {
                        token.kind = Token::Kind::PUNCTUATOR;
                        token.type = Token::Type::RIGHT_BRACE;
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
                    if (*i == '=') // ==
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
                    if (*i == '=') // &=
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_AND_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '&') // &&
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
                    if (*i == '=') // ~=
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
                    if (*i == '=') // ^=
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
                    if (*i == '=') // |=
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_OR_ASSIGNMENT;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '|') // ||
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
                    if (*i == '=') // <=
                    {
                        token.type = Token::Type::OPERATOR_LESS_THAN_EQUAL;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '<') // <<
                    {
                        token.type = Token::Type::OPERATOR_SHIFT_LEFT;
                        token.value.push_back(*i);
                        ++i;

                        if (i != code.end())
                        {
                            if (*i == '=') // <<=
                            {
                                token.type = Token::Type::OPERATOR_SHIFT_LEFT_ASSIGNMENT;
                                token.value.push_back(*i);
                                ++i;
                            }
                        }
                    }
                    else if (*i == '%') // <%
                    {
                        token.kind = Token::Kind::PUNCTUATOR;
                        token.type = Token::Type::LEFT_BRACE;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == ':') // <:
                    {
                        token.kind = Token::Kind::PUNCTUATOR;
                        token.type = Token::Type::LEFT_BRACKET;
                        token.value.push_back(*i);
                        ++i;
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
                    if (*i == '=') // >=
                    {
                        token.type = Token::Type::OPERATOR_GREATER_THAN_EQUAL;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else if (*i == '>') // >>
                    {
                        token.type = Token::Type::OPERATOR_SHIFT_RIGHT;
                        token.value.push_back(*i);
                        ++i;

                        if (i != code.end())
                        {
                            if (*i == '=') // >>=
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
                    if (*i == '=') // !=
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
