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
        Empty,
        Expression,
        Declaration,
        Compound,
        If,
        For,
        Switch,
        Case,
        Default,
        While,
        Do,
        Break,
        Continue,
        Return,
    };

    Statement(Kind initStatementKind) noexcept: Construct(Construct::Kind::Statement), statementKind(initStatementKind) {}

    inline Kind getStatementKind() const noexcept { return statementKind; }

protected:
    Kind statementKind;
};

class ExpressionStatement: public Statement
{
public:
    ExpressionStatement(): Statement(Statement::Kind::Expression) {}

    Expression* expression = nullptr;
};

class DeclarationStatement: public Statement
{
public:
    DeclarationStatement(): Statement(Statement::Kind::Declaration) {}

    Declaration* declaration = nullptr;
};

class CompoundStatement: public Statement
{
public:
    CompoundStatement(): Statement(Statement::Kind::Compound) {}

    std::vector<Statement*> statements;
};

class IfStatement: public Statement
{
public:
    IfStatement(): Statement(Statement::Kind::If) {}

    Construct* condition = nullptr;
    Statement* body = nullptr;
    Statement* elseBody = nullptr;
};

class ForStatement: public Statement
{
public:
    ForStatement(): Statement(Statement::Kind::For) {}

    Construct* initialization = nullptr;
    Construct* condition = nullptr;
    Expression* increment = nullptr;
    Statement* body = nullptr;
};

class SwitchStatement: public Statement
{
public:
    SwitchStatement(): Statement(Statement::Kind::Switch) {}

    Construct* condition = nullptr;
    Statement* body = nullptr;
};

class CaseStatement: public Statement
{
public:
    CaseStatement(): Statement(Statement::Kind::Case) {}

    Expression* condition = nullptr;
    Statement* body = nullptr;
};

class DefaultStatement: public Statement
{
public:
    DefaultStatement(): Statement(Statement::Kind::Default) {}

    Statement* body = nullptr;
};

class WhileStatement: public Statement
{
public:
    WhileStatement(): Statement(Statement::Kind::While) {}

    Construct* condition = nullptr;
    Statement* body = nullptr;
};

class DoStatement: public Statement
{
public:
    DoStatement(): Statement(Statement::Kind::Do) {}

    Expression* condition = nullptr;
    Statement* body = nullptr;
};

class BreakStatement: public Statement
{
public:
    BreakStatement(): Statement(Statement::Kind::Break) {}
};

class ContinueStatement: public Statement
{
public:
    ContinueStatement(): Statement(Statement::Kind::Continue) {}
};

class ReturnStatement: public Statement
{
public:
    ReturnStatement(): Statement(Statement::Kind::Return) {}

    Expression* result = nullptr;
};

#endif // STATEMENTS_HPP
