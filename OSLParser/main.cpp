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

struct Token
{
    enum class Type
    {
        NONE,
        PUNCTUATION,
        NUMBER,
        STRING,
        KEYWORD,
        IDENTIFIER,
        OPERATOR
    };

    Type type = Type::NONE;
    std::string value;
};

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

    for (std::vector<uint8_t>::iterator i = buffer.begin(); i != buffer.end(); ++i)
    {
        char c = static_cast<char>(*i);

        Token token;

        if (c == ',' || c == ';' ||
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
            token.type = Token::Type::NUMBER;

            bool dot = false;

            while (c >= '0' && c <= '9')
            {
                if (c == '.')
                {
                    if (dot)
                    {
                        std::cerr << "Invalid number" << std::endl;
                        return EXIT_FAILURE;
                    }
                    else
                    {
                        dot = true;
                    }
                }

                token.value.push_back(c);

                ++i;
                if (i == buffer.end()) break; // reached end of file
                c = static_cast<char>(*i);
            }
        }
        else if (c == '"') // string literal
        {
            token.type = Token::Type::STRING;

            ++i;
            if (i == buffer.end()) break; // reached end of file
            c = static_cast<char>(*i);

            while (c != '"')
            {
                token.value.push_back(c);

                ++i;
                if (i == buffer.end()) break; // reached end of file
                c = static_cast<char>(*i);
            }

            if (c != '"')
            {
                std::cerr << "Unterminated string" << std::endl;
                return EXIT_FAILURE;
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

                ++i;
                if (i == buffer.end()) break; // reached end of file
                c = static_cast<char>(*i);
            }

            if (token.value == "if" ||
                token.value == "else" ||
                token.value == "true" ||
                token.value == "false" ||
                token.value == "for" ||
                token.value == "while")
            {
                token.type = Token::Type::KEYWORD;
            }
            else
            {
                token.type = Token::Type::IDENTIFIER;
            }
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

                ++i;
                if (i == buffer.end()) break; // reached end of file
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
            return EXIT_FAILURE;
        }

        std::cout << "Token, type: ";

        switch (token.type)
        {
            case Token::Type::NONE: std::cout << "NONE"; break;
            case Token::Type::PUNCTUATION: std::cout << "PUNCTUATION"; break;
            case Token::Type::NUMBER: std::cout << "NUMBER"; break;
            case Token::Type::STRING: std::cout << "STRING"; break;
            case Token::Type::KEYWORD: std::cout << "KEYWORD"; break;
            case Token::Type::IDENTIFIER: std::cout << "IDENTIFIER"; break;
            case Token::Type::OPERATOR: std::cout << "OPERATOR"; break;
        }

        std::cout << ", value: " << token.value << std::endl;

        tokens.push_back(token);
    }

    return EXIT_SUCCESS;
}
