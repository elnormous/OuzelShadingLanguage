//
//  OSL
//

#ifndef STATEMENTS_HPP
#define STATEMENTS_HPP

#include <type_traits>
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
            Return,
            Value
        };

        Statement(Kind initStatementKind) noexcept: Construct(Construct::Kind::Statement), statementKind(initStatementKind) {}

        inline Kind getStatementKind() const noexcept { return statementKind; }

    private:
        const Kind statementKind;
    };

    class ExpressionStatement final: public Statement
    {
    public:
        ExpressionStatement(const Expression* initExpression) noexcept:
            Statement(Statement::Kind::Expression), expression(initExpression) {}

        const Expression* expression = nullptr;
    };

    class DeclarationStatement final: public Statement
    {
    public:
        DeclarationStatement(const Declaration* initDeclaration) noexcept:
            Statement(Statement::Kind::Declaration), declaration(initDeclaration) {}

        const Declaration* declaration = nullptr;
    };

    class CompoundStatement final: public Statement
    {
    public:
        CompoundStatement() noexcept: Statement(Statement::Kind::Compound) {}

        std::vector<const Statement*> statements;
    };

    class IfStatement final: public Statement
    {
    public:
        IfStatement() noexcept: Statement(Statement::Kind::If) {}

        const Statement* condition;
        const Statement* body = nullptr;
        const Statement* elseBody = nullptr;
    };

    class ForStatement final: public Statement
    {
    public:
        ForStatement() noexcept: Statement(Statement::Kind::For) {}

        const Statement* initialization;
        const Statement* condition;
        const Expression* increment = nullptr;
        const Statement* body = nullptr;
    };

    class SwitchStatement final: public Statement
    {
    public:
        SwitchStatement() noexcept: Statement(Statement::Kind::Switch) {}

        const Statement* condition;
        const Statement* body = nullptr;
    };

    class CaseStatement final: public Statement
    {
    public:
        CaseStatement() noexcept: Statement(Statement::Kind::Case) {}

        const Expression* condition = nullptr;
        const Statement* body = nullptr;
    };

    class DefaultStatement final: public Statement
    {
    public:
        DefaultStatement(const Statement* initBody) noexcept:
            Statement(Statement::Kind::Default),
            body(initBody) {}

        const Statement* body = nullptr;
    };

    class WhileStatement final: public Statement
    {
    public:
        WhileStatement() noexcept: Statement(Statement::Kind::While) {}

        const Statement* condition;
        const Statement* body = nullptr;
    };

    class DoStatement final: public Statement
    {
    public:
        DoStatement(const Expression* initCondition,
                    const Statement* initBody) noexcept:
            Statement(Statement::Kind::Do),
            condition(initCondition),
            body(initBody) {}

        const Expression* condition = nullptr;
        const Statement* body = nullptr;
    };

    class BreakStatement final: public Statement
    {
    public:
        BreakStatement() noexcept: Statement(Statement::Kind::Break) {}
    };

    class ContinueStatement final: public Statement
    {
    public:
        ContinueStatement() noexcept: Statement(Statement::Kind::Continue) {}
    };

    class ReturnStatement final: public Statement
    {
    public:
        ReturnStatement(const Expression* initResult) noexcept:
            Statement(Statement::Kind::Return),
            result(initResult) {}

        const Expression* result = nullptr;
    };

    class ValueStatement final: public Statement
    {
    public:
        ValueStatement(const Expression* initExpression) noexcept:
            Statement(Statement::Kind::Value), expression(initExpression) {}

        const Expression* expression = nullptr;
    };
}

#endif // STATEMENTS_HPP
