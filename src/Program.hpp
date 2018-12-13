//
// OSL
//

#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <string>

enum class Program
{
    NONE,
    FRAGMENT,
    VERTEX
};

inline std::string toString(Program program)
{
    switch (program)
    {
        case Program::NONE: return "NONE";
        case Program::FRAGMENT: return "FRAGMENT";
        case Program::VERTEX: return "VERTEX";
        default: return "unknown";
    }
}

#endif // PROGRAM_HPP
