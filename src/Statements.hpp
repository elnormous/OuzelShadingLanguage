//
//  OSL
//

#ifndef STATEMENTS_HPP
#define STATEMENTS_HPP

#include "Construct.hpp"

class Declaration;
class Expression;

class Statement: public Construct
{
public:
    enum class Kind
    {
        NONE,
        EMPTY,
        EXPRESSION,
        DECLARATION,
        COMPOUND,
        IF,
        FOR,
        SWITCH,
        CASE,
        DEFAULT,
        WHILE,
        DO,
        BREAK,
        CONTINUE,
        RETURN,
    };

    Statement(Kind initStatementKind): Construct(Construct::Kind::STATEMENT), statementKind(initStatementKind) {}

    inline Kind getStatementKind() const { return statementKind; }

protected:
    Kind statementKind = Kind::NONE;
};

class ExpressionStatement: public Statement
{
public:
    ExpressionStatement(): Statement(Statement::Kind::EXPRESSION) {}

    Expression* expression = nullptr;
};

class DeclarationStatement: public Statement
{
public:
    DeclarationStatement(): Statement(Statement::Kind::DECLARATION) {}

    Declaration* declaration = nullptr;
};

class CompoundStatement: public Statement
{
public:
    CompoundStatement(): Statement(Statement::Kind::COMPOUND) {}

    std::vector<Statement*> statements;
};

class IfStatement: public Statement
{
public:
    IfStatement(): Statement(Statement::Kind::IF) {}

    Construct* condition = nullptr;
    Statement* body = nullptr;
    Statement* elseBody = nullptr;
};

class ForStatement: public Statement
{
public:
    ForStatement(): Statement(Statement::Kind::FOR) {}

    Construct* initialization = nullptr;
    Construct* condition = nullptr;
    Expression* increment = nullptr;
    Statement* body = nullptr;
};

class SwitchStatement: public Statement
{
public:
    SwitchStatement(): Statement(Statement::Kind::SWITCH) {}

    Construct* condition = nullptr;
    Statement* body = nullptr;
};

class CaseStatement: public Statement
{
public:
    CaseStatement(): Statement(Statement::Kind::CASE) {}

    Expression* condition = nullptr;
    Statement* body = nullptr;
};

class DefaultStatement: public Statement
{
public:
    DefaultStatement(): Statement(Statement::Kind::DEFAULT) {}

    Statement* body = nullptr;
};

class WhileStatement: public Statement
{
public:
    WhileStatement(): Statement(Statement::Kind::WHILE) {}

    Construct* condition = nullptr;
    Statement* body = nullptr;
};

class DoStatement: public Statement
{
public:
    DoStatement(): Statement(Statement::Kind::DO) {}

    Expression* condition = nullptr;
    Statement* body = nullptr;
};

class BreakStatement: public Statement
{
public:
    BreakStatement(): Statement(Statement::Kind::BREAK) {}
};

class ContinueStatement: public Statement
{
public:
    ContinueStatement(): Statement(Statement::Kind::CONTINUE) {}
};

class ReturnStatement: public Statement
{
public:
    ReturnStatement(): Statement(Statement::Kind::RETURN) {}

    Expression* result = nullptr;
};

#endif // STATEMENTS_HPP
