//
//  OSL
//

#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include <string>

namespace ouzel
{
    class Preprocessor final
    {
    public:
        std::string preprocess(const std::string& code);
    };
}

#endif // PREPROCESSOR_HPP
