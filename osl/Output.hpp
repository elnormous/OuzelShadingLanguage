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
        explicit Output(Program initProgram):
            program(initProgram)
        {
        }

        virtual ~Output() = default;

        virtual std::string output(const Context& context, bool whitespaces) = 0;

    private:
        Program program;
    };
}

#endif // OUTPUT_HPP
