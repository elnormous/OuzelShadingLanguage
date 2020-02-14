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

namespace
{
    enum class OutputProgram
    {
        None,
        Fragment,
        Vertex
    };

    constexpr ouzel::Program getProgram(OutputProgram outputProgram)
    {
        return (outputProgram == OutputProgram::Fragment) ? ouzel::Program::Fragment :
            (outputProgram == OutputProgram::Vertex) ? ouzel::Program::Vertex :
            throw std::runtime_error("Invalid program");
    }
}

int main(int argc, const char* argv[])
{
    std::string inputFilename;
    bool printTokens = false;
    bool printAST = false;
    bool whitespaces = false;
    bool preprocess = false;
    std::string format;
    std::string outputFilename;
    uint32_t outputVersion = 0;
    OutputProgram program = OutputProgram::None;
    std::string mainFunction;

    try
    {
        for (int i = 1; i < argc; ++i)
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
            else if (std::string(argv[i]) == "--preprocess")
                preprocess = true;
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
                
                if (std::string(argv[i]) == "fragment") program = OutputProgram::Fragment;
                else if (std::string(argv[i]) == "vertex") program = OutputProgram::Vertex;
                else
                    throw std::runtime_error("Invalid program: " + std::string(argv[i]));
            }
            else if (std::string(argv[i]) == "--main")
            {
                if (++i >= argc)
                    throw std::runtime_error("Argument to " + std::string(argv[i]) + " is missing");
                mainFunction = argv[i];
            }
            else
                throw std::runtime_error("Invalid argument " + std::string(argv[i]));
        }

        if (inputFilename.empty())
            throw std::runtime_error("No input file");

        if (mainFunction.empty())
            throw std::runtime_error("No main function");

        std::ifstream inputFile(inputFilename, std::ios::binary);

        if (!inputFile)
            throw std::runtime_error("Failed to open file " + inputFilename);

        std::string inCode;
        inCode.assign(std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>());

        ouzel::Preprocessor preprocessor;
        auto preprocessed = preprocessor.preprocess(inCode);

        if (preprocess)
        {
            std::cout << std::string(preprocessed.begin(), preprocessed.end()) << "\n";
        }
        else
        {
            std::vector<ouzel::Token> tokens = ouzel::tokenize(preprocessed);

            if (printTokens)
                dump(tokens);
            else
            {
                ouzel::ASTContext context(tokens);

                if (printAST)
                    context.dump();
                else
                {
                    std::unique_ptr<ouzel::Output> output;

                    if (format.empty())
                        throw std::runtime_error("No format");
                    if (format == "hlsl")
                        output.reset(new ouzel::OutputHLSL(getProgram(program), mainFunction));
                    else if (format == "glsl")
                        output.reset(new ouzel::OutputGLSL(getProgram(program), mainFunction, outputVersion));
                    else if (format == "msl")
                        output.reset(new ouzel::OutputMSL(getProgram(program), mainFunction));
                    else
                        throw std::runtime_error("Invalid format");

                    try
                    {
                        std::string outCode = output->output(context, whitespaces);

                        if (outputFilename.empty())
                            std::cout << outCode << '\n';
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
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
