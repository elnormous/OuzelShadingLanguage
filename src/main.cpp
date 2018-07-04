//
//  OSL
//

#include <fstream>
#include <iostream>
#include "Tokenizer.hpp"
#include "Parser.hpp"
#include "OutputHLSL.hpp"
#include "OutputGLSL.hpp"
#include "OutputMSL.hpp"

int main(int argc, const char* argv[])
{
    std::string inputFile;
    bool printTokens = false;
    bool printAST = false;
    bool whitespaces = false;
    std::string format;
    std::string outputFile;
    Program program = Program::NONE;

    try
    {
        for (int i = 0; i < argc; ++i)
        {
            if (std::string(argv[i]) == "--input")
            {
                if (++i < argc) inputFile = argv[i];
                else
                    throw std::runtime_error("Argument to " + std::string(argv[i]) + " is missing");
            }
            else if (std::string(argv[i]) == "--print-tokens")
            {
                printTokens = true;
            }
            else if (std::string(argv[i]) == "--print-ast")
            {
                printAST = true;
            }
            else if (std::string(argv[i]) == "--whitespaces")
            {
                if (++i < argc) whitespaces = true;
            }
            else if (std::string(argv[i]) == "--format")
            {
                if (++i < argc) format = argv[i];
                else
                    throw std::runtime_error("Argument to " + std::string(argv[i]) + " is missing");
            }
            else if (std::string(argv[i]) == "--output")
            {
                if (++i < argc) outputFile = argv[i];
                else
                    throw std::runtime_error("Argument to " + std::string(argv[i]) + " is missing");
            }
            else if (std::string(argv[i]) == "--program")
            {
                if (++i < argc)
                {
                    if (std::string(argv[i]) == "fragment") program = Program::FRAGMENT;
                    else if (std::string(argv[i]) == "vertex") program = Program::VERTEX;
                    else
                        throw std::runtime_error("Invalid program: " + std::string(argv[i]));
                }
                else
                    throw std::runtime_error("Argument to " + std::string(argv[i]) + " is missing");
            }
        }

        if (inputFile.empty())
            throw std::runtime_error("No input file");

        std::vector<char> code;

        std::ifstream file(inputFile, std::ios::binary);

        if (!file)
            throw std::runtime_error("Failed to open file " + inputFile);

        code.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

        std::vector<Token> tokens;

        try
        {
            tokens = tokenize(code);
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(std::string("Failed to tokenize: ") + e.what());
        }

        if (printTokens) dump(tokens);

        try
        {
            ASTContext context(tokens);

            if (printAST) context.dump();

            if (!format.empty())
            {
                if (program == Program::NONE)
                    throw std::runtime_error("No program");

                std::unique_ptr<Output> output;

                if (format == "hlsl")
                    output.reset(new OutputHLSL(program));
                else if (format == "glsl")
                    output.reset(new OutputGLSL(program, 110, {}));
                else if (format == "msl")
                    output.reset(new OutputMSL(program, {}));
                else
                    throw std::runtime_error("Invalid format");

                try
                {
                    std::string code = output->output(context, whitespaces);

                    if (outputFile.empty())
                        std::cout << code << std::endl;
                    else
                    {
                        std::ofstream file(outputFile, std::ios::binary);

                        if (!file)
                            throw std::runtime_error("Failed to open file " + outputFile);

                        file << code;
                    }
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(std::string("Failed to output code: ") + e.what());
                }
            }
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(std::string("Failed to parse: ") + e.what());
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
