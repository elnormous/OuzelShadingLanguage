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
    bool printNode(const Construct* node, const std::string& prefix, std::string& code);
};
