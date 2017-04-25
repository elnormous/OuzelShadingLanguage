//
//  main.cpp
//  OSLParser
//
//  Created by Elviss Strazdins on 24/04/2017.
//  Copyright © 2017 Elviss Strazdins. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

std::vector<std::string> builtinTypes = {
    "void", "bool", "int", "uint", "float", "double",
    "vec2", "vec3", "vec4", "mat3", "mat4"
};

struct Token
{
    enum class Type
    {
        NONE,
        INT_LITERAL, // int
        FLOAT_LITERAL, // float
        STRING_LITERAL, // string
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

bool tokenize(const std::vector<uint8_t>& buffer, std::vector<Token>& tokens)
{
    for (std::vector<uint8_t>::const_iterator i = buffer.begin(); i != buffer.end(); ++i)
    {
        char c = static_cast<char>(*i);

        Token token;

        if (c == '(' || c == ')' ||
            c == '{' || c == '}' ||
            c == '[' || c == ']' ||
            c == ',' || c == ';' || c == ':') // punctuation
        {
            if (c == '(') token.type = Token::Type::LEFT_PARENTHESIS;
            if (c == ')') token.type = Token::Type::RIGHT_PARENTHESIS;
            if (c == '{') token.type = Token::Type::LEFT_BRACE;
            if (c == '}') token.type = Token::Type::RIGHT_BRACE;
            if (c == '[') token.type = Token::Type::LEFT_BRACKET;
            if (c == ']') token.type = Token::Type::RIGHT_BRACKET;
            if (c == ',') token.type = Token::Type::COMMA;
            if (c == ';') token.type = Token::Type::SEMICOLON;
            if (c == ':') token.type = Token::Type::COLON;
            token.value.push_back(c);
        }
        else if (c >= '0' && c <= '9') // number
        {
            token.type = Token::Type::INT_LITERAL;

            bool dot = false;

            while ((c >= '0' && c <= '9') || c == '.')
            {
                if (c == '.')
                {
                    if (dot)
                    {
                        std::cerr << "Invalid number" << std::endl;
                        return false;
                    }
                    else
                    {
                        dot = true;
                        token.type = Token::Type::FLOAT_LITERAL;
                    }
                }

                token.value.push_back(c);

                if (++i == buffer.end()) break; // reached end of file
                c = static_cast<char>(*i);
            }
        }
        else if (c == '"') // string literal
        {
            token.type = Token::Type::STRING_LITERAL;

            if (++i == buffer.end()) break; // reached end of file
            c = static_cast<char>(*i);

            while (c != '"')
            {
                token.value.push_back(c);

                if (++i == buffer.end()) break; // reached end of file
                c = static_cast<char>(*i);
            }

            if (c != '"')
            {
                std::cerr << "Unterminated string" << std::endl;
                return false;
            }
        }
        else if ((c >= 'a' && c <= 'z') ||
                 (c >= 'A' && c <= 'Z') ||
                 c == '_')
        {
            while ((c >= 'a' && c <= 'z') ||
                   (c >= 'A' && c <= 'Z') ||
                   c == '_' ||
                   (c >= '0' && c <= '9'))
            {
                token.value.push_back(c);

                if (++i == buffer.end()) break; // reached end of file
                c = static_cast<char>(*i);
            }

            if (token.value == "if") token.type = Token::Type::KEYWORD_IF;
            else if (token.value == "else") token.type = Token::Type::KEYWORD_ELSE;
            else if (token.value == "return") token.type = Token::Type::KEYWORD_RETURN;
            else if (token.value == "for") token.type = Token::Type::KEYWORD_FOR;
            else if (token.value == "while") token.type = Token::Type::KEYWORD_WHILE;
            else if (token.value == "do") token.type = Token::Type::KEYWORD_DO;
            else if (token.value == "break") token.type = Token::Type::KEYWORD_BREAK;
            else if (token.value == "continue") token.type = Token::Type::KEYWORD_CONTINUE;
            else if (token.value == "true") token.type = Token::Type::KEYWORD_TRUE;
            else if (token.value == "false") token.type = Token::Type::KEYWORD_FALSE;
            else if (token.value == "inline") token.type = Token::Type::KEYWORD_INLINE;
            else if (token.value == "struct") token.type = Token::Type::KEYWORD_STRUCT;
            else if (token.value == "typedef") token.type = Token::Type::KEYWORD_TYPEDEF;
            else token.type = Token::Type::IDENTIFIER;
        }
        else if (c == '+' || c == '-' ||
                 c == '*' || c == '/' ||
                 c == '%' || c == '=' ||
                 c == '&' || c == '|' ||
                 c == '<' || c == '>' ||
                 c == '!' || c == '.' ||
                 c == '~' || c == '^')
        {
            if (c == '/' && (i + 1) != buffer.end() && // comment
                (static_cast<char>(*(i + 1)) == '/' || static_cast<char>(*(i + 1)) == '*'))
            {
                ++i;
                c = static_cast<char>(*i);

                if (c == '/') // single-line comment
                {
                    if (++i == buffer.end()) break; // reached end of file
                    c = static_cast<char>(*i);

                    while (c != '\n')
                    {
                        if (++i == buffer.end()) break; // reached end of file
                        c = static_cast<char>(*i);
                    }
                }
                else if (c == '*') // multi-line comment
                {
                    if (++i == buffer.end()) break; // reached end of file
                    c = static_cast<char>(*i);

                    while (c != '*' && (i + 1) != buffer.end() &&
                           static_cast<char>(*(i + 1)) != '/')
                    {
                        if (++i == buffer.end()) break; // reached end of file
                        c = static_cast<char>(*i);
                    }

                    if (c != '*' || (i + 1) == buffer.end() ||
                        static_cast<char>(*(i + 1)) != '/')
                    {
                        std::cerr << "Unterminated block comment" << std::endl;
                        return false;
                    }
                    else
                    {
                        ++i;
                    }
                }
                
                continue;
            }
            else if (c == '+')
            {
                token.type = Token::Type::OPERATOR_PLUS;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_PLUS_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '+')
                    {
                        token.type = Token::Type::OPERATOR_INCREMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '-')
            {
                token.type = Token::Type::OPERATOR_MINUS;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_MINUS_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '-')
                    {
                        token.type = Token::Type::OPERATOR_DECREMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '*')
            {
                token.type = Token::Type::OPERATOR_MULTIPLY;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '/')
            {
                token.type = Token::Type::OPERATOR_DIVIDE;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '/' || // single line comment
                             static_cast<char>(*(i + 1)) == '*') // multiline comment
                    {
                        ++i;
                        c = static_cast<char>(*i);

                        if (c == '/') // single-line comment
                        {
                            if (++i == buffer.end()) break; // reached end of file
                            c = static_cast<char>(*i);

                            while (c != '\n')
                            {
                                if (++i == buffer.end()) break; // reached end of file
                                c = static_cast<char>(*i);
                            }
                        }
                        else if (c == '*') // multi-line comment
                        {
                            if (++i == buffer.end()) break; // reached end of file
                            c = static_cast<char>(*i);

                            while (c != '*' && (i + 1) != buffer.end() &&
                                   static_cast<char>(*(i + 1)) != '/')
                            {
                                if (++i == buffer.end()) break; // reached end of file
                                c = static_cast<char>(*i);
                            }

                            if (c != '*' || (i + 1) == buffer.end() ||
                                static_cast<char>(*(i + 1)) != '/')
                            {
                                std::cerr << "Unterminated block comment" << std::endl;
                                return false;
                            }
                            else
                            {
                                ++i;
                            }
                        }
                        
                        continue;

                    }
                }
            }
            else if (c == '%')
            {
                token.type = Token::Type::OPERATOR_MODULO;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_MODULO_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '=')
            {
                token.type = Token::Type::OPERATOR_ASSIGNMENT;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_EQUAL;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '&')
            {
                token.type = Token::Type::OPERATOR_BITWISE_AND;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_AND_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '&')
                    {
                        token.type = Token::Type::OPERATOR_AND;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '|')
            {
                token.type = Token::Type::OPERATOR_BITWISE_OR;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_BITWISE_OR_ASSIGNMENT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '|')
                    {
                        token.type = Token::Type::OPERATOR_OR;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '<')
            {
                token.type = Token::Type::OPERATOR_LESS_THAN;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_LESS_THAN_EQUAL;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '<')
                    {
                        token.type = Token::Type::OPERATOR_SHIFT_LEFT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);

                        if ((i + 1) != buffer.end())
                        {
                            if (static_cast<char>(*(i + 1)) == '=')
                            {
                                token.type = Token::Type::OPERATOR_SHIFT_LEFT_ASSIGNMENT;
                                ++i;
                                c = static_cast<char>(*i);
                                token.value.push_back(c);
                            }
                        }
                    }
                }
            }
            else if (c == '>')
            {
                token.type = Token::Type::OPERATOR_GREATER_THAN;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_GREATER_THAN_EQUAL;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                    else if (static_cast<char>(*(i + 1)) == '>')
                    {
                        token.type = Token::Type::OPERATOR_SHIFT_RIGHT;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);

                        if ((i + 1) != buffer.end())
                        {
                            if (static_cast<char>(*(i + 1)) == '=')
                            {
                                token.type = Token::Type::OPERATOR_SHIFT_RIGHT_ASSIGNMENT;
                                ++i;
                                c = static_cast<char>(*i);
                                token.value.push_back(c);
                            }
                        }
                    }
                }
            }
            else if (c == '!')
            {
                token.type = Token::Type::OPERATOR_NOT;
                token.value.push_back(c);

                if ((i + 1) != buffer.end())
                {
                    if (static_cast<char>(*(i + 1)) == '=')
                    {
                        token.type = Token::Type::OPERATOR_NOT_EQUAL;
                        ++i;
                        c = static_cast<char>(*i);
                        token.value.push_back(c);
                    }
                }
            }
            else if (c == '?')
            {
                token.type = Token::Type::OPERATOR_CONDITIONAL;
                token.value.push_back(c);
            }
            else if (c == '.')
            {
                token.type = Token::Type::OPERATOR_DOT;
                token.value.push_back(c);
            }
        }
        else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') // whitespace
        {
            continue;
        }
        else
        {
            std::cerr << "Unknown character" << std::endl;
            return false;
        }

        tokens.push_back(token);
    }

    return true;
}

struct ASTNode
{
    enum class Type
    {
        NONE,
        TYPE_DECLARATION,
        FIELD_DECLARATION,
        FUNCTION_DECLARATION,
        VARIABLE_DECLARATION,
        PARAMETER_DECLARATION,
        COMPOUND_STATEMENT,
        RETURN_STATEMENT,
        CALL_EXPRESSION,
        IF_STATEMENT,
        FOR_STATEMENT,
        WHILE_STATEMENT,
        DO_STATEMENT,
        ASSIGN_OPERATOR,
        UNARY_OPERATOR,
        BINARY_OPERATOR,
        TERNARY_OPERATOR,
    };

    Type type = Type::NONE;
    std::vector<ASTNode> children;
};

struct ASTContext
{
    std::vector<ASTNode> nodes;
};

bool parse(const std::vector<Token>& tokens, ASTContext& context)
{
    for (std::vector<Token>::const_iterator i = tokens.begin(); i != tokens.end(); ++i)
    {
        // TODO: parse statement
        // file must have only declaration (type, variable, function) statements at the top level
    }

    return true;
}

void dumpNode(const ASTNode& node, std::string indent = std::string())
{
    for (const auto child : node.children)
    {
        std::cout << indent;
        switch (child.type)
        {
            case ASTNode::Type::NONE: std::cout << "NONE"; break;
            case ASTNode::Type::TYPE_DECLARATION: std::cout << "TYPE_DECLARATION"; break;
            case ASTNode::Type::FIELD_DECLARATION: std::cout << "FIELD_DECLARATION"; break;
            case ASTNode::Type::FUNCTION_DECLARATION: std::cout << "FUNCTION_DECLARATION"; break;
            case ASTNode::Type::VARIABLE_DECLARATION: std::cout << "VARIABLE_DECLARATION"; break;
            case ASTNode::Type::PARAMETER_DECLARATION: std::cout << "PARAMETER_DECLARATION"; break;
            case ASTNode::Type::COMPOUND_STATEMENT: std::cout << "COMPOUND_STATEMENT"; break;
            case ASTNode::Type::RETURN_STATEMENT: std::cout << "RETURN_STATEMENT"; break;
            case ASTNode::Type::CALL_EXPRESSION: std::cout << "CALL_EXPRESSION"; break;
            case ASTNode::Type::IF_STATEMENT: std::cout << "IF_STATEMENT"; break;
            case ASTNode::Type::FOR_STATEMENT: std::cout << "FOR_STATEMENT"; break;
            case ASTNode::Type::WHILE_STATEMENT: std::cout << "WHILE_STATEMENT"; break;
            case ASTNode::Type::DO_STATEMENT: std::cout << "DO_STATEMENT"; break;
            case ASTNode::Type::ASSIGN_OPERATOR: std::cout << "ASSIGN_OPERATOR"; break;
            case ASTNode::Type::UNARY_OPERATOR: std::cout << "UNARY_OPERATOR"; break;
            case ASTNode::Type::BINARY_OPERATOR: std::cout << "BINARY_OPERATOR"; break;
            case ASTNode::Type::TERNARY_OPERATOR: std::cout << "TERNARY_OPERATOR"; break;
        }

        std::cout << std::endl;

        dumpNode(child, indent + "  ");
    }
}

void dumpContext(ASTContext& context)
{
    for (const auto node : context.nodes)
    {
        dumpNode(node);
    }
}

int main(int argc, const char * argv[])
{
    if (argc < 2)
    {
        std::cerr << "Too few arguments" << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<uint8_t> buffer;

    FILE* file = fopen(argv[1], "rb");

    if (!file)
    {
        std::cerr << "Failed to open file" << std::endl;
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer.resize(size);
    fread(&buffer[0], size, 1, file);
    fclose(file);

    std::vector<Token> tokens;

    if (!tokenize(buffer, tokens))
    {
        std::cerr << "Failed to tokenize" << std::endl;
        return EXIT_FAILURE;
    }

    for (const Token& token : tokens)
    {
        std::cout << "Token, type: ";

        switch (token.type)
        {
            case Token::Type::NONE: std::cout << "NONE"; break;
            case Token::Type::INT_LITERAL: std::cout << "INT_LITERAL"; break;
            case Token::Type::FLOAT_LITERAL: std::cout << "FLOAT_LITERAL"; break;
            case Token::Type::STRING_LITERAL: std::cout << "STRING_LITERAL"; break;
            case Token::Type::KEYWORD_IF: std::cout << "KEYWORD_IF"; break;
            case Token::Type::KEYWORD_ELSE: std::cout << "KEYWORD_ELSE"; break;
            case Token::Type::KEYWORD_RETURN: std::cout << "KEYWORD_RETURN"; break;
            case Token::Type::KEYWORD_FOR: std::cout << "KEYWORD_FOR"; break;
            case Token::Type::KEYWORD_WHILE: std::cout << "KEYWORD_WHILE"; break;
            case Token::Type::KEYWORD_DO: std::cout << "KEYWORD_DO"; break;
            case Token::Type::KEYWORD_BREAK: std::cout << "KEYWORD_BREAK"; break;
            case Token::Type::KEYWORD_CONTINUE: std::cout << "KEYWORD_CONTINUE"; break;
            case Token::Type::KEYWORD_TRUE: std::cout << "KEYWORD_TRUE"; break;
            case Token::Type::KEYWORD_FALSE: std::cout << "KEYWORD_FALSE"; break;
            case Token::Type::KEYWORD_INLINE: std::cout << "KEYWORD_INLINE"; break;
            case Token::Type::KEYWORD_STRUCT: std::cout << "KEYWORD_STRUCT"; break;
            case Token::Type::KEYWORD_TYPEDEF: std::cout << "KEYWORD_TYPEDEF"; break;
            case Token::Type::LEFT_PARENTHESIS: std::cout << "LEFT_PARENTHESIS"; break;
            case Token::Type::RIGHT_PARENTHESIS: std::cout << "RIGHT_PARENTHESIS"; break;
            case Token::Type::LEFT_BRACE: std::cout << "LEFT_BRACE"; break;
            case Token::Type::RIGHT_BRACE: std::cout << "RIGHT_BRACE"; break;
            case Token::Type::LEFT_BRACKET: std::cout << "LEFT_BRACKET"; break;
            case Token::Type::RIGHT_BRACKET: std::cout << "RIGHT_BRACKET"; break;
            case Token::Type::COMMA: std::cout << "COMMA"; break;
            case Token::Type::SEMICOLON: std::cout << "SEMICOLON"; break;
            case Token::Type::COLON: std::cout << "COLON"; break;
            case Token::Type::IDENTIFIER: std::cout << "IDENTIFIER"; break;
            case Token::Type::OPERATOR_PLUS: std::cout << "OPERATOR_PLUS"; break;
            case Token::Type::OPERATOR_MINUS: std::cout << "OPERATOR_MINUS"; break;
            case Token::Type::OPERATOR_MULTIPLY: std::cout << "OPERATOR_MULTIPLY"; break;
            case Token::Type::OPERATOR_DIVIDE: std::cout << "OPERATOR_DIVIDE"; break;
            case Token::Type::OPERATOR_MODULO: std::cout << "OPERATOR_MODULO"; break;
            case Token::Type::OPERATOR_INCREMENT: std::cout << "OPERATOR_INCREMENT"; break;
            case Token::Type::OPERATOR_DECREMENT: std::cout << "OPERATOR_DECREMENT"; break;
            case Token::Type::OPERATOR_ASSIGNMENT: std::cout << "OPERATOR_ASSIGNMENT"; break;
            case Token::Type::OPERATOR_PLUS_ASSIGNMENT: std::cout << "OPERATOR_PLUS_ASSIGNMENT"; break;
            case Token::Type::OPERATOR_MINUS_ASSIGNMENT: std::cout << "OPERATOR_MINUS_ASSIGNMENT"; break;
            case Token::Type::OPERATOR_MULTIPLY_ASSIGNMENT: std::cout << "OPERATOR_MULTIPLY_ASSIGNMENT"; break;
            case Token::Type::OPERATOR_DIVIDE_ASSIGNMENT: std::cout << "OPERATOR_DIVIDE_ASSIGNMENT"; break;
            case Token::Type::OPERATOR_MODULO_ASSIGNMENT: std::cout << "OPERATOR_MODULO_ASSIGNMENT"; break;
            case Token::Type::OPERATOR_BITWISE_AND_ASSIGNMENT: std::cout << "OPERATOR_BITWISE_AND_ASSIGNMENT"; break;
            case Token::Type::OPERATOR_BITWISE_OR_ASSIGNMENT: std::cout << "OPERATOR_BITWISE_OR_ASSIGNMENT"; break;
            case Token::Type::OPERATOR_BITWISE_NOT_ASSIGNMENT: std::cout << "OPERATOR_BITWISE_NOT_ASSIGNMENT"; break;
            case Token::Type::OPERATOR_BITWISE_XOR_ASSIGNMENT: std::cout << "OPERATOR_BITWISE_XOR_ASSIGNMENT"; break;
            case Token::Type::OPERATOR_SHIFT_RIGHT_ASSIGNMENT: std::cout << "OPERATOR_SHIFT_RIGHT_ASSIGNMENT"; break;
            case Token::Type::OPERATOR_SHIFT_LEFT_ASSIGNMENT: std::cout << "OPERATOR_SHIFT_LEFT_ASSIGNMENT"; break;
            case Token::Type::OPERATOR_BITWISE_AND: std::cout << "OPERATOR_BITWISE_AND"; break;
            case Token::Type::OPERATOR_BITWISE_OR: std::cout << "OPERATOR_BITWISE_OR"; break;
            case Token::Type::OPERATOR_BITWISE_NOT: std::cout << "OPERATOR_BITWISE_NOT"; break;
            case Token::Type::OPERATOR_BITWISE_XOR: std::cout << "OPERATOR_BITWISE_XOR"; break;
            case Token::Type::OPERATOR_SHIFT_RIGHT: std::cout << "OPERATOR_SHIFT_RIGHT"; break;
            case Token::Type::OPERATOR_SHIFT_LEFT: std::cout << "OPERATOR_SHIFT_LEFT"; break;
            case Token::Type::OPERATOR_EQUAL: std::cout << "OPERATOR_EQUAL"; break;
            case Token::Type::OPERATOR_NOT_EQUAL: std::cout << "OPERATOR_NOT_EQUAL"; break;
            case Token::Type::OPERATOR_LESS_THAN: std::cout << "OPERATOR_LESS_THAN"; break;
            case Token::Type::OPERATOR_GREATER_THAN: std::cout << "OPERATOR_GREATER_THAN"; break;
            case Token::Type::OPERATOR_LESS_THAN_EQUAL: std::cout << "OPERATOR_LESS_THAN_EQUAL"; break;
            case Token::Type::OPERATOR_GREATER_THAN_EQUAL: std::cout << "OPERATOR_GREATER_THAN_EQUAL"; break;
            case Token::Type::OPERATOR_AND: std::cout << "OPERATOR_AND"; break;
            case Token::Type::OPERATOR_OR: std::cout << "OPERATOR_OR"; break;
            case Token::Type::OPERATOR_NOT: std::cout << "OPERATOR_NOT"; break;
            case Token::Type::OPERATOR_CONDITIONAL: std::cout << "OPERATOR_CONDITIONAL"; break;
            case Token::Type::OPERATOR_DOT: std::cout << "OPERATOR_DOT"; break;
        }

        std::cout << ", value: " << token.value << std::endl;
    }

    ASTContext context;

    if (!parse(tokens, context))
    {
        std::cerr << "Dailed to parse" << std::endl;
        return EXIT_FAILURE;
    }

    dumpContext(context);

    return EXIT_SUCCESS;
}
