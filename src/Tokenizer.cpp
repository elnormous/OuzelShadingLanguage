//
//  OSL
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

    {"auto", Token::Type::KEYWORD_AUTO},
    {"bool", Token::Type::KEYWORD_BOOL},
    {"break", Token::Type::KEYWORD_BREAK},
    {"case", Token::Type::KEYWORD_CASE},
    {"catch", Token::Type::KEYWORD_CATCH},
    {"char", Token::Type::KEYWORD_CHAR},
    {"class", Token::Type::KEYWORD_CLASS},
    {"continue", Token::Type::KEYWORD_CONTINUE},
    {"const", Token::Type::KEYWORD_CONST},
    {"default", Token::Type::KEYWORD_DEFAULT},
    {"delete", Token::Type::KEYWORD_DELETE},
    {"do", Token::Type::KEYWORD_DO},
    {"double", Token::Type::KEYWORD_DOUBLE},
    {"else", Token::Type::KEYWORD_ELSE},
    {"enum", Token::Type::KEYWORD_ENUM},
    {"extern", Token::Type::KEYWORD_EXTERN},
    {"false", Token::Type::KEYWORD_FALSE},
    {"float", Token::Type::KEYWORD_FLOAT},
    {"for", Token::Type::KEYWORD_FOR},
    {"goto", Token::Type::KEYWORD_GOTO},
    {"if", Token::Type::KEYWORD_IF},
    {"inline", Token::Type::KEYWORD_INLINE},
    {"int", Token::Type::KEYWORD_INT},
    {"long", Token::Type::KEYWORD_LONG},
    {"mutable", Token::Type::KEYWORD_MUTABLE},
    {"namespace", Token::Type::KEYWORD_NAMESPACE},
    {"new", Token::Type::KEYWORD_NEW},
    {"noexcept", Token::Type::KEYWORD_NOEXCEPT},
    {"operator", Token::Type::KEYWORD_OPERATOR},
    {"private", Token::Type::KEYWORD_PRIVATE},
    {"protected", Token::Type::KEYWORD_PROTECTED},
    {"public", Token::Type::KEYWORD_PUBLIC},
    {"return", Token::Type::KEYWORD_RETURN},
    {"short", Token::Type::KEYWORD_SHORT},
    {"signed", Token::Type::KEYWORD_SIGNED},
    {"sizeof", Token::Type::KEYWORD_SIZEOF},
    {"static", Token::Type::KEYWORD_STATIC},
    {"struct", Token::Type::KEYWORD_STRUCT},
    {"switch", Token::Type::KEYWORD_SWITCH},
    {"template", Token::Type::KEYWORD_TEMPLATE},
    {"this", Token::Type::KEYWORD_THIS},
    {"throw", Token::Type::KEYWORD_THROW},
    {"true", Token::Type::KEYWORD_TRUE},
    {"try", Token::Type::KEYWORD_TRY},
    {"type", Token::Type::KEYWORD_TYPE},
    {"typedef", Token::Type::KEYWORD_TYPEDEF},
    {"union", Token::Type::KEYWORD_UNION},
    {"unsigned", Token::Type::KEYWORD_UNSIGNED},
    {"using", Token::Type::KEYWORD_USING},
    {"virtual", Token::Type::KEYWORD_VIRTUAL},
    {"void", Token::Type::KEYWORD_VOID},
    {"while", Token::Type::KEYWORD_WHILE}
};

bool tokenize(const std::vector<char>& code, std::vector<Token>& tokens)
{
    uint32_t line = 1;
    std::vector<char>::const_iterator lineStart = code.begin();

    for (std::vector<char>::const_iterator i = code.begin(); i != code.end();)
    {
        Token token;
        token.line = line;
        token.column = static_cast<uint32_t>(i - lineStart) + 1;

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

            while (i != code.end() &&
                   ((*i >= '0' && *i <= '9')))
            {
                token.value.push_back(*i);
                ++i;
            }

            if (i != code.end() && *i == '.')
            {
                token.type = Token::Type::LITERAL_FLOAT;

                token.value.push_back(*i);
                ++i;

                while (i != code.end() &&
                       ((*i >= '0' && *i <= '9')))
                {
                    token.value.push_back(*i);
                    ++i;
                }
            }

            // parse exponent
            if (i != code.end() &&
                (*i == 'e' || *i == 'E'))
            {
                token.value.push_back(*i);
                ++i;

                if (i == code.end() || *i != '+' || *i != '-')
                {
                    std::cerr << "Invalid exponent" << std::endl;
                    return false;
                }

                token.value.push_back(*i);
                ++i;

                if (i == code.end() || *i < '0' || *i > '9')
                {
                    std::cerr << "Invalid exponent" << std::endl;
                    return false;
                }

                while (i != code.end() &&
                       ((*i >= '0' && *i <= '9')))
                {
                    token.value.push_back(*i);
                    ++i;
                }
            }
        }
        else if (*i == '"') // string literal
        {
            token.kind = Token::Kind::LITERAL;
            token.type = Token::Type::LITERAL_STRING;

            for (;;)
            {
                if (++i == code.end())
                {
                    std::cerr << "Unterminated string literal" << std::endl;
                    return false;
                }

                if (*i == '"')
                {
                    ++i;
                    break;
                }
                else if (*i == '\\')
                {
                    if (++i == code.end())
                    {
                        std::cerr << "Unterminated string literal" << std::endl;
                        return false;
                    }

                    if (*i == 'a') token.value.push_back('\a');
                    else if (*i == 'b') token.value.push_back('\b');
                    else if (*i == 't') token.value.push_back('\t');
                    else if (*i == 'n') token.value.push_back('\n');
                    else if (*i == 'v') token.value.push_back('\v');
                    else if (*i == 'f') token.value.push_back('\f');
                    else if (*i == 'r') token.value.push_back('\r');
                    else if (*i == '"') token.value.push_back('"');
                    else if (*i == '\?') token.value.push_back('\?');
                    else if (*i == '\\') token.value.push_back('\\');
                    else
                    {
                        std::cerr << "Unrecognized escape character" << std::endl;
                        return false;
                    }
                    // TODO: handle numeric character references
                }
                else if (*i == '\n' || *i == '\r')
                {
                    std::cerr << "Unterminated string literal" << std::endl;
                    return false;
                }
                else
                {
                    token.value.push_back(*i);
                }
            }
        }
        else if (*i == '\'') // char literal
        {
            token.kind = Token::Kind::LITERAL;
            token.type = Token::Type::LITERAL_CHAR;

            if (++i == code.end()) // reached end of file
            {
                std::cerr << "Unterminated char literal" << std::endl;
                return false;
            }

            if (*i == '\\')
            {
                if (++i == code.end())
                {
                    std::cerr << "Unterminated char literal" << std::endl;
                    return false;
                }

                if (*i == 'a') token.value.push_back('\a');
                else if (*i == 'b') token.value.push_back('\b');
                else if (*i == 't') token.value.push_back('\t');
                else if (*i == 'n') token.value.push_back('\n');
                else if (*i == 'v') token.value.push_back('\v');
                else if (*i == 'f') token.value.push_back('\f');
                else if (*i == 'r') token.value.push_back('\r');
                else if (*i == '\'') token.value.push_back('\'');
                else if (*i == '\?') token.value.push_back('\?');
                else if (*i == '\\') token.value.push_back('\\');
                else
                {
                    std::cerr << "Unrecognized escape character" << std::endl;
                    return false;
                }
                // TODO: handle numeric character references
            }
            else
            {
                token.value.push_back(*i);
            }

            if (++i == code.end()) // reached end of file
            {
                std::cerr << "Unterminated char literal" << std::endl;
                return false;
            }

            if (*i != '\'')
            {
                std::cerr << "Invalid char literal" << std::endl;
                return false;
            }

            ++i;
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
                 *i == '~' || *i == '^' ||
                 *i == '?')
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
                                ++line;
                                lineStart = i;
                                break;
                            }
                            else if (*i == '\r') // end of the comment
                            {
                                ++i;
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
                            if (*i == '\n') // reached newline
                            {
                                ++line;
                                lineStart = i;
                            }

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
        else if (*i == '\n')
        {
            ++i;
            ++line;
            lineStart = i;
            continue;
        }
        else if (*i == ' ' || *i == '\t' || *i == '\r') // whitespace
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
