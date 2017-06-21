//
//  Tokenizer.h
//  OSLParser
//
//  Created by Elviss Strazdins on 25/04/2017.
//  Copyright (c) 2017 Elviss Strazdins. All rights reserved.
//

#pragma once

#include <vector>
#include <string>

struct Token
{
    enum class Type
    {
        NONE,
        LITERAL_INT, // int
        LITERAL_FLOAT, // float
        //LITERAL_CHAR, // char
        //LITERAL_STRING, // string
        KEYWORD_IF, // if
        KEYWORD_ELSE, // else
        KEYWORD_RETURN, // return
        KEYWORD_FOR, // for
        KEYWORD_WHILE, // while
        KEYWORD_DO, // do
        KEYWORD_BREAK, // break
        KEYWORD_CONTINUE, // continue
        KEYWORD_TRUE, // true
        KEYWORD_FALSE, // false
        KEYWORD_INLINE, // inline
        KEYWORD_STRUCT, // struct
        KEYWORD_TYPEDEF, // typedef
        KEYWORD_CONST, // const
        LEFT_PARENTHESIS, // )
        RIGHT_PARENTHESIS, // (
        LEFT_BRACE, // {
        RIGHT_BRACE, // }
        LEFT_BRACKET, // [
        RIGHT_BRACKET, // ]
        COMMA, // ,
        SEMICOLON, // ;
        COLON, // :
        IDENTIFIER,
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
        OPERATOR_BITWISE_AND_ASSIGNMENT, // &=
        OPERATOR_BITWISE_OR_ASSIGNMENT, // |=
        OPERATOR_BITWISE_NOT_ASSIGNMENT, // ~=
        OPERATOR_BITWISE_XOR_ASSIGNMENT, // ^=
        OPERATOR_SHIFT_RIGHT_ASSIGNMENT, // >>=
        OPERATOR_SHIFT_LEFT_ASSIGNMENT, // <<=
        OPERATOR_BITWISE_AND, // &
        OPERATOR_BITWISE_OR, // |
        OPERATOR_BITWISE_NOT, // ~
        OPERATOR_BITWISE_XOR, // ^
        OPERATOR_SHIFT_RIGHT, // >>
        OPERATOR_SHIFT_LEFT, // <<
        OPERATOR_EQUAL, // ==
        OPERATOR_NOT_EQUAL, // !=
        OPERATOR_LESS_THAN, // <
        OPERATOR_GREATER_THAN, // >
        OPERATOR_LESS_THAN_EQUAL, // <=
        OPERATOR_GREATER_THAN_EQUAL, // >=
        OPERATOR_AND, // &&
        OPERATOR_OR, // ||
        OPERATOR_NOT, // !
        OPERATOR_CONDITIONAL, // ?
        OPERATOR_DOT, // .
    };

    Type type = Type::NONE;
    std::string value;
};

inline std::string tokenTypeToString(Token::Type type)
{
    switch (type)
    {
        case Token::Type::NONE: return "NONE";
        case Token::Type::LITERAL_INT: return "LITERAL_INT";
        case Token::Type::LITERAL_FLOAT: return "LITERAL_FLOAT";
        //case Token::Type::LITERAL_CHAR: return "LITERAL_CHAR";
        //case Token::Type::LITERAL_STRING: return "LITERAL_STRING";
        case Token::Type::KEYWORD_IF: return "KEYWORD_IF";
        case Token::Type::KEYWORD_ELSE: return "KEYWORD_ELSE";
        case Token::Type::KEYWORD_RETURN: return "KEYWORD_RETURN";
        case Token::Type::KEYWORD_FOR: return "KEYWORD_FOR";
        case Token::Type::KEYWORD_WHILE: return "KEYWORD_WHILE";
        case Token::Type::KEYWORD_DO: return "KEYWORD_DO";
        case Token::Type::KEYWORD_BREAK: return "KEYWORD_BREAK";
        case Token::Type::KEYWORD_CONTINUE: return "KEYWORD_CONTINUE";
        case Token::Type::KEYWORD_TRUE: return "KEYWORD_TRUE";
        case Token::Type::KEYWORD_FALSE: return "KEYWORD_FALSE";
        case Token::Type::KEYWORD_INLINE: return "KEYWORD_INLINE";
        case Token::Type::KEYWORD_STRUCT: return "KEYWORD_STRUCT";
        case Token::Type::KEYWORD_TYPEDEF: return "KEYWORD_TYPEDEF";
        case Token::Type::KEYWORD_CONST: return "KEYWORD_CONST";
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
        default: return "unknown";
    }
}

bool tokenize(const std::vector<char>& code, std::vector<Token>& tokens);
