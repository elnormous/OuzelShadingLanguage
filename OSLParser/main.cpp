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

std::string tokenTypeToString(Token::Type type)
{
    switch (type)
    {
        case Token::Type::NONE: return "NONE";
        case Token::Type::INT_LITERAL: return "INT_LITERAL";
        case Token::Type::FLOAT_LITERAL: return "FLOAT_LITERAL";
        case Token::Type::STRING_LITERAL: return "STRING_LITERAL";
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
        LITERAL_EXPRESSION,
        DECLARATION_REFERENCE_EXPRESSION,
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

std::string nodeTypeToString(ASTNode::Type type)
{
    switch (type)
    {
        case ASTNode::Type::NONE: return "NONE";
        case ASTNode::Type::TYPE_DECLARATION: return "TYPE_DECLARATION";
        case ASTNode::Type::FIELD_DECLARATION: return "FIELD_DECLARATION";
        case ASTNode::Type::FUNCTION_DECLARATION: return "FUNCTION_DECLARATION";
        case ASTNode::Type::VARIABLE_DECLARATION: return "VARIABLE_DECLARATION";
        case ASTNode::Type::PARAMETER_DECLARATION: return "PARAMETER_DECLARATION";
        case ASTNode::Type::COMPOUND_STATEMENT: return "COMPOUND_STATEMENT";
        case ASTNode::Type::RETURN_STATEMENT: return "RETURN_STATEMENT";
        case ASTNode::Type::CALL_EXPRESSION: return "CALL_EXPRESSION";
        case ASTNode::Type::LITERAL_EXPRESSION: return "LITERAL_EXPRESSION";
        case ASTNode::Type::DECLARATION_REFERENCE_EXPRESSION: return "DECLARATION_REFERENCE_EXPRESSION";
        case ASTNode::Type::IF_STATEMENT: return "IF_STATEMENT";
        case ASTNode::Type::FOR_STATEMENT: return "FOR_STATEMENT";
        case ASTNode::Type::WHILE_STATEMENT: return "WHILE_STATEMENT";
        case ASTNode::Type::DO_STATEMENT: return "DO_STATEMENT";
        case ASTNode::Type::ASSIGN_OPERATOR: return "ASSIGN_OPERATOR";
        case ASTNode::Type::UNARY_OPERATOR: return "UNARY_OPERATOR";
        case ASTNode::Type::BINARY_OPERATOR: return "BINARY_OPERATOR";
        case ASTNode::Type::TERNARY_OPERATOR: return "TERNARY_OPERATOR";
        default: return "unknwon";
    }
}

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
    std::cout << indent << nodeTypeToString(node.type) << std::endl;

    for (const auto child : node.children)
    {
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
        std::cout << "Token, type: " << tokenTypeToString(token.type) << ", value: " << token.value << std::endl;
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
