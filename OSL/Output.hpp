//
//  Output.hpp
//  OSL
//
//  Created by Elviss Strazdins on 14/10/2017.
//  Copyright (c) 2017 Elviss Strazdins. All rights reserved.
//

#pragma once

#include <string>
#include "Parser.hpp"

class Output
{
public:
    virtual bool output(const ASTContext& context, const std::string& file) = 0;
};
