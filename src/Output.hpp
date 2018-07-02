//
//  OSL
//

#pragma once

#include <string>
#include "Parser.hpp"

class Output
{
public:
    virtual std::string output(const ASTContext& context) = 0;
};
