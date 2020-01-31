//
//  OSL
//

#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <string>
#include "Parser.hpp"

namespace ouzel
{
    class Output
    {
    public:
        virtual ~Output() {}
        virtual std::string output(const ASTContext& context, bool whitespaces) = 0;
    };
}

#endif // OUTPUT_HPP
