//
//  OSL
//

#pragma once

#include "Output.hpp"

class OutputMetal: public Output
{
public:
    virtual bool output(const ASTContext& context, const std::string& outputFile);
};
