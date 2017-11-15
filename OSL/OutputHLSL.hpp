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
    bool printNode(const Construct* node, Options options, std::string& code);
};
