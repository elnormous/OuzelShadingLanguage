//
//  OutputHLSL.hpp
//  OSL
//
//  Created by Elviss Strazdins on 14/10/2017.
//  Copyright (c) 2017 Elviss Strazdins. All rights reserved.
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
