//
//  OSL
//

#include <fstream>
#include <iostream>
#include "Tokenizer.hpp"
#include "Parser.hpp"
#include "Preprocessor.hpp"
#include "OutputHLSL.hpp"
#include "OutputGLSL.hpp"
#include "OutputMSL.hpp"

int main(int argc, const char* argv[])
{
    std::string inputFilename;
    bool printTokens = false;
    bool printAST = false;
    bool whitespaces = false;
    std::string format;
    std::string outputFilename;
    uint32_t outputVersion = 0;
    Program program = Program::NONE;

    try
    {
        for (int i = 0; i < argc; ++i)
        {
            if (std::string(argv[i]) == "--input")
            {
                if (++i >= argc)
                    throw std::runtime_error("Argument to " + std::string(argv[i]) + " is missing");
                inputFilename = argv[i];
            }
            else if (std::string(argv[i]) == "--print-tokens")
                printTokens = true;
            else if (std::string(argv[i]) == "--print-ast")
                printAST = true;
            else if (std::string(argv[i]) == "--whitespaces")
                whitespaces = true;
            else if (std::string(argv[i]) == "--format")
            {
                if (++i >= argc)
                    throw std::runtime_error("Argument to " + std::string(argv[i]) + " is missing");
                format = argv[i];
            }
            else if (std::string(argv[i]) == "--output")
            {
                if (++i >= argc)
                    throw std::runtime_error("Argument to " + std::string(argv[i]) + " is missing");
                outputFilename = argv[i];
            }
            else if (std::string(argv[i]) == "--output-version")
            {
                if (++i >= argc)
                    throw std::runtime_error("Argument to " + std::string(argv[i]) + " is missing");
                outputVersion = std::stoi(argv[i]);
            }
            else if (std::string(argv[i]) == "--program")
            {
                if (++i >= argc)
                    throw std::runtime_error("Argument to " + std::string(argv[i]) + " is missing");
                
                if (std::string(argv[i]) == "fragment") program = Program::FRAGMENT;
                else if (std::string(argv[i]) == "vertex") program = Program::VERTEX;
                else
                    throw std::runtime_error("Invalid program: " + std::string(argv[i]));
            }
        }

        if (inputFilename.empty())
            throw std::runtime_error("No input file");

        std::ifstream inputFile(inputFilename, std::ios::binary);

        if (!inputFile)
            throw std::runtime_error("Failed to open file " + inputFilename);

        std::vector<char> inCode;
        inCode.assign(std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>());

        Preprocessor preprocessor;

        std::vector<Token> tokens = tokenize(preprocessor.preprocess(inCode));

        if (printTokens)
            dump(tokens);
        else
        {
            ASTContext context(tokens);

            if (printAST)
                context.dump();
            else
            {
                if (program == Program::NONE)
                    throw std::runtime_error("No program");

                std::unique_ptr<Output> output;

                if (format.empty())
                    throw std::runtime_error("No format");
                if (format == "hlsl")
                    output.reset(new OutputHLSL(program));
                else if (format == "glsl")
                    output.reset(new OutputGLSL(program, outputVersion, {}));
                else if (format == "msl")
                    output.reset(new OutputMSL(program, {}));
                else
                    throw std::runtime_error("Invalid format");

                try
                {
                    std::string outCode = output->output(context, whitespaces);

                    if (outputFilename.empty())
                        std::cout << outCode << std::endl;
                    else
                    {
                        std::ofstream outputFile(outputFilename, std::ios::binary);

                        if (!outputFile)
                            throw std::runtime_error("Failed to open file " + outputFilename);

                        outputFile << outCode;
                    }
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(std::string("Failed to output code: ") + e.what());
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
