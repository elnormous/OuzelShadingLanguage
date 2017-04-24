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

    for (uint32_t i = 0; i < buffer.size(); ++i)
    {
        char c = buffer[i];

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
                if (buffer[i] == '.')
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
                if (i == buffer.size()) break; // reached end of file
                c = buffer[i];
            }
        }
        else if (c == '"') // string literal
        {
            token.type = Token::Type::STRING;

            ++i;
            if (i == buffer.size()) break; // reached end of file
            c = buffer[i];

            bool end = false;

            while (i < buffer.size())
            {
                if (c == '"')
                {
                    end = true;
                    break; // end of string
                }

                token.value.push_back(c);

                ++i;
                if (i == buffer.size()) break; // reached end of file
                c = buffer[i];
            }

            if (!end)
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
                if (i == buffer.size()) break; // reached end of file
                c = buffer[i];
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
                if (i == buffer.size()) break; // reached end of file
                c = buffer[i];
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

        tokens.push_back(token);
    }

    return EXIT_SUCCESS;
}
