//
// OSL
//

#pragma once

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
    }

    return "unknown";
}
