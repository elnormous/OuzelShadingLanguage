//
//  OSL
//

#pragma once

#include <map>
#include <string>
#include "Output.hpp"
#include "Program.hpp"

class OutputGLSL: public Output
{
public:
    OutputGLSL(Program initProgram,
               uint32_t initGLSLVersion,
               const std::map<Semantic, std::string>& initSemantics);
    virtual std::string output(const ASTContext& context, bool whitespaces);

private:
    struct Options
    {
        Options(uint32_t initIndentation, bool initWhitespaces):
            indentation(initIndentation), whitespaces(initWhitespaces) {}

        uint32_t indentation = 0;
        bool whitespaces = false;
    };
    void printDeclaration(const Declaration* declaration, Options options, std::string& code);
    void printStatement(const Statement* statement, Options options, std::string& code);
    void printExpression(const Expression* expression, Options options, std::string& code);
    void printConstruct(const Construct* construct, Options options, std::string& code);

    Program program;
    uint32_t glslVersion;
    const std::map<Semantic, std::string> semantics;
};
