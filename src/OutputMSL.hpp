//
//  OSL
//

#pragma once

#include <map>
#include "Output.hpp"
#include "Program.hpp"

class OutputMSL: public Output
{
public:
    OutputMSL(Program initProgram,
              const std::map<Semantic, uint32_t>& initSemantics);
    virtual std::string output(const ASTContext& context);

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
    const std::map<Semantic, uint32_t> semantics;
};
