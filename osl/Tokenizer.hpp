//
//  OSL
//

#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <string>
#include <vector>

namespace ouzel
{
    struct Token
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
            Friend, // friend
            Goto, // goto
            If, // if
            Inline, // inline
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
            Uniform, // uniform
            Union, // union
            Unsigned, // unsigned
            Using, // using
            Varying, // varying
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
        uint32_t line = 0;
        uint32_t column = 0;
    };

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
            case Token::Type::Friend: return "Friend";
            case Token::Type::Goto: return "Goto";
            case Token::Type::If: return "If";
            case Token::Type::Inline: return "Inline";
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
            case Token::Type::Uniform: return "Uniform";
            case Token::Type::Union: return "Union";
            case Token::Type::Unsigned: return "Unsigned";
            case Token::Type::Using: return "Using";
            case Token::Type::Varying: return "Varying";
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
            default: return "Unknown";
        }
    }

    std::vector<Token> tokenize(const std::string& code);
    void dump(const std::vector<Token>& tokens);
}

#endif // TOKENIZER_HPP
