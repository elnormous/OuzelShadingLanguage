//
//  OSL
//

#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <map>
#include <string>
#include <vector>

namespace ouzel
{
    struct Token final
    {
        enum class Type
        {
            None,
            CharLiteral, // char
            IntLiteral, // int
            FloatLiteral, // float
            DoubleLiteral, // double
            StringLiteral, // string
            And, // && or and
            Asm, // asm
            Auto, // auto
            Bool, // bool
            Break, // break
            Case, // case
            Catch, // catch
            Char, // char
            Class, // class
            Const, // const
            ConstCast, // const_cast
            Continue, // continue
            Default, // default
            Delete, // delete
            Do, // do
            Double, // double
            DynamicCast, // dynamic_cast
            Else, // else
            Enum, // enum
            Explicit, // explicit
            Export, // export
            Extern, // extern
            False, // false
            Float, // float
            For, // for
            Fragment, // fragment
            Friend, // friend
            Function, // function
            Goto, // goto
            If, // if
            In, // in
            Inline, // inline
            Inout, // inout
            Int, // int
            Long, // long
            Mutable, // mutable
            Namespace, // namespace
            New, // new
            Noexcept, // noexcept
            Not, // ! or not
            NotEq, // != or not_eq
            Nullptr, // nullptr
            Operator, // operator
            Or, // || or or
            Out, // out
            Private, // private
            Protected, // protected
            Public, // public
            Register, // register
            ReinterpretCast, // reinterpret_cast
            Return, // return
            Short, // short
            Signed, // signed
            Sizeof, // sizeof
            Static, // static
            StaticCast, // static_cast
            Struct, // struct
            Switch, // switch
            Template, // template
            This, // this
            Throw, // throw
            True, // true
            Try, // try
            Typedef, // typedef
            Typeid, // typeid
            Typename, // typename
            Union, // union
            Unsigned, // unsigned
            Using, // using
            Var, // var
            Varying, // varying
            Vertex, // vertex
            Virtual, // virtual
            Void, // void
            Volatile, // volatile
            WcharT, // wchar_t
            While, // while
            LeftParenthesis, // (
            RightParenthesis, // )
            LeftBrace, // {
            RightBrace, // }
            LeftBracket, // [
            RightBracket, // ]
            Comma, // ,
            Semicolon, // ;
            Colon, // :
            Plus, // +
            Minus, // -
            Multiply, // *
            Divide, // /
            Modulo, // %
            Increment, // ++
            Decrement, // --
            Assignment, // =
            PlusAssignment, // +=
            MinusAssignment, // -=
            MultiplyAssignment, // *=
            DivideAssignment, // /=
            ModuloAssignment, // %=
            BitwiseAndAssignment, // &= or and_eq
            BitwiseOrAssignment, // |= or or_eq
            BitwiseNotAssignment, // ~=
            BitwiseXorAssignment, // ^= or xor_eq
            ShiftRightAssignment, // >>=
            ShiftLeftAssignment, // <<=
            BitwiseAnd, // & or bitand
            BitwiseOr, // | or bitor
            BitwiseNot, // ~ or compl
            BitwiseXor, // ^ or xor
            ShiftRight, // >>
            ShiftLeft, // <<
            Equal, // ==
            LessThan, // <
            GreaterThan, // >
            LessThanEqual, // <=
            GreaterThanEqual, // >=
            Conditional, // ?
            Dot, // .
            Arrow, // ->
            Ellipsis, // ...
            Identifier
        };

        Type type = Type::None;
        std::string value;
        std::uint32_t line = 0;
        std::uint32_t column = 0;
    };

    inline std::vector<Token> tokenize(const std::string& code)
    {
        const std::map<std::string, Token::Type> keywordMap = {
            {"and", Token::Type::And},
            {"and_eq", Token::Type::BitwiseAndAssignment},
            {"asm", Token::Type::Asm},
            {"auto", Token::Type::Auto},
            {"bitand", Token::Type::BitwiseAnd},
            {"bitor", Token::Type::BitwiseOr},
            {"bool", Token::Type::Bool},
            {"break", Token::Type::Break},
            {"case", Token::Type::Case},
            {"catch", Token::Type::Catch},
            {"char", Token::Type::Char},
            {"class", Token::Type::Class},
            {"compl", Token::Type::BitwiseNot},
            {"const", Token::Type::Const},
            {"continue", Token::Type::Continue},
            {"default", Token::Type::Default},
            {"delete", Token::Type::Delete},
            {"do", Token::Type::Do},
            {"double", Token::Type::Double},
            {"dynamic_cast", Token::Type::DynamicCast},
            {"else", Token::Type::Else},
            {"enum", Token::Type::Enum},
            {"explicit", Token::Type::Explicit},
            {"export", Token::Type::Export},
            {"extern", Token::Type::Extern},
            {"false", Token::Type::False},
            {"float", Token::Type::Float},
            {"for", Token::Type::For},
            {"fragment", Token::Type::Fragment},
            {"friend", Token::Type::Friend},
            {"function", Token::Type::Function},
            {"goto", Token::Type::Goto},
            {"if", Token::Type::If},
            {"in", Token::Type::In},
            {"inline", Token::Type::Inline},
            {"inout", Token::Type::Inout},
            {"int", Token::Type::Int},
            {"long", Token::Type::Long},
            {"mutable", Token::Type::Mutable},
            {"namespace", Token::Type::Namespace},
            {"new", Token::Type::New},
            {"noexcept", Token::Type::Noexcept},
            {"not", Token::Type::Not},
            {"not_eq", Token::Type::NotEq},
            {"nullptr", Token::Type::Nullptr},
            {"operator", Token::Type::Operator},
            {"or", Token::Type::Or},
            {"or_eq", Token::Type::BitwiseOrAssignment},
            {"out", Token::Type::Out},
            {"private", Token::Type::Private},
            {"protected", Token::Type::Protected},
            {"public", Token::Type::Public},
            {"register", Token::Type::Register},
            {"reinterpret_cast", Token::Type::ReinterpretCast},
            {"return", Token::Type::Return},
            {"short", Token::Type::Short},
            {"signed", Token::Type::Signed},
            {"sizeof", Token::Type::Sizeof},
            {"static", Token::Type::Static},
            {"static_cast", Token::Type::StaticCast},
            {"struct", Token::Type::Struct},
            {"switch", Token::Type::Switch},
            {"template", Token::Type::Template},
            {"this", Token::Type::This},
            {"throw", Token::Type::Throw},
            {"true", Token::Type::True},
            {"try", Token::Type::Try},
            {"typedef", Token::Type::Typedef},
            {"typeid", Token::Type::Typeid},
            {"typename", Token::Type::Typename},
            {"union", Token::Type::Union},
            {"unsigned", Token::Type::Unsigned},
            {"using", Token::Type::Using},
            {"var", Token::Type::Var},
            {"varying", Token::Type::Varying},
            {"vertex", Token::Type::Vertex},
            {"virtual", Token::Type::Virtual},
            {"void", Token::Type::Void},
            {"volatile", Token::Type::Volatile},
            {"wchar_t", Token::Type::WcharT},
            {"while", Token::Type::While},
            {"xor", Token::Type::BitwiseXor},
            {"xor_eq", Token::Type::BitwiseXorAssignment}
        };

        std::vector<Token> tokens;
        std::uint32_t line = 1;
        auto lineStart = code.begin();

        for (auto i = code.begin(); i != code.end();)
        {
            Token token;
            token.line = line;
            token.column = static_cast<std::uint32_t>(i - lineStart) + 1;

            if (*i == '(' || *i == ')' ||
                *i == '{' || *i == '}' ||
                *i == '[' || *i == ']' ||
                *i == ',' || *i == ';' ||
                *i == ':') // punctuation
            {
                if (*i == '(') token.type = Token::Type::LeftParenthesis;
                if (*i == ')') token.type = Token::Type::RightParenthesis;
                if (*i == '{') token.type = Token::Type::LeftBrace;
                if (*i == '}') token.type = Token::Type::RightBrace;
                if (*i == '[') token.type = Token::Type::LeftBracket;
                if (*i == ']') token.type = Token::Type::RightBracket;
                if (*i == ',') token.type = Token::Type::Comma;
                if (*i == ';') token.type = Token::Type::Semicolon;
                if (*i == ':') token.type = Token::Type::Colon;
                token.value.push_back(*i);

                ++i;
            }
            else if ((*i >= '0' && *i <= '9') ||  // number
                     (*i == '.' && (i + 1) != code.end() && *(i + 1) >= '0' && *(i + 1) <= '9')) // starts with a dot
            {
                bool integer = true;

                while (i != code.end() && (*i >= '0' && *i <= '9'))
                {
                    token.value.push_back(*i);
                    ++i;
                }

                if (i != code.end() && *i == '.')
                {
                    integer = false;

                    token.value.push_back(*i);
                    ++i;

                    while (i != code.end() && (*i >= '0' && *i <= '9'))
                    {
                        token.value.push_back(*i);
                        ++i;
                    }
                }

                // parse exponent
                if (i != code.end() &&
                    (*i == 'e' || *i == 'E'))
                {
                    integer = false;

                    token.value.push_back(*i);
                    if (++i == code.end())
                        throw std::runtime_error("Invalid exponent");

                    if (*i == '+' || *i == '-')
                        token.value.push_back(*i++);

                    if (i == code.end() || *i < '0' || *i > '9')
                        throw std::runtime_error("Invalid exponent");

                    while (i != code.end() && *i >= '0' && *i <= '9')
                    {
                        token.value.push_back(*i);
                        ++i;
                    }
                }

                std::string suffix;

                while (i != code.end() && ((*i >= 'a' && *i <= 'z') || (*i >= 'A' && *i <= 'Z')))
                {
                    suffix.push_back(*i);
                    ++i;
                }

                if (suffix.empty())
                {
                    if (integer) token.type = Token::Type::IntLiteral;
                    else token.type = Token::Type::DoubleLiteral;
                }
                else if (suffix == "f" || suffix == "F")
                {
                    if (integer) throw std::runtime_error("Invalid integer constant");
                    else token.type = Token::Type::FloatLiteral;
                }
                else throw std::runtime_error("Invalid suffix " + suffix);
            }
            else if (*i == '"') // string literal
            {
                token.type = Token::Type::StringLiteral;

                for (;;)
                {
                    if (++i == code.end())
                        throw std::runtime_error("Unterminated string literal");

                    if (*i == '"')
                    {
                        ++i;
                        break;
                    }
                    else if (*i == '\\')
                    {
                        if (++i == code.end())
                            throw std::runtime_error("Unterminated string literal");

                        if (*i == 'a') token.value.push_back('\a');
                        else if (*i == 'b') token.value.push_back('\b');
                        else if (*i == 't') token.value.push_back('\t');
                        else if (*i == 'n') token.value.push_back('\n');
                        else if (*i == 'v') token.value.push_back('\v');
                        else if (*i == 'f') token.value.push_back('\f');
                        else if (*i == 'r') token.value.push_back('\r');
                        else if (*i == '"') token.value.push_back('"');
                        else if (*i == '\?') token.value.push_back('\?');
                        else if (*i == '\\') token.value.push_back('\\');
                        else
                            throw std::runtime_error("Unrecognized escape character");
                        // TODO: handle numeric character references
                    }
                    else if (*i == '\n')
                        throw std::runtime_error("Unterminated string literal");
                    else
                        token.value.push_back(*i);
                }
            }
            else if (*i == '\'') // char literal
            {
                token.type = Token::Type::CharLiteral;

                if (++i == code.end()) // reached end of file
                    throw std::runtime_error("Unterminated char literal");

                if (*i == '\\')
                {
                    if (++i == code.end())
                        throw std::runtime_error("Unterminated char literal");

                    if (*i == 'a') token.value.push_back('\a');
                    else if (*i == 'b') token.value.push_back('\b');
                    else if (*i == 't') token.value.push_back('\t');
                    else if (*i == 'n') token.value.push_back('\n');
                    else if (*i == 'v') token.value.push_back('\v');
                    else if (*i == 'f') token.value.push_back('\f');
                    else if (*i == 'r') token.value.push_back('\r');
                    else if (*i == '\'') token.value.push_back('\'');
                    else if (*i == '\?') token.value.push_back('\?');
                    else if (*i == '\\') token.value.push_back('\\');
                    else
                        throw std::runtime_error("Unrecognized escape character");
                    // TODO: handle numeric character references
                }
                else
                    token.value.push_back(*i);

                if (++i == code.end()) // reached end of file
                    throw std::runtime_error("Unterminated char literal");

                if (*i != '\'')
                    throw std::runtime_error("Invalid char literal");

                ++i;
            }
            else if ((*i >= 'a' && *i <= 'z') ||
                     (*i >= 'A' && *i <= 'Z') ||
                     *i == '_')
            {
                while (i != code.end() &&
                       ((*i >= 'a' && *i <= 'z') ||
                        (*i >= 'A' && *i <= 'Z') ||
                        *i == '_' ||
                        (*i >= '0' && *i <= '9')))
                {
                    token.value.push_back(*i);
                    ++i;
                }

                std::map<std::string, Token::Type>::const_iterator keywordIterator = keywordMap.find(token.value);

                if (keywordIterator != keywordMap.end())
                    token.type = keywordIterator->second;
                else
                    token.type = Token::Type::Identifier;
            }
            else if (*i == '+' || *i == '-' ||
                     *i == '*' || *i == '/' ||
                     *i == '%' || *i == '=' ||
                     *i == '&' || *i == '|' ||
                     *i == '<' || *i == '>' ||
                     *i == '!' || *i == '.' ||
                     *i == '~' || *i == '^' ||
                     *i == '?')
            {
                if (*i == '+')
                {
                    token.type = Token::Type::Plus;
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end())
                    {
                        if (*i == '=') // +=
                        {
                            token.type = Token::Type::PlusAssignment;
                            token.value.push_back(*i);
                            ++i;
                        }
                        else if (*i == '+') // ++
                        {
                            token.type = Token::Type::Increment;
                            token.value.push_back(*i);
                            ++i;
                        }
                    }
                }
                else if (*i == '-')
                {
                    token.type = Token::Type::Minus;
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end())
                    {
                        if (*i == '=') // -=
                        {
                            token.type = Token::Type::MinusAssignment;
                            token.value.push_back(*i);
                            ++i;
                        }
                        else if (*i == '-') // --
                        {
                            token.type = Token::Type::Decrement;
                            token.value.push_back(*i);
                            ++i;
                        }
                        else if (*i == '>') // ->
                        {
                            token.type = Token::Type::Arrow;
                            token.value.push_back(*i);
                            ++i;
                        }
                    }
                }
                else if (*i == '*')
                {
                    token.type = Token::Type::Multiply;
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end())
                    {
                        if (*i == '=') // *=
                        {
                            token.type = Token::Type::Multiply;
                            token.value.push_back(*i);
                            ++i;
                        }
                    }
                }
                else if (*i == '/')
                {
                    token.type = Token::Type::Divide;
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end())
                    {
                        if (*i == '=') // /=
                        {
                            token.type = Token::Type::Multiply;
                            token.value.push_back(*i);
                            ++i;
                        }
                    }
                }
                else if (*i == '%')
                {
                    token.type = Token::Type::Modulo;
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end())
                    {
                        if (*i == '=') // %=
                        {
                            token.type = Token::Type::ModuloAssignment;
                            token.value.push_back(*i);
                            ++i;
                        }
                    }
                }
                else if (*i == '=')
                {
                    token.type = Token::Type::Assignment;
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end())
                    {
                        if (*i == '=') // ==
                        {
                            token.type = Token::Type::Equal;
                            token.value.push_back(*i);
                            ++i;
                        }
                    }
                }
                else if (*i == '&')
                {
                    token.type = Token::Type::BitwiseAnd;
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end())
                    {
                        if (*i == '=') // &=
                        {
                            token.type = Token::Type::BitwiseAndAssignment;
                            token.value.push_back(*i);
                            ++i;
                        }
                        else if (*i == '&') // &&
                        {
                            token.type = Token::Type::And;
                            token.value.push_back(*i);
                            ++i;
                        }
                    }
                }
                else if (*i == '~')
                {
                    token.type = Token::Type::BitwiseNot;
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end())
                    {
                        if (*i == '=') // ~=
                        {
                            token.type = Token::Type::BitwiseNotAssignment;
                            token.value.push_back(*i);
                            ++i;
                        }
                    }
                }
                else if (*i == '^')
                {
                    token.type = Token::Type::BitwiseXor;
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end())
                    {
                        if (*i == '=') // ^=
                        {
                            token.type = Token::Type::BitwiseXorAssignment;
                            token.value.push_back(*i);
                            ++i;
                        }
                    }
                }
                else if (*i == '|')
                {
                    token.type = Token::Type::BitwiseOr;
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end())
                    {
                        if (*i == '=') // |=
                        {
                            token.type = Token::Type::BitwiseOrAssignment;
                            token.value.push_back(*i);
                            ++i;
                        }
                        else if (*i == '|') // ||
                        {
                            token.type = Token::Type::Or;
                            token.value.push_back(*i);
                            ++i;
                        }
                    }
                }
                else if (*i == '<')
                {
                    token.type = Token::Type::LessThan;
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end())
                    {
                        if (*i == '=') // <=
                        {
                            token.type = Token::Type::LessThanEqual;
                            token.value.push_back(*i);
                            ++i;
                        }
                        else if (*i == '<') // <<
                        {
                            token.type = Token::Type::ShiftLeft;
                            token.value.push_back(*i);
                            ++i;

                            if (i != code.end())
                            {
                                if (*i == '=') // <<=
                                {
                                    token.type = Token::Type::ShiftLeftAssignment;
                                    token.value.push_back(*i);
                                    ++i;
                                }
                            }
                        }
                    }
                }
                else if (*i == '>')
                {
                    token.type = Token::Type::GreaterThan;
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end())
                    {
                        if (*i == '=') // >=
                        {
                            token.type = Token::Type::GreaterThanEqual;
                            token.value.push_back(*i);
                            ++i;
                        }
                        else if (*i == '>') // >>
                        {
                            token.type = Token::Type::ShiftRight;
                            token.value.push_back(*i);
                            ++i;

                            if (i != code.end())
                            {
                                if (*i == '=') // >>=
                                {
                                    token.type = Token::Type::ShiftRightAssignment;
                                    token.value.push_back(*i);
                                    ++i;
                                }
                            }
                        }
                    }
                }
                else if (*i == '!')
                {
                    token.type = Token::Type::Not;
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end())
                    {
                        if (*i == '=') // !=
                        {
                            token.type = Token::Type::NotEq;
                            token.value.push_back(*i);
                            ++i;
                        }
                    }
                }
                else if (*i == '?')
                {
                    token.type = Token::Type::Conditional;
                    token.value.push_back(*i);
                    ++i;
                }
                else if (*i == '.')
                {
                    token.value.push_back(*i);
                    ++i;

                    if (i != code.end() && *i == '.' &&
                        (i + 1) != code.end() && *(i + 1) == '.')
                    {
                        token.type = Token::Type::Ellipsis;
                        token.value.push_back(*i);
                        ++i;
                        token.value.push_back(*i);
                        ++i;
                    }
                    else
                        token.type = Token::Type::Dot;
                }
            }
            else if (*i == '\n')
            {
                ++i;
                ++line;
                lineStart = i;
                continue;
            }
            else if (*i == ' ' || *i == '\t' || *i == '\r') // whitespace
            {
                ++i;
                continue;
            }
            else
                throw std::runtime_error("Unknown character");

            tokens.push_back(token);
        }

        return tokens;
    }
}

#endif // TOKENIZER_HPP
