//
//  OSL
//

#pragma once

#include "Output.hpp"

class OutputHLSL: public Output
{
public:
    virtual bool output(const ASTContext& context, const std::string& outputFile);

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
};
