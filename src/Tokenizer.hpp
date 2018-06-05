//
//  OSL
//

#pragma once

#include <vector>
#include <string>

struct Token
{
    enum class Kind
    {
        NONE,
        LITERAL,
        KEYWORD,
        PUNCTUATOR,
        OPERATOR,
        IDENTIFIER
    };

    enum class Type
    {
        NONE,
        LITERAL_CHAR, // char
        LITERAL_INT, // int
        LITERAL_FLOAT, // float
        LITERAL_DOUBLE, // double
        LITERAL_STRING, // string
        KEYWORD_AND, // and
        KEYWORD_AND_EQ, // and_eq
        KEYWORD_AUTO, // auto
        KEYWORD_BITAND, // bitand
        KEYWORD_BITOR, // bitor
        KEYWORD_BOOL, // bool
        KEYWORD_BREAK, // break
        KEYWORD_CASE, // case
        KEYWORD_CATCH, // catch
        KEYWORD_CHAR, // char
        KEYWORD_CLASS, // class
        KEYWORD_COMPL, // compl
        KEYWORD_CONST, // const
        KEYWORD_CONTINUE, // continue
        KEYWORD_DEFAULT, // default
        KEYWORD_DELETE, // delete
        KEYWORD_DO, // do
        KEYWORD_DOUBLE, // double
        KEYWORD_ELSE, // else
        KEYWORD_ENUM, // enum
        KEYWORD_EXPLICIT, // extern
        KEYWORD_EXPORT, // explicit
        KEYWORD_EXTERN, // export
        KEYWORD_FALSE, // false
        KEYWORD_FLOAT, // float
        KEYWORD_FOR, // for
        KEYWORD_FRIEND, // friend
        KEYWORD_GOTO, // goto
        KEYWORD_IF, // if
        KEYWORD_INLINE, // inline
        KEYWORD_INT, // int
        KEYWORD_LONG, // long
        KEYWORD_MUTABLE, // mutable
        KEYWORD_NAMESPACE, // namespace
        KEYWORD_NEW, // new
        KEYWORD_NOEXCEPT, // noexcept
        KEYWORD_NOT, // not
        KEYWORD_NOT_EQ, // not_eq
        KEYWORD_NULLPTR, // nullptr
        KEYWORD_OPERATOR, // operator
        KEYWORD_OR, // or
        KEYWORD_OR_EQ, // or_eq
        KEYWORD_PRIVATE, // private
        KEYWORD_PROTECTED, // protected
        KEYWORD_PUBLIC, // public
        KEYWORD_RETURN, // return
        KEYWORD_SHORT, // short
        KEYWORD_SIGNED, // signed
        KEYWORD_SIZEOF, // sizeof
        KEYWORD_STATIC, // static
        KEYWORD_STRUCT, // struct
        KEYWORD_SWITCH, // switch
        KEYWORD_TEMPLATE, // template
        KEYWORD_THIS, // this
        KEYWORD_THROW, // throw
        KEYWORD_TRUE, // true
        KEYWORD_TRY, // try
        KEYWORD_TYPEDEF, // typedef
        KEYWORD_TYPEID, // typeid
        KEYWORD_TYPENAME, // typename
        KEYWORD_UNION, // union
        KEYWORD_UNSIGNED, // unsigned
        KEYWORD_USING, // using
        KEYWORD_VIRTUAL, // virtual
        KEYWORD_VOID, // void
        KEYWORD_VOLATILE, // volatile
        KEYWORD_WHILE, // while
        KEYWORD_XOR, // xor
        KEYWORD_XOR_EQ, // xor_eq
        LEFT_PARENTHESIS, // (
        RIGHT_PARENTHESIS, // )
        LEFT_BRACE, // {
        RIGHT_BRACE, // }
        LEFT_BRACKET, // [
        RIGHT_BRACKET, // ]
        COMMA, // ,
        SEMICOLON, // ;
        COLON, // :
        OPERATOR_PLUS, // +
        OPERATOR_MINUS, // -
        OPERATOR_MULTIPLY, // *
        OPERATOR_DIVIDE, // /
        OPERATOR_MODULO, // %
        OPERATOR_INCREMENT, // ++
        OPERATOR_DECREMENT, // --
        OPERATOR_ASSIGNMENT, // =
        OPERATOR_PLUS_ASSIGNMENT, // +=
        OPERATOR_MINUS_ASSIGNMENT, // -=
        OPERATOR_MULTIPLY_ASSIGNMENT, // *=
        OPERATOR_DIVIDE_ASSIGNMENT, // /=
        OPERATOR_MODULO_ASSIGNMENT, // %=
        OPERATOR_BITWISE_AND_ASSIGNMENT, // &= or and_eq
        OPERATOR_BITWISE_OR_ASSIGNMENT, // |= or or_eq
        OPERATOR_BITWISE_NOT_ASSIGNMENT, // ~=
        OPERATOR_BITWISE_XOR_ASSIGNMENT, // ^= or xor_eq
        OPERATOR_SHIFT_RIGHT_ASSIGNMENT, // >>=
        OPERATOR_SHIFT_LEFT_ASSIGNMENT, // <<=
        OPERATOR_BITWISE_AND, // & or bitand
        OPERATOR_BITWISE_OR, // | or bitor
        OPERATOR_BITWISE_NOT, // ~ or compl
        OPERATOR_BITWISE_XOR, // ^ or xor
        OPERATOR_SHIFT_RIGHT, // >>
        OPERATOR_SHIFT_LEFT, // <<
        OPERATOR_EQUAL, // ==
        OPERATOR_NOT_EQUAL, // != or not_eq
        OPERATOR_LESS_THAN, // <
        OPERATOR_GREATER_THAN, // >
        OPERATOR_LESS_THAN_EQUAL, // <=
        OPERATOR_GREATER_THAN_EQUAL, // >=
        OPERATOR_AND, // && or and
        OPERATOR_OR, // || or or
        OPERATOR_NOT, // ! or not
        OPERATOR_CONDITIONAL, // ?
        OPERATOR_DOT, // .
        OPERATOR_ARROW, // ->
        OPERATOR_ELLIPSIS, // ...
        IDENTIFIER
    };

    Kind kind = Kind::NONE;
    Type type = Type::NONE;
    std::string value;
    uint32_t line = 0;
    uint32_t column = 0;
};

std::vector<Token> tokenize(const std::vector<char>& code);
void dump(const std::vector<Token>& tokens);
