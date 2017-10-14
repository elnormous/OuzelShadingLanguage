//
//  main.cpp
//  OSLParser
//
//  Created by Elviss Strazdins on 24/04/2017.
//  Copyright © 2017 Elviss Strazdins. All rights reserved.
//

#include <iostream>
#include "Tokenizer.hpp"
#include "Parser.hpp"
#include "OutputHLSL.hpp"
#include "OutputGLSL.hpp"
#include "OutputMetal.hpp"

int main(int argc, const char * argv[])
{
    std::string inputFile;
    bool printTokens = false;
    bool printAST = false;
    std::string format;
    std::string outputFile;

    for (int i = 0; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--input")
        {
            if (++i < argc) inputFile = argv[i];
        }
        else if (std::string(argv[i]) == "--print-tokens")
        {
            printTokens = true;
        }
        else if (std::string(argv[i]) == "--print-ast")
        {
            printAST = true;
        }
        else if (std::string(argv[i]) == "--format")
        {
            if (++i < argc) format = argv[i];
        }
        else if (std::string(argv[i]) == "--output")
        {
            if (++i < argc) outputFile = argv[i];
        }
    }

    if (inputFile.empty())
    {
        std::cerr << "No input file" << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<char> code;

    FILE* file = fopen(inputFile.c_str(), "rb");

    if (!file)
    {
        std::cerr << "Failed to open file" << std::endl;
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    code.resize(size);
    fread(&code[0], size, 1, file);
    fclose(file);

    std::vector<Token> tokens;

    if (!tokenize(code, tokens))
    {
        std::cerr << "Failed to tokenize" << std::endl;
        return EXIT_FAILURE;
    }

    if (printTokens)
    {
        for (const Token& token : tokens)
        {
            std::cout << "Token, kind: " << tokenKindToString(token.kind) << ", type: " << tokenTypeToString(token.type) << ", value: " << token.value << std::endl;
        }
    }

    ASTContext context;

    if (!context.parse(tokens))
    {
        std::cerr << "Failed to parse" << std::endl;

        std::cout << "Parsed so far:" << std::endl;
        context.dump();

        return EXIT_FAILURE;
    }

    if (printAST)
    {
        context.dump();
    }

    if (!format.empty())
    {
        std::unique_ptr<Output> output;

        if (outputFile.empty())
        {
            std::cerr << "No output file" << std::endl;
            return EXIT_FAILURE;
        }

        if (format == "hlsl")
        {
            output.reset(new OutputHLSL());
        }
        else if (format == "glsl")
        {
            output.reset(new OutputGLSL());
        }
        else if (format == "metal")
        {
            output.reset(new OutputMetal());
        }
        else
        {
            std::cerr << "Invalid format" << std::endl;
            return EXIT_FAILURE;
        }

        if (!output->output(context, outputFile))
        {
            std::cerr << "Failed to output code" << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}