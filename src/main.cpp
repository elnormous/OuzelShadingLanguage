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

int main(int argc, const char * argv[])
{
    std::string inputFile;
    bool printTokens = false;
    bool printAST = false;
    std::string format;
    std::string outputFile;
    Program program = Program::NONE;

    for (int i = 0; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--input")
        {
            if (++i < argc) inputFile = argv[i];
            else
            {
                std::cerr << "Argument to " << argv[i] << " is missing" << std::endl;
                return EXIT_FAILURE;
            }
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
            else
            {
                std::cerr << "Argument to " << argv[i] << " is missing" << std::endl;
                return EXIT_FAILURE;
            }
        }
        else if (std::string(argv[i]) == "--output")
        {
            if (++i < argc) outputFile = argv[i];
            else
            {
                std::cerr << "Argument to " << argv[i] << " is missing" << std::endl;
                return EXIT_FAILURE;
            }
        }
        else if (std::string(argv[i]) == "--program")
        {
            if (++i < argc)
            {
                if (std::string(argv[i]) == "fragment") program = Program::FRAGMENT;
                else if (std::string(argv[i]) == "vertex") program = Program::VERTEX;
                else
                {
                    std::cerr << "Invalid program " << argv[i] << std::endl;
                    return EXIT_FAILURE;
                }
            }
            else
            {
                std::cerr << "Argument to " << argv[i] << " is missing" << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    if (inputFile.empty())
    {
        std::cerr << "No input file" << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<char> code;

    std::ifstream file(inputFile, std::ios::binary);

    if (!file)
    {
        std::cerr << "Failed to open file " << inputFile << std::endl;
        return EXIT_FAILURE;
    }

    code.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    std::vector<Token> tokens;

    if (!tokenize(code, tokens))
    {
        std::cerr << "Failed to tokenize" << std::endl;
        return EXIT_FAILURE;
    }

    if (printTokens)
    {
        dump(tokens);
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
        if (program == Program::NONE)
        {
            std::cerr << "No program" << std::endl;
            return EXIT_FAILURE;
        }

        std::ofstream file(outputFile, std::ios::binary);

        if (!file)
        {
            std::cerr << "Failed to open file " << outputFile << std::endl;
            return EXIT_FAILURE;
        }

        std::unique_ptr<Output> output;

        if (outputFile.empty())
        {
            std::cerr << "No output file" << std::endl;
            return EXIT_FAILURE;
        }

        if (format == "hlsl")
        {
            output.reset(new OutputHLSL(program));
        }
        else if (format == "glsl")
        {
            output.reset(new OutputGLSL(program, 110, {}));
        }
        else if (format == "msl")
        {
            output.reset(new OutputMSL(program, {}));
        }
        else
        {
            std::cerr << "Invalid format" << std::endl;
            return EXIT_FAILURE;
        }

        std::string code;

        if (!output->output(context, code))
        {
            std::cerr << "Failed to output code" << std::endl;
            return EXIT_FAILURE;
        }

        file << code;
    }

    return EXIT_SUCCESS;
}
