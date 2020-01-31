//
// OSL
//

#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <string>

namespace ouzel
{
    enum class Program
    {
        Fragment,
        Vertex
    };

    inline std::string toString(Program program)
    {
        switch (program)
        {
            case Program::Fragment: return "Fragment";
            case Program::Vertex: return "Vertex";
            default: return "Unknown";
        }
    }
}

#endif // PROGRAM_HPP
