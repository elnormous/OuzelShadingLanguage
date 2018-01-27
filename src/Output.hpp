//
//  OSL
//

#pragma once

#include <string>
#include "Parser.hpp"

class Output
{
public:
    virtual bool output(const ASTContext& context, std::string& code) = 0;
};
