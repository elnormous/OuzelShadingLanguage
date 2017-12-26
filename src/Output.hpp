//
//  OSL
//

#pragma once

#include <string>
#include "Parser.hpp"

class Output
{
public:
    virtual bool output(const ASTContext& context, const std::string& file) = 0;
};
