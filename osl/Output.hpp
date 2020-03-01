//
//  OSL
//

#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <string>
#include "Parser.hpp"

namespace ouzel
{
    enum class Program
    {
        Fragment,
        Vertex
    };

    class Output
    {
    public:
        Output(Program initProgram):
            program(initProgram)
        {
        }

        virtual ~Output() {}

        virtual std::string output(const ASTContext& context, bool whitespaces) = 0;

    private:
        Program program;
    };
}

#endif // OUTPUT_HPP
