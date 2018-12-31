//
//  OSL
//

#include "Preprocessor.hpp"

std::vector<char> Preprocessor::preprocess(const std::vector<char>& code)
{
    std::vector<char> result;

    for (auto i = code.begin(); i != code.end();)
    {
        if (*i == '\\' && (i + 1) != code.end() && *(i + 1) == '\n') // backslash followed by a newline
        {
            ++i; // skip the backslash
            ++i; // skip the newline
        }
        else
        {
            result.push_back(*i);
            ++i;
        }
    }

    // TODO: preprocess the code
    return result;
}
