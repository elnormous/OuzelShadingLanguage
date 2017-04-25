//
//  main.cpp
//  OSLParser
//
//  Created by Elviss Strazdins on 24/04/2017.
//  Copyright © 2017 Elviss Strazdins. All rights reserved.
//

#include <iostream>
#include "Tokenizer.h"
#include "Parser.h"

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
        std::cout << "Token, type: " << tokenTypeToString(token.type) << ", value: " << token.value << std::endl;
    }

    ASTContext context;

    if (!parse(tokens, context))
    {
        std::cerr << "Dailed to parse" << std::endl;
        return EXIT_FAILURE;
    }

    dumpContext(context);

    return EXIT_SUCCESS;
}
