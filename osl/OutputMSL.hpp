//
//  OSL
//

#ifndef OUTPUTMSL_HPP
#define OUTPUTMSL_HPP

#include <map>
#include "Output.hpp"
#include "Attributes.hpp"

namespace ouzel
{
    class OutputMSL: public Output
    {
    public:
        OutputMSL(Program initProgram);
        virtual std::string output(const ASTContext& context, bool whitespaces);

    private:
        struct Options
        {
            Options(uint32_t initIndentation, bool initWhitespaces):
                indentation(initIndentation), whitespaces(initWhitespaces) {}

            uint32_t indentation = 0;
            bool whitespaces = false;
        };
        void printDeclaration(const Declaration* declaration, Options options, std::string& code);
        void printStatement(const Statement* statement, Options options, std::string& code);
        void printExpression(const Expression* expression, Options options, std::string& code);
        void printConstruct(const Construct* construct, Options options, std::string& code);

        Program program;
    };
}

#endif // OUTPUTMSL_HPP
