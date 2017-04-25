//
//  main.cpp
//  OSLParser
//
//  Created by Elviss Strazdins on 24/04/2017.
//  Copyright © 2017 Elviss Strazdins. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

std::vector<std::string> builtinTypes = {
    "bool", "int", "uint", "float", "double",
    "vec2", "vec3", "vec4", "mat3", "mat4"
};

struct Token
{
    enum class Type
    {
        NONE,
        PUNCTUATION,
        INT_LITERAL,
        FLOAT_LITERAL,
        STRING_LITERAL,
        KEYWORD_IF,
        KEYWORD_ELSE,
        KEYWORD_RETURN,
        KEYWORD_FOR,
        KEYWORD_WHILE,
        KEYWORD_TRUE,
        KEYWORD_FALSE,
        IDENTIFIER,
        OPERATOR
    };

    Type type = Type::NONE;
    std::string value;
};

bool tokenize(const std::vector<uint8_t>& buffer, std::vector<Token>& tokens)
{
    for (std::vector<uint8_t>::const_iterator i = buffer.begin(); i != buffer.end(); ++i)
    {
        char c = static_cast<char>(*i);

        Token token;

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
        else if (c == ',' || c == ';' ||
                 c == '(' || c == ')' ||
                 c == '{' || c == '}' ||
                 c == '[' || c == ']' ||
                 c == ':') // punctuation
        {
            token.type = Token::Type::PUNCTUATION;
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
            else if (token.value == "true") token.type = Token::Type::KEYWORD_TRUE;
            else if (token.value == "false") token.type = Token::Type::KEYWORD_FALSE;
            else token.type = Token::Type::IDENTIFIER;
        }
        else if (c == '+' || c == '-' ||
                 c == '*' || c == '/' ||
                 c == '%' || c == '=' ||
                 c == '&' || c == '|' ||
                 c == '<' || c == '>' ||
                 c == '!' || c == '.')
        {
            token.type = Token::Type::OPERATOR;

            while (c == '+' || c == '-' ||
                   c == '*' || c == '/' ||
                   c == '%' || c == '=' ||
                   c == '&' || c == '|' ||
                   c == '<' || c == '>' ||
                   c == '!' || c == '.')
            {
                token.value.push_back(c);

                if (++i == buffer.end()) break; // reached end of file
                c = static_cast<char>(*i);
            }
        }
        else if (c == ' ' || c == '\t' || c == '\n') // whitespace
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

struct ASTNode
{
    enum class Type
    {
        NONE,
        FUNCTION_DECLARATION,
        VARIABLE_DECLARATION,
        PARAMETER_DECLARATION,
        COMPOUND_STATEMENT,
        FUNCTION_CALL,
        OPERATOR,
    };

    Type type = Type::NONE;
    std::vector<ASTNode> children;
};

struct ASTContext
{
    std::vector<ASTNode> nodes;
};

bool parse(const std::vector<Token>& tokens, ASTContext& context)
{
    return true;
}

int main(int argc, const char * argv[])
{
    if (argc < 2)
    {
        std::cerr << "Too few arguments" << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<uint8_t> buffer;

    FILE* file = fopen(argv[1], "rb");

    if (!file)
    {
        std::cerr << "Failed to open file" << std::endl;
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer.resize(size);
    fread(&buffer[0], size, 1, file);
    fclose(file);

    std::vector<Token> tokens;

    if (!tokenize(buffer, tokens))
    {
        std::cerr << "Failed to tokenize" << std::endl;
        return EXIT_FAILURE;
    }

    for (const Token& token : tokens)
    {
        std::cout << "Token, type: ";

        switch (token.type)
        {
            case Token::Type::NONE: std::cout << "NONE"; break;
            case Token::Type::PUNCTUATION: std::cout << "PUNCTUATION"; break;
            case Token::Type::INT_LITERAL: std::cout << "INT_LITERAL"; break;
            case Token::Type::FLOAT_LITERAL: std::cout << "FLOAT_LITERAL"; break;
            case Token::Type::STRING_LITERAL: std::cout << "STRING_LITERAL"; break;
            case Token::Type::KEYWORD_IF: std::cout << "KEYWORD_IF"; break;
            case Token::Type::KEYWORD_ELSE: std::cout << "KEYWORD_ELSE"; break;
            case Token::Type::KEYWORD_RETURN: std::cout << "KEYWORD_RETURN"; break;
            case Token::Type::KEYWORD_FOR: std::cout << "KEYWORD_FOR"; break;
            case Token::Type::KEYWORD_WHILE: std::cout << "KEYWORD_WHILE"; break;
            case Token::Type::KEYWORD_TRUE: std::cout << "KEYWORD_TRUE"; break;
            case Token::Type::KEYWORD_FALSE: std::cout << "KEYWORD_FALSE"; break;
            case Token::Type::IDENTIFIER: std::cout << "IDENTIFIER"; break;
            case Token::Type::OPERATOR: std::cout << "OPERATOR"; break;
        }

        std::cout << ", value: " << token.value << std::endl;
    }

    ASTContext context;

    if (!parse(tokens, context))
    {
        std::cerr << "Dailed to parse" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
