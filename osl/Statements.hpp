//
//  OSL
//

#ifndef STATEMENTS_HPP
#define STATEMENTS_HPP

#include "Construct.hpp"

namespace ouzel
{
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
            Return
        };

        Statement(Kind initStatementKind) noexcept: Construct(Construct::Kind::Statement), statementKind(initStatementKind) {}

        inline Kind getStatementKind() const noexcept { return statementKind; }

    protected:
        const Kind statementKind;
    };

    class ExpressionStatement: public Statement
    {
    public:
        ExpressionStatement() noexcept: Statement(Statement::Kind::Expression) {}

        const Expression* expression = nullptr;
    };

    class DeclarationStatement: public Statement
    {
    public:
        DeclarationStatement() noexcept: Statement(Statement::Kind::Declaration) {}

        const Declaration* declaration = nullptr;
    };

    class CompoundStatement: public Statement
    {
    public:
        CompoundStatement() noexcept: Statement(Statement::Kind::Compound) {}

        std::vector<const Statement*> statements;
    };

    class IfStatement: public Statement
    {
    public:
        IfStatement() noexcept: Statement(Statement::Kind::If) {}

        const Construct* condition = nullptr;
        const Statement* body = nullptr;
        const Statement* elseBody = nullptr;
    };

    class ForStatement: public Statement
    {
    public:
        ForStatement() noexcept: Statement(Statement::Kind::For) {}

        const Construct* initialization = nullptr;
        const Construct* condition = nullptr;
        const Expression* increment = nullptr;
        const Statement* body = nullptr;
    };

    class SwitchStatement: public Statement
    {
    public:
        SwitchStatement() noexcept: Statement(Statement::Kind::Switch) {}

        const Construct* condition = nullptr;
        const Statement* body = nullptr;
    };

    class CaseStatement: public Statement
    {
    public:
        CaseStatement() noexcept: Statement(Statement::Kind::Case) {}

        const Expression* condition = nullptr;
        const Statement* body = nullptr;
    };

    class DefaultStatement: public Statement
    {
    public:
        DefaultStatement() noexcept: Statement(Statement::Kind::Default) {}

        const Statement* body = nullptr;
    };

    class WhileStatement: public Statement
    {
    public:
        WhileStatement() noexcept: Statement(Statement::Kind::While) {}

        const Construct* condition = nullptr;
        const Statement* body = nullptr;
    };

    class DoStatement: public Statement
    {
    public:
        DoStatement() noexcept: Statement(Statement::Kind::Do) {}

        const Expression* condition = nullptr;
        const Statement* body = nullptr;
    };

    class BreakStatement: public Statement
    {
    public:
        BreakStatement() noexcept: Statement(Statement::Kind::Break) {}
    };

    class ContinueStatement: public Statement
    {
    public:
        ContinueStatement() noexcept: Statement(Statement::Kind::Continue) {}
    };

    class ReturnStatement: public Statement
    {
    public:
        ReturnStatement() noexcept: Statement(Statement::Kind::Return) {}

        const Expression* result = nullptr;
    };
}

#endif // STATEMENTS_HPP
