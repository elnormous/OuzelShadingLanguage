//
//  OSL
//

#pragma once

#include <string>
#include "Parser.hpp"

class Output
{
public:
    virtual void output(const ASTContext& context, std::string& code) = 0;
};
