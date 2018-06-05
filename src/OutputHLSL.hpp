//
//  OSL
//

#pragma once

#include "Output.hpp"
#include "Program.hpp"

class OutputHLSL: public Output
{
public:
    OutputHLSL(Program initProgram);
    virtual void output(const ASTContext& context, std::string& code);

private:
    struct Options
    {
        Options(uint32_t aIndentation): indentation(aIndentation) {}

        uint32_t indentation = 0;
    };
    void printDeclaration(const Declaration* declaration, Options options, std::string& code);
    void printStatement(const Statement* statement, Options options, std::string& code);
    void printExpression(const Expression* expression, Options options, std::string& code);
    void printConstruct(const Construct* construct, Options options, std::string& code);

    Program program;
};
