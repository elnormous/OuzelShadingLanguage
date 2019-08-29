//
//  OSL
//

#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

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
        KEYWORD_ASM, // asm
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
        KEYWORD_CONST_CAST, // const_cast
        KEYWORD_CONTINUE, // continue
        KEYWORD_DEFAULT, // default
        KEYWORD_DELETE, // delete
        KEYWORD_DO, // do
        KEYWORD_DOUBLE, // double
        KEYWORD_DYNAMIC_CAST, // dynamic_cast
        KEYWORD_ELSE, // else
        KEYWORD_ENUM, // enum
        KEYWORD_EXPLICIT, // explicit
        KEYWORD_EXPORT, // export
        KEYWORD_EXTERN, // extern
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
        KEYWORD_REGISTER, // register
        KEYWORD_REINTERPRET_CAST, // reinterpret_cast
        KEYWORD_RETURN, // return
        KEYWORD_SHORT, // short
        KEYWORD_SIGNED, // signed
        KEYWORD_SIZEOF, // sizeof
        KEYWORD_STATIC, // static
        KEYWORD_STATIC_CAST, // static_cast
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
        KEYWORD_WCHAR_T, // wchar_t
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

inline std::string toString(Token::Kind kind)
{
    switch (kind)
    {
        case Token::Kind::NONE: return "NONE";
        case Token::Kind::LITERAL: return "LITERAL";
        case Token::Kind::KEYWORD: return "KEYWORD";
        case Token::Kind::PUNCTUATOR: return "PUNCTUATOR";
        case Token::Kind::OPERATOR: return "OPERATOR";
        case Token::Kind::IDENTIFIER: return "IDENTIFIER";
        default: return "Unknown";
    }
}

inline std::string toString(Token::Type type)
{
    switch (type)
    {
        case Token::Type::NONE: return "NONE";
        case Token::Type::LITERAL_INT: return "LITERAL_INT";
        case Token::Type::LITERAL_FLOAT: return "LITERAL_FLOAT";
        case Token::Type::LITERAL_DOUBLE: return "LITERAL_DOUBLE";
        case Token::Type::LITERAL_CHAR: return "LITERAL_CHAR";
        case Token::Type::LITERAL_STRING: return "LITERAL_STRING";
        case Token::Type::KEYWORD_AND: return "KEYWORD_AND";
        case Token::Type::KEYWORD_AND_EQ: return "KEYWORD_AND_EQ";
        case Token::Type::KEYWORD_ASM: return "KEYWORD_ASM";
        case Token::Type::KEYWORD_AUTO: return "KEYWORD_AUTO";
        case Token::Type::KEYWORD_BITAND: return "KEYWORD_BITAND";
        case Token::Type::KEYWORD_BITOR: return "KEYWORD_BITOR";
        case Token::Type::KEYWORD_BOOL: return "KEYWORD_BOOL";
        case Token::Type::KEYWORD_BREAK: return "KEYWORD_BREAK";
        case Token::Type::KEYWORD_CASE: return "KEYWORD_CASE";
        case Token::Type::KEYWORD_CATCH: return "KEYWORD_CATCH";
        case Token::Type::KEYWORD_CHAR: return "KEYWORD_CHAR";
        case Token::Type::KEYWORD_CLASS: return "KEYWORD_CLASS";
        case Token::Type::KEYWORD_COMPL: return "KEYWORD_COMPL";
        case Token::Type::KEYWORD_CONST: return "KEYWORD_CONST";
        case Token::Type::KEYWORD_CONST_CAST: return "KEYWORD_CONST_CAST";
        case Token::Type::KEYWORD_CONTINUE: return "KEYWORD_CONTINUE";
        case Token::Type::KEYWORD_DEFAULT: return "KEYWORD_DEFAULT";
        case Token::Type::KEYWORD_DELETE: return "KEYWORD_DELETE";
        case Token::Type::KEYWORD_DO: return "KEYWORD_DO";
        case Token::Type::KEYWORD_DOUBLE: return "KEYWORD_DOUBLE";
        case Token::Type::KEYWORD_DYNAMIC_CAST: return "KEYWORD_DYNAMIC_CAST";
        case Token::Type::KEYWORD_ELSE: return "KEYWORD_ELSE";
        case Token::Type::KEYWORD_ENUM: return "KEYWORD_ENUM";
        case Token::Type::KEYWORD_EXPLICIT: return "KEYWORD_EXPLICIT";
        case Token::Type::KEYWORD_EXPORT: return "KEYWORD_EXPORT";
        case Token::Type::KEYWORD_EXTERN: return "KEYWORD_EXTERN";
        case Token::Type::KEYWORD_FALSE: return "KEYWORD_FALSE";
        case Token::Type::KEYWORD_FLOAT: return "KEYWORD_FLOAT";
        case Token::Type::KEYWORD_FOR: return "KEYWORD_FOR";
        case Token::Type::KEYWORD_FRIEND: return "KEYWORD_FRIEND";
        case Token::Type::KEYWORD_GOTO: return "KEYWORD_GOTO";
        case Token::Type::KEYWORD_IF: return "KEYWORD_IF";
        case Token::Type::KEYWORD_INLINE: return "KEYWORD_INLINE";
        case Token::Type::KEYWORD_INT: return "KEYWORD_INT";
        case Token::Type::KEYWORD_LONG: return "KEYWORD_LONG";
        case Token::Type::KEYWORD_MUTABLE: return "KEYWORD_MUTABLE";
        case Token::Type::KEYWORD_NAMESPACE: return "KEYWORD_NAMESPACE";
        case Token::Type::KEYWORD_NEW: return "KEYWORD_NEW";
        case Token::Type::KEYWORD_NOEXCEPT: return "KEYWORD_NOEXCEPT";
        case Token::Type::KEYWORD_NOT: return "KEYWORD_NOT";
        case Token::Type::KEYWORD_NOT_EQ: return "KEYWORD_NOT_EQ";
        case Token::Type::KEYWORD_NULLPTR: return "KEYWORD_NULLPTR";
        case Token::Type::KEYWORD_OPERATOR: return "KEYWORD_OPERATOR";
        case Token::Type::KEYWORD_OR: return "KEYWORD_OR";
        case Token::Type::KEYWORD_OR_EQ: return "KEYWORD_OR_EQ";
        case Token::Type::KEYWORD_PRIVATE: return "KEYWORD_PRIVATE";
        case Token::Type::KEYWORD_PROTECTED: return "KEYWORD_PROTECTED";
        case Token::Type::KEYWORD_PUBLIC: return "KEYWORD_PUBLIC";
        case Token::Type::KEYWORD_REGISTER: return "KEYWORD_REGISTER";
        case Token::Type::KEYWORD_REINTERPRET_CAST: return "KEYWORD_REINTERPRET_CAST";
        case Token::Type::KEYWORD_RETURN: return "KEYWORD_RETURN";
        case Token::Type::KEYWORD_SHORT: return "KEYWORD_SHORT";
        case Token::Type::KEYWORD_SIGNED: return "KEYWORD_SIGNED";
        case Token::Type::KEYWORD_SIZEOF: return "KEYWORD_SIZEOF";
        case Token::Type::KEYWORD_STATIC: return "KEYWORD_STATIC";
        case Token::Type::KEYWORD_STATIC_CAST: return "KEYWORD_STATIC_CAST";
        case Token::Type::KEYWORD_STRUCT: return "KEYWORD_STRUCT";
        case Token::Type::KEYWORD_SWITCH: return "KEYWORD_SWITCH";
        case Token::Type::KEYWORD_TEMPLATE: return "KEYWORD_TEMPLATE";
        case Token::Type::KEYWORD_THIS: return "KEYWORD_THIS";
        case Token::Type::KEYWORD_THROW: return "KEYWORD_THROW";
        case Token::Type::KEYWORD_TRUE: return "KEYWORD_TRUE";
        case Token::Type::KEYWORD_TRY: return "KEYWORD_TRY";
        case Token::Type::KEYWORD_TYPEDEF: return "KEYWORD_TYPEDEF";
        case Token::Type::KEYWORD_TYPEID: return "KEYWORD_TYPEID";
        case Token::Type::KEYWORD_TYPENAME: return "KEYWORD_TYPENAME";
        case Token::Type::KEYWORD_UNION: return "KEYWORD_UNION";
        case Token::Type::KEYWORD_UNSIGNED: return "KEYWORD_UNSIGNED";
        case Token::Type::KEYWORD_USING: return "KEYWORD_USING";
        case Token::Type::KEYWORD_VIRTUAL: return "KEYWORD_VIRTUAL";
        case Token::Type::KEYWORD_VOID: return "KEYWORD_VOID";
        case Token::Type::KEYWORD_VOLATILE: return "KEYWORD_VOLATILE";
        case Token::Type::KEYWORD_WCHAR_T: return "KEYWORD_WCHAR_T";
        case Token::Type::KEYWORD_WHILE: return "KEYWORD_WHILE";
        case Token::Type::KEYWORD_XOR: return "KEYWORD_XOR";
        case Token::Type::KEYWORD_XOR_EQ: return "KEYWORD_XOR_EQ";
        case Token::Type::LEFT_PARENTHESIS: return "LEFT_PARENTHESIS";
        case Token::Type::RIGHT_PARENTHESIS: return "RIGHT_PARENTHESIS";
        case Token::Type::LEFT_BRACE: return "LEFT_BRACE";
        case Token::Type::RIGHT_BRACE: return "RIGHT_BRACE";
        case Token::Type::LEFT_BRACKET: return "LEFT_BRACKET";
        case Token::Type::RIGHT_BRACKET: return "RIGHT_BRACKET";
        case Token::Type::COMMA: return "COMMA";
        case Token::Type::SEMICOLON: return "SEMICOLON";
        case Token::Type::COLON: return "COLON";
        case Token::Type::IDENTIFIER: return "IDENTIFIER";
        case Token::Type::OPERATOR_PLUS: return "OPERATOR_PLUS";
        case Token::Type::OPERATOR_MINUS: return "OPERATOR_MINUS";
        case Token::Type::OPERATOR_MULTIPLY: return "OPERATOR_MULTIPLY";
        case Token::Type::OPERATOR_DIVIDE: return "OPERATOR_DIVIDE";
        case Token::Type::OPERATOR_MODULO: return "OPERATOR_MODULO";
        case Token::Type::OPERATOR_INCREMENT: return "OPERATOR_INCREMENT";
        case Token::Type::OPERATOR_DECREMENT: return "OPERATOR_DECREMENT";
        case Token::Type::OPERATOR_ASSIGNMENT: return "OPERATOR_ASSIGNMENT";
        case Token::Type::OPERATOR_PLUS_ASSIGNMENT: return "OPERATOR_PLUS_ASSIGNMENT";
        case Token::Type::OPERATOR_MINUS_ASSIGNMENT: return "OPERATOR_MINUS_ASSIGNMENT";
        case Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT: return "OPERATOR_MULTIPLY_ASSIGNMENT";
        case Token::Type::OPERATOR_DIVIDE_ASSIGNMENT: return "OPERATOR_DIVIDE_ASSIGNMENT";
        case Token::Type::OPERATOR_MODULO_ASSIGNMENT: return "OPERATOR_MODULO_ASSIGNMENT";
        case Token::Type::OPERATOR_BITWISE_AND_ASSIGNMENT: return "OPERATOR_BITWISE_AND_ASSIGNMENT";
        case Token::Type::OPERATOR_BITWISE_OR_ASSIGNMENT: return "OPERATOR_BITWISE_OR_ASSIGNMENT";
        case Token::Type::OPERATOR_BITWISE_NOT_ASSIGNMENT: return "OPERATOR_BITWISE_NOT_ASSIGNMENT";
        case Token::Type::OPERATOR_BITWISE_XOR_ASSIGNMENT: return "OPERATOR_BITWISE_XOR_ASSIGNMENT";
        case Token::Type::OPERATOR_SHIFT_RIGHT_ASSIGNMENT: return "OPERATOR_SHIFT_RIGHT_ASSIGNMENT";
        case Token::Type::OPERATOR_SHIFT_LEFT_ASSIGNMENT: return "OPERATOR_SHIFT_LEFT_ASSIGNMENT";
        case Token::Type::OPERATOR_BITWISE_AND: return "OPERATOR_BITWISE_AND";
        case Token::Type::OPERATOR_BITWISE_OR: return "OPERATOR_BITWISE_OR";
        case Token::Type::OPERATOR_BITWISE_NOT: return "OPERATOR_BITWISE_NOT";
        case Token::Type::OPERATOR_BITWISE_XOR: return "OPERATOR_BITWISE_XOR";
        case Token::Type::OPERATOR_SHIFT_RIGHT: return "OPERATOR_SHIFT_RIGHT";
        case Token::Type::OPERATOR_SHIFT_LEFT: return "OPERATOR_SHIFT_LEFT";
        case Token::Type::OPERATOR_EQUAL: return "OPERATOR_EQUAL";
        case Token::Type::OPERATOR_NOT_EQUAL: return "OPERATOR_NOT_EQUAL";
        case Token::Type::OPERATOR_LESS_THAN: return "OPERATOR_LESS_THAN";
        case Token::Type::OPERATOR_GREATER_THAN: return "OPERATOR_GREATER_THAN";
        case Token::Type::OPERATOR_LESS_THAN_EQUAL: return "OPERATOR_LESS_THAN_EQUAL";
        case Token::Type::OPERATOR_GREATER_THAN_EQUAL: return "OPERATOR_GREATER_THAN_EQUAL";
        case Token::Type::OPERATOR_AND: return "OPERATOR_AND";
        case Token::Type::OPERATOR_OR: return "OPERATOR_OR";
        case Token::Type::OPERATOR_NOT: return "OPERATOR_NOT";
        case Token::Type::OPERATOR_CONDITIONAL: return "OPERATOR_CONDITIONAL";
        case Token::Type::OPERATOR_DOT: return "OPERATOR_DOT";
        case Token::Type::OPERATOR_ARROW: return "OPERATOR_ARROW";
        case Token::Type::OPERATOR_ELLIPSIS: return "OPERATOR_ELLIPSIS";
        default: return "Unknown";
    }
}

std::vector<Token> tokenize(const std::vector<char>& code);
void dump(const std::vector<Token>& tokens);

#endif // TOKENIZER_HPP
