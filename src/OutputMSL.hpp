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
    virtual bool output(const ASTContext& context, std::string& code);

private:
    struct Options
    {
        Options(uint32_t aIndentation): indentation(aIndentation) {}

        uint32_t indentation = 0;
    };
    bool printDeclaration(const Declaration* declaration, Options options, std::string& code);
    bool printStatement(const Statement* statement, Options options, std::string& code);
    bool printExpression(const Expression* expression, Options options, std::string& code);
    bool printConstruct(const Construct* construct, Options options, std::string& code);

    Program program;
    const std::map<Semantic, uint32_t> semantics;
};
