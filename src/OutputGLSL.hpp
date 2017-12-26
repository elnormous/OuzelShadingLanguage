//
//  OSL
//

#pragma once

#include "Output.hpp"

class OutputGLSL: public Output
{
public:
    virtual bool output(const ASTContext& context, const std::string& outputFile);
};
