//
//  OSL
//

#ifndef OUTPUTHLSL_HPP
#define OUTPUTHLSL_HPP

#include "Output.hpp"
#include "Program.hpp"

namespace ouzel
{
    class OutputHLSL: public Output
    {
    public:
        OutputHLSL(Program initProgram);
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

#endif // OUTPUTHLSL_HPP
