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

    inline std::string toString(Token::Type type)
    {
        switch (type)
        {
            case Token::Type::None: return "None";
            case Token::Type::IntLiteral: return "IntLiteral";
            case Token::Type::FloatLiteral: return "FloatLiteral";
            case Token::Type::DoubleLiteral: return "DoubleLiteral";
            case Token::Type::CharLiteral: return "CharLiteral";
            case Token::Type::StringLiteral: return "StringLiteral";
            case Token::Type::And: return "And";
            case Token::Type::Asm: return "Asm";
            case Token::Type::Auto: return "Auto";
            case Token::Type::Bool: return "Bool";
            case Token::Type::Break: return "Break";
            case Token::Type::Case: return "Case";
            case Token::Type::Catch: return "Catch";
            case Token::Type::Char: return "Char";
            case Token::Type::Class: return "Class";
            case Token::Type::Const: return "Const";
            case Token::Type::ConstCast: return "ConstCast";
            case Token::Type::Continue: return "Continue";
            case Token::Type::Default: return "Default";
            case Token::Type::Delete: return "Delete";
            case Token::Type::Do: return "Do";
            case Token::Type::Double: return "Double";
            case Token::Type::DynamicCast: return "DynamicCast";
            case Token::Type::Else: return "Else";
            case Token::Type::Enum: return "Enum";
            case Token::Type::Explicit: return "Explicit";
            case Token::Type::Export: return "Export";
            case Token::Type::Extern: return "Extern";
            case Token::Type::False: return "False";
            case Token::Type::Float: return "Float";
            case Token::Type::For: return "For";
            case Token::Type::Fragment: return "Fragment";
            case Token::Type::Friend: return "Friend";
            case Token::Type::Function: return "Function";
            case Token::Type::Goto: return "Goto";
            case Token::Type::If: return "If";
            case Token::Type::In: return "In";
            case Token::Type::Inline: return "Inline";
            case Token::Type::Inout: return "Inout";
            case Token::Type::Int: return "Int";
            case Token::Type::Long: return "Long";
            case Token::Type::Mutable: return "Mutable";
            case Token::Type::Namespace: return "Namespace";
            case Token::Type::New: return "New";
            case Token::Type::Noexcept: return "Noexcept";
            case Token::Type::Not: return "Not";
            case Token::Type::NotEq: return "NotEq";
            case Token::Type::Nullptr: return "Nullptr";
            case Token::Type::Operator: return "Operator";
            case Token::Type::Or: return "Or";
            case Token::Type::Out: return "Out";
            case Token::Type::Private: return "Private";
            case Token::Type::Protected: return "Protected";
            case Token::Type::Public: return "Public";
            case Token::Type::Register: return "Register";
            case Token::Type::ReinterpretCast: return "ReinterpretCast";
            case Token::Type::Return: return "Return";
            case Token::Type::Short: return "Short";
            case Token::Type::Signed: return "Signed";
            case Token::Type::Sizeof: return "Sizeof";
            case Token::Type::Static: return "Static";
            case Token::Type::StaticCast: return "StaticCast";
            case Token::Type::Struct: return "Struct";
            case Token::Type::Switch: return "Switch";
            case Token::Type::Template: return "Template";
            case Token::Type::This: return "This";
            case Token::Type::Throw: return "Throw";
            case Token::Type::True: return "True";
            case Token::Type::Try: return "Try";
            case Token::Type::Typedef: return "Typedef";
            case Token::Type::Typeid: return "Typeid";
            case Token::Type::Typename: return "Typename";
            case Token::Type::Union: return "Union";
            case Token::Type::Unsigned: return "Unsigned";
            case Token::Type::Using: return "Using";
            case Token::Type::Var: return "Var";
            case Token::Type::Varying: return "Varying";
            case Token::Type::Vertex: return "Vertex";
            case Token::Type::Virtual: return "Virtual";
            case Token::Type::Void: return "Void";
            case Token::Type::Volatile: return "Volatile";
            case Token::Type::WcharT: return "WcharT";
            case Token::Type::While: return "While";
            case Token::Type::LeftParenthesis: return "LeftParenthesis";
            case Token::Type::RightParenthesis: return "RightParenthesis";
            case Token::Type::LeftBrace: return "LeftBrace";
            case Token::Type::RightBrace: return "RightBrace";
            case Token::Type::LeftBracket: return "LeftBracket";
            case Token::Type::RightBracket: return "RightBracket";
            case Token::Type::Comma: return "Comma";
            case Token::Type::Semicolon: return "Semicolon";
            case Token::Type::Colon: return "Colon";
            case Token::Type::Plus: return "Plus";
            case Token::Type::Minus: return "Minus";
            case Token::Type::Multiply: return "Multiply";
            case Token::Type::Divide: return "Divide";
            case Token::Type::Modulo: return "Modulo";
            case Token::Type::Increment: return "Increment";
            case Token::Type::Decrement: return "Decrement";
            case Token::Type::Assignment: return "Assignment";
            case Token::Type::PlusAssignment: return "PlusAssignment";
            case Token::Type::MinusAssignment: return "MinusAssignment";
            case Token::Type::MultiplyAssignment: return "MultiplyAssignment";
            case Token::Type::DivideAssignment: return "DivideAssignment";
            case Token::Type::ModuloAssignment: return "ModuloAssignment";
            case Token::Type::BitwiseAndAssignment: return "BitwiseAndAssignment";
            case Token::Type::BitwiseOrAssignment: return "BitwiseOrAssignment";
            case Token::Type::BitwiseNotAssignment: return "BitwiseNotAssignment";
            case Token::Type::BitwiseXorAssignment: return "BitwiseXorAssignment";
            case Token::Type::ShiftRightAssignment: return "ShiftRightAssignment";
            case Token::Type::ShiftLeftAssignment: return "ShiftLeftAssignment";
            case Token::Type::BitwiseAnd: return "BitwiseAnd";
            case Token::Type::BitwiseOr: return "BitwiseOr";
            case Token::Type::BitwiseNot: return "BitwiseNot";
            case Token::Type::BitwiseXor: return "BitwiseXor";
            case Token::Type::ShiftRight: return "ShiftRight";
            case Token::Type::ShiftLeft: return "ShiftLeft";
            case Token::Type::Equal: return "Equal";
            case Token::Type::LessThan: return "LessThan";
            case Token::Type::GreaterThan: return "GreaterThan";
            case Token::Type::LessThanEqual: return "LessThanEqual";
            case Token::Type::GreaterThanEqual: return "GreaterThanEqual";
            case Token::Type::Conditional: return "Conditional";
            case Token::Type::Dot: return "Dot";
            case Token::Type::Arrow: return "Arrow";
            case Token::Type::Ellipsis: return "Ellipsis";
            case Token::Type::Identifier: return "Identifier";
        }

        throw std::runtime_error("Unknown token type");
    }
}

#endif // TOKENIZER_HPP
