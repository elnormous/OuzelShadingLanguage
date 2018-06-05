//
//  OSL
//

#include <iostream>
#include <map>
#include "Tokenizer.hpp"

static const std::map<std::string, Token::Type> keywordMap = {
    {"and", Token::Type::KEYWORD_AND},
    {"and_eq", Token::Type::KEYWORD_AND},
    {"auto", Token::Type::KEYWORD_AUTO},
    {"bitand", Token::Type::KEYWORD_BITAND},
    {"bitor", Token::Type::KEYWORD_BITOR},
    {"bool", Token::Type::KEYWORD_BOOL},
    {"break", Token::Type::KEYWORD_BREAK},
    {"case", Token::Type::KEYWORD_CASE},
    {"catch", Token::Type::KEYWORD_CATCH},
    {"char", Token::Type::KEYWORD_CHAR},
    {"class", Token::Type::KEYWORD_CLASS},
    {"compl", Token::Type::KEYWORD_COMPL},
    {"const", Token::Type::KEYWORD_CONST},
    {"continue", Token::Type::KEYWORD_CONTINUE},
    {"default", Token::Type::KEYWORD_DEFAULT},
    {"delete", Token::Type::KEYWORD_DELETE},
    {"do", Token::Type::KEYWORD_DO},
    {"double", Token::Type::KEYWORD_DOUBLE},
    {"else", Token::Type::KEYWORD_ELSE},
    {"enum", Token::Type::KEYWORD_ENUM},
    {"explicit", Token::Type::KEYWORD_EXPLICIT},
    {"export", Token::Type::KEYWORD_EXPORT},
    {"extern", Token::Type::KEYWORD_EXTERN},
    {"false", Token::Type::KEYWORD_FALSE},
    {"float", Token::Type::KEYWORD_FLOAT},
    {"for", Token::Type::KEYWORD_FOR},
    {"friend", Token::Type::KEYWORD_FRIEND},
    {"goto", Token::Type::KEYWORD_GOTO},
    {"if", Token::Type::KEYWORD_IF},
    {"inline", Token::Type::KEYWORD_INLINE},
    {"int", Token::Type::KEYWORD_INT},
    {"long", Token::Type::KEYWORD_LONG},
    {"mutable", Token::Type::KEYWORD_MUTABLE},
    {"namespace", Token::Type::KEYWORD_NAMESPACE},
    {"new", Token::Type::KEYWORD_NEW},
    {"noexcept", Token::Type::KEYWORD_NOEXCEPT},
    {"not", Token::Type::KEYWORD_NOT},
    {"not_eq", Token::Type::KEYWORD_NOT_EQ},
    {"nullptr", Token::Type::KEYWORD_NULLPTR},
    {"operator", Token::Type::KEYWORD_OPERATOR},
    {"or", Token::Type::KEYWORD_OR},
    {"or_eq", Token::Type::KEYWORD_OR_EQ},
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
    {"typedef", Token::Type::KEYWORD_TYPEDEF},
    {"typeid", Token::Type::KEYWORD_TYPEID},
    {"typename", Token::Type::KEYWORD_TYPENAME},
    {"union", Token::Type::KEYWORD_UNION},
    {"unsigned", Token::Type::KEYWORD_UNSIGNED},
    {"using", Token::Type::KEYWORD_USING},
    {"virtual", Token::Type::KEYWORD_VIRTUAL},
    {"void", Token::Type::KEYWORD_VOID},
    {"volatile", Token::Type::KEYWORD_VOLATILE},
    {"while", Token::Type::KEYWORD_WHILE},
    {"xor", Token::Type::KEYWORD_XOR},
    {"xor_eq", Token::Type::KEYWORD_XOR_EQ}
};

std::vector<Token> tokenize(const std::vector<char>& code)
{
    std::vector<Token> tokens;
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
            bool integer = true;
            token.kind = Token::Kind::LITERAL;

            while (i != code.end() && (*i >= '0' && *i <= '9'))
            {
                token.value.push_back(*i);
                ++i;
            }

            if (i != code.end() && *i == '.')
            {
                integer = false;

                token.value.push_back(*i);
                ++i;

                while (i != code.end() && (*i >= '0' && *i <= '9'))
                {
                    token.value.push_back(*i);
                    ++i;
                }
            }

            // parse exponent
            if (i != code.end() &&
                (*i == 'e' || *i == 'E'))
            {
                integer = false;

                token.value.push_back(*i);
                ++i;

                if (i == code.end() || *i != '+' || *i != '-')
                    throw std::runtime_error("Invalid exponent");

                token.value.push_back(*i);
                ++i;

                if (i == code.end() || *i < '0' || *i > '9')
                    throw std::runtime_error("Invalid exponent");

                while (i != code.end() && (*i >= '0' && *i <= '9'))
                {
                    token.value.push_back(*i);
                    ++i;
                }
            }

            std::string suffix;

            while (i != code.end() && ((*i >= 'a' && *i <= 'z') || (*i >= 'A' && *i <= 'Z')))
            {
                suffix.push_back(*i);
                ++i;
            }

            if (suffix.empty())
            {
                if (integer) token.type = Token::Type::LITERAL_INT;
                else token.type = Token::Type::LITERAL_DOUBLE;
            }
            else if (suffix == "f" || suffix == "F")
            {
                if (integer) throw std::runtime_error("Invalid integer constant");
                else token.type = Token::Type::LITERAL_FLOAT;
            }
            else throw std::runtime_error("Invalid suffix " + suffix);
        }
        else if (*i == '"') // string literal
        {
            token.kind = Token::Kind::LITERAL;
            token.type = Token::Type::LITERAL_STRING;

            for (;;)
            {
                if (++i == code.end())
                    throw std::runtime_error("Unterminated string literal");

                if (*i == '"')
                {
                    ++i;
                    break;
                }
                else if (*i == '\\')
                {
                    if (++i == code.end())
                        throw std::runtime_error("Unterminated string literal");

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
                        throw std::runtime_error("Unrecognized escape character");
                    // TODO: handle numeric character references
                }
                else if (*i == '\n' || *i == '\r')
                    throw std::runtime_error("Unterminated string literal");
                else
                    token.value.push_back(*i);
            }
        }
        else if (*i == '\'') // char literal
        {
            token.kind = Token::Kind::LITERAL;
            token.type = Token::Type::LITERAL_CHAR;

            if (++i == code.end()) // reached end of file
                throw std::runtime_error("Unterminated char literal");

            if (*i == '\\')
            {
                if (++i == code.end())
                    throw std::runtime_error("Unterminated char literal");

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
                    throw std::runtime_error("Unrecognized escape character");
                // TODO: handle numeric character references
            }
            else
            {
                token.value.push_back(*i);
            }

            if (++i == code.end()) // reached end of file
                throw std::runtime_error("Unterminated char literal");

            if (*i != '\'')
                throw std::runtime_error("Invalid char literal");

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

            std::map<std::string, Token::Type>::const_iterator keywordIterator = keywordMap.find(token.value);

            if (keywordIterator != keywordMap.end())
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
                            throw std::runtime_error("Unterminated block comment");

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
                token.value.push_back(*i);
                ++i;

                if (i != code.end() && *i == '.' &&
                    (i + 1) != code.end() && *(i + 1) == '.')
                {
                    token.kind = Token::Kind::OPERATOR;
                    token.type = Token::Type::OPERATOR_ELLIPSIS;
                    token.value.push_back(*i);
                    ++i;
                    token.value.push_back(*i);
                    ++i;
                }
                else
                {
                    token.kind = Token::Kind::OPERATOR;
                    token.type = Token::Type::OPERATOR_DOT;
                }
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
            throw std::runtime_error("Unknown character");

        tokens.push_back(token);
    }

    return tokens;
}

static std::string toString(Token::Kind kind)
{
    switch (kind)
    {
        case Token::Kind::NONE: return "NONE";
        case Token::Kind::LITERAL: return "LITERAL";
        case Token::Kind::KEYWORD: return "KEYWORD";
        case Token::Kind::PUNCTUATOR: return "PUNCTUATOR";
        case Token::Kind::OPERATOR: return "OPERATOR";
        case Token::Kind::IDENTIFIER: return "IDENTIFIER";
        default: return "unknown";
    }
}

static std::string toString(Token::Type type)
{
    switch (type)
    {
        case Token::Type::NONE: return "NONE";
        case Token::Type::LITERAL_INT: return "LITERAL_INT";
        case Token::Type::LITERAL_FLOAT: return "LITERAL_FLOAT";
        case Token::Type::LITERAL_DOUBLE: return "LITERAL_DOUBLE";
        case Token::Type::LITERAL_CHAR: return "LITERAL_CHAR";
        case Token::Type::LITERAL_STRING: return "LITERAL_STRING";
        case Token::Type::KEYWORD_AND: return "KEYWORD_AND";
        case Token::Type::KEYWORD_AND_EQ: return "KEYWORD_AND_EQ";
        case Token::Type::KEYWORD_AUTO: return "KEYWORD_AUTO";
        case Token::Type::KEYWORD_BITAND: return "KEYWORD_BITAND";
        case Token::Type::KEYWORD_BITOR: return "KEYWORD_BITOR";
        case Token::Type::KEYWORD_BOOL: return "KEYWORD_BOOL";
        case Token::Type::KEYWORD_BREAK: return "KEYWORD_BREAK";
        case Token::Type::KEYWORD_CASE: return "KEYWORD_CASE";
        case Token::Type::KEYWORD_CATCH: return "KEYWORD_CATCH";
        case Token::Type::KEYWORD_CHAR: return "KEYWORD_CHAR";
        case Token::Type::KEYWORD_CLASS: return "KEYWORD_CLASS";
        case Token::Type::KEYWORD_COMPL: return "KEYWORD_COMPL";
        case Token::Type::KEYWORD_CONST: return "KEYWORD_CONST";
        case Token::Type::KEYWORD_CONTINUE: return "KEYWORD_CONTINUE";
        case Token::Type::KEYWORD_DEFAULT: return "KEYWORD_DEFAULT";
        case Token::Type::KEYWORD_DELETE: return "KEYWORD_DELETE";
        case Token::Type::KEYWORD_DO: return "KEYWORD_DO";
        case Token::Type::KEYWORD_DOUBLE: return "KEYWORD_DOUBLE";
        case Token::Type::KEYWORD_ELSE: return "KEYWORD_ELSE";
        case Token::Type::KEYWORD_ENUM: return "KEYWORD_ENUM";
        case Token::Type::KEYWORD_EXPLICIT: return "KEYWORD_EXPLICIT";
        case Token::Type::KEYWORD_EXPORT: return "KEYWORD_EXPORT";
        case Token::Type::KEYWORD_EXTERN: return "KEYWORD_EXTERN";
        case Token::Type::KEYWORD_FALSE: return "KEYWORD_FALSE";
        case Token::Type::KEYWORD_FLOAT: return "KEYWORD_FLOAT";
        case Token::Type::KEYWORD_FOR: return "KEYWORD_FOR";
        case Token::Type::KEYWORD_FRIEND: return "KEYWORD_FRIEND";
        case Token::Type::KEYWORD_GOTO: return "KEYWORD_GOTO";
        case Token::Type::KEYWORD_IF: return "KEYWORD_IF";
        case Token::Type::KEYWORD_INLINE: return "KEYWORD_INLINE";
        case Token::Type::KEYWORD_INT: return "KEYWORD_INT";
        case Token::Type::KEYWORD_LONG: return "KEYWORD_LONG";
        case Token::Type::KEYWORD_MUTABLE: return "KEYWORD_MUTABLE";
        case Token::Type::KEYWORD_NAMESPACE: return "KEYWORD_NAMESPACE";
        case Token::Type::KEYWORD_NEW: return "KEYWORD_NEW";
        case Token::Type::KEYWORD_NOEXCEPT: return "KEYWORD_NOEXCEPT";
        case Token::Type::KEYWORD_NOT: return "KEYWORD_NOT";
        case Token::Type::KEYWORD_NOT_EQ: return "KEYWORD_NOT_EQ";
        case Token::Type::KEYWORD_NULLPTR: return "KEYWORD_NULLPTR";
        case Token::Type::KEYWORD_OPERATOR: return "KEYWORD_OPERATOR";
        case Token::Type::KEYWORD_OR: return "KEYWORD_OR";
        case Token::Type::KEYWORD_OR_EQ: return "KEYWORD_OR_EQ";
        case Token::Type::KEYWORD_PRIVATE: return "KEYWORD_PRIVATE";
        case Token::Type::KEYWORD_PROTECTED: return "KEYWORD_PROTECTED";
        case Token::Type::KEYWORD_PUBLIC: return "KEYWORD_PUBLIC";
        case Token::Type::KEYWORD_RETURN: return "KEYWORD_RETURN";
        case Token::Type::KEYWORD_SHORT: return "KEYWORD_SHORT";
        case Token::Type::KEYWORD_SIGNED: return "KEYWORD_SIGNED";
        case Token::Type::KEYWORD_SIZEOF: return "KEYWORD_SIZEOF";
        case Token::Type::KEYWORD_STATIC: return "KEYWORD_STATIC";
        case Token::Type::KEYWORD_STRUCT: return "KEYWORD_STRUCT";
        case Token::Type::KEYWORD_SWITCH: return "KEYWORD_SWITCH";
        case Token::Type::KEYWORD_TEMPLATE: return "KEYWORD_TEMPLATE";
        case Token::Type::KEYWORD_THIS: return "KEYWORD_THIS";
        case Token::Type::KEYWORD_THROW: return "KEYWORD_THROW";
        case Token::Type::KEYWORD_TRUE: return "KEYWORD_TRUE";
        case Token::Type::KEYWORD_TRY: return "KEYWORD_TRY";
        case Token::Type::KEYWORD_TYPEDEF: return "KEYWORD_TYPEDEF";
        case Token::Type::KEYWORD_TYPEID: return "KEYWORD_TYPEID";
        case Token::Type::KEYWORD_TYPENAME: return "KEYWORD_TYPENAME";
        case Token::Type::KEYWORD_UNION: return "KEYWORD_UNION";
        case Token::Type::KEYWORD_UNSIGNED: return "KEYWORD_UNSIGNED";
        case Token::Type::KEYWORD_USING: return "KEYWORD_USING";
        case Token::Type::KEYWORD_VIRTUAL: return "KEYWORD_VIRTUAL";
        case Token::Type::KEYWORD_VOID: return "KEYWORD_VOID";
        case Token::Type::KEYWORD_VOLATILE: return "KEYWORD_VOLATILE";
        case Token::Type::KEYWORD_WHILE: return "KEYWORD_WHILE";
        case Token::Type::KEYWORD_XOR: return "KEYWORD_XOR";
        case Token::Type::KEYWORD_XOR_EQ: return "KEYWORD_XOR_EQ";
        case Token::Type::LEFT_PARENTHESIS: return "LEFT_PARENTHESIS";
        case Token::Type::RIGHT_PARENTHESIS: return "RIGHT_PARENTHESIS";
        case Token::Type::LEFT_BRACE: return "LEFT_BRACE";
        case Token::Type::RIGHT_BRACE: return "RIGHT_BRACE";
        case Token::Type::LEFT_BRACKET: return "LEFT_BRACKET";
        case Token::Type::RIGHT_BRACKET: return "RIGHT_BRACKET";
        case Token::Type::COMMA: return "COMMA";
        case Token::Type::SEMICOLON: return "SEMICOLON";
        case Token::Type::COLON: return "COLON";
        case Token::Type::IDENTIFIER: return "IDENTIFIER";
        case Token::Type::OPERATOR_PLUS: return "OPERATOR_PLUS";
        case Token::Type::OPERATOR_MINUS: return "OPERATOR_MINUS";
        case Token::Type::OPERATOR_MULTIPLY: return "OPERATOR_MULTIPLY";
        case Token::Type::OPERATOR_DIVIDE: return "OPERATOR_DIVIDE";
        case Token::Type::OPERATOR_MODULO: return "OPERATOR_MODULO";
        case Token::Type::OPERATOR_INCREMENT: return "OPERATOR_INCREMENT";
        case Token::Type::OPERATOR_DECREMENT: return "OPERATOR_DECREMENT";
        case Token::Type::OPERATOR_ASSIGNMENT: return "OPERATOR_ASSIGNMENT";
        case Token::Type::OPERATOR_PLUS_ASSIGNMENT: return "OPERATOR_PLUS_ASSIGNMENT";
        case Token::Type::OPERATOR_MINUS_ASSIGNMENT: return "OPERATOR_MINUS_ASSIGNMENT";
        case Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT: return "OPERATOR_MULTIPLY_ASSIGNMENT";
        case Token::Type::OPERATOR_DIVIDE_ASSIGNMENT: return "OPERATOR_DIVIDE_ASSIGNMENT";
        case Token::Type::OPERATOR_MODULO_ASSIGNMENT: return "OPERATOR_MODULO_ASSIGNMENT";
        case Token::Type::OPERATOR_BITWISE_AND_ASSIGNMENT: return "OPERATOR_BITWISE_AND_ASSIGNMENT";
        case Token::Type::OPERATOR_BITWISE_OR_ASSIGNMENT: return "OPERATOR_BITWISE_OR_ASSIGNMENT";
        case Token::Type::OPERATOR_BITWISE_NOT_ASSIGNMENT: return "OPERATOR_BITWISE_NOT_ASSIGNMENT";
        case Token::Type::OPERATOR_BITWISE_XOR_ASSIGNMENT: return "OPERATOR_BITWISE_XOR_ASSIGNMENT";
        case Token::Type::OPERATOR_SHIFT_RIGHT_ASSIGNMENT: return "OPERATOR_SHIFT_RIGHT_ASSIGNMENT";
        case Token::Type::OPERATOR_SHIFT_LEFT_ASSIGNMENT: return "OPERATOR_SHIFT_LEFT_ASSIGNMENT";
        case Token::Type::OPERATOR_BITWISE_AND: return "OPERATOR_BITWISE_AND";
        case Token::Type::OPERATOR_BITWISE_OR: return "OPERATOR_BITWISE_OR";
        case Token::Type::OPERATOR_BITWISE_NOT: return "OPERATOR_BITWISE_NOT";
        case Token::Type::OPERATOR_BITWISE_XOR: return "OPERATOR_BITWISE_XOR";
        case Token::Type::OPERATOR_SHIFT_RIGHT: return "OPERATOR_SHIFT_RIGHT";
        case Token::Type::OPERATOR_SHIFT_LEFT: return "OPERATOR_SHIFT_LEFT";
        case Token::Type::OPERATOR_EQUAL: return "OPERATOR_EQUAL";
        case Token::Type::OPERATOR_NOT_EQUAL: return "OPERATOR_NOT_EQUAL";
        case Token::Type::OPERATOR_LESS_THAN: return "OPERATOR_LESS_THAN";
        case Token::Type::OPERATOR_GREATER_THAN: return "OPERATOR_GREATER_THAN";
        case Token::Type::OPERATOR_LESS_THAN_EQUAL: return "OPERATOR_LESS_THAN_EQUAL";
        case Token::Type::OPERATOR_GREATER_THAN_EQUAL: return "OPERATOR_GREATER_THAN_EQUAL";
        case Token::Type::OPERATOR_AND: return "OPERATOR_AND";
        case Token::Type::OPERATOR_OR: return "OPERATOR_OR";
        case Token::Type::OPERATOR_NOT: return "OPERATOR_NOT";
        case Token::Type::OPERATOR_CONDITIONAL: return "OPERATOR_CONDITIONAL";
        case Token::Type::OPERATOR_DOT: return "OPERATOR_DOT";
        case Token::Type::OPERATOR_ARROW: return "OPERATOR_ARROW";
        case Token::Type::OPERATOR_ELLIPSIS: return "OPERATOR_ELLIPSIS";
    }

    return "unknown";
}

void dump(const std::vector<Token>& tokens)
{
    for (const Token& token : tokens)
    {
        std::cout << "Token, kind: " << toString(token.kind) <<
            ", type: " << toString(token.type) <<
            ", value: " << token.value <<
            ", line: " << token.line <<
            ", column: " << token.column << std::endl;
    }
}
