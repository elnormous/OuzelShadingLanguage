//
//  OSL
//

#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include <stdexcept>
#include <string>

namespace ouzel
{
    class Preprocessor final
    {
    public:
        std::string preprocess(const std::string& code)
        {
            std::string result;

            for (auto i = code.begin(); i != code.end();)
            {
                if (*i == '\\' && (i + 1) != code.end() && *(i + 1) == '\n') // backslash followed by a newline
                {
                    ++i; // skip the backslash
                    ++i; // skip the newline
                }
                else if (*i == '/' && (i + 1) != code.end() && *(i + 1) == '/') // single-line comment
                {
                    ++i; // skip the forward slash
                    ++i; // skip the forward slash
                    while (++i != code.end())
                        if (*i == '\n')
                        {
                            ++i; // skip the newline
                            break;
                        }
                }
                else if (*i == '/' && (i + 1) != code.end() && *(i + 1) == '*') // multi-line comment
                {
                    ++i; // skip the forward slash
                    ++i; // skip the star

                    bool terminated = false;
                    while (++i != code.end()) // TODO: insert #line directives
                        if (*i == '*' && ++i != code.end() && *i == '/')
                        {
                            terminated = true;
                            ++i; // skip the star
                            ++i; // skip the forward slash
                            break;
                        }

                    if (!terminated)
                        throw std::runtime_error("Unterminated block comment");
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
    };
}

#endif // PREPROCESSOR_HPP
