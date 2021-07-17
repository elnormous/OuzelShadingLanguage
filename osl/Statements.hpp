//
//  OSL
//

#ifndef STATEMENTS_HPP
#define STATEMENTS_HPP

#include <type_traits>
#include "Construct.hpp"

namespace ouzel
{
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

        Statement(const Statement&) = delete;
        explicit Statement(Kind initStatementKind) noexcept:
            Construct{Construct::Kind::Statement},
            statementKind{initStatementKind} {}

        Statement& operator=(const Statement&) = delete;

        const Kind statementKind;
    };

    using StatementRef = std::reference_wrapper<const Statement>;

    class Expression;

    class ExpressionStatement final: public Statement
    {
    public:
        explicit ExpressionStatement(const Expression& initExpression) noexcept:
            Statement{Statement::Kind::Expression}, expression{initExpression} {}

        const Expression& expression;
    };

    class Declaration;

    class DeclarationStatement final: public Statement
    {
    public:
        explicit DeclarationStatement(const Declaration& initDeclaration) noexcept:
            Statement{Statement::Kind::Declaration}, declaration{initDeclaration} {}

        const Declaration& declaration;
    };

    class CompoundStatement final: public Statement
    {
    public:
        explicit CompoundStatement(std::vector<StatementRef> initStatements):
            Statement{Statement::Kind::Compound},
            statements{std::move(initStatements)} {}

        const std::vector<StatementRef> statements;
    };

    class IfStatement final: public Statement
    {
    public:
        IfStatement(const Construct& initCondition,
                    const Statement& initBody,
                    const Statement* initElseBody) noexcept:
            Statement{Statement::Kind::If},
            condition{initCondition},
            body{initBody},
            elseBody{initElseBody} {}

        const Construct& condition;
        const Statement& body;
        const Statement* elseBody = nullptr;
    };

    class ForStatement final: public Statement
    {
    public:
        ForStatement(const Construct* initInitialization,
                     const Construct* initCondition,
                     const Expression* initIncrement,
                     const Statement& initBody) noexcept:
            Statement{Statement::Kind::For},
            initialization{initInitialization},
            condition{initCondition},
            increment{initIncrement},
            body{initBody} {}

        const Construct* initialization = nullptr;
        const Construct* condition = nullptr;
        const Expression* increment = nullptr;
        const Statement& body;
    };

    class SwitchStatement final: public Statement
    {
    public:
        SwitchStatement(const Construct& initCondition,
                        const Statement& initBody) noexcept:
            Statement{Statement::Kind::Switch},
            condition{initCondition},
            body{initBody} {}

        const Construct& condition;
        const Statement& body;
    };

    class CaseStatement final: public Statement
    {
    public:
        CaseStatement(const Expression& initCondition,
                      const Statement& initBody) noexcept:
            Statement{Statement::Kind::Case},
            condition{initCondition},
            body{initBody} {}

        const Expression& condition;
        const Statement& body;
    };

    class DefaultStatement final: public Statement
    {
    public:
        explicit DefaultStatement(const Statement& initBody) noexcept:
            Statement{Statement::Kind::Default},
            body{initBody} {}

        const Statement& body;
    };

    class WhileStatement final: public Statement
    {
    public:
        WhileStatement(const Construct& initCondition,
                       const Statement& initBody) noexcept:
            Statement{Statement::Kind::While},
            condition{initCondition},
            body{initBody} {}

        const Construct& condition;
        const Statement& body;
    };

    class DoStatement final: public Statement
    {
    public:
        DoStatement(const Expression& initCondition,
                    const Statement& initBody) noexcept:
            Statement{Statement::Kind::Do},
            condition{initCondition},
            body{initBody} {}

        const Expression& condition;
        const Statement& body;
    };

    class BreakStatement final: public Statement
    {
    public:
        BreakStatement() noexcept: Statement{Statement::Kind::Break} {}
    };

    class ContinueStatement final: public Statement
    {
    public:
        ContinueStatement() noexcept: Statement{Statement::Kind::Continue} {}
    };

    class ReturnStatement final: public Statement
    {
    public:
        explicit ReturnStatement(const Expression* initResult = nullptr) noexcept:
            Statement{Statement::Kind::Return},
            result{initResult} {}

        const Expression* result = nullptr;
    };

    inline std::string toString(Statement::Kind kind)
    {
        switch (kind)
        {
            case Statement::Kind::Empty: return "Empty";
            case Statement::Kind::Expression: return "Expression";
            case Statement::Kind::Declaration: return "Declaration";
            case Statement::Kind::Compound: return "Compound";
            case Statement::Kind::If: return "If";
            case Statement::Kind::For: return "For";
            case Statement::Kind::Switch: return "Switch";
            case Statement::Kind::Case: return "Case";
            case Statement::Kind::Default: return "Default";
            case Statement::Kind::While: return "While";
            case Statement::Kind::Do: return "Do";
            case Statement::Kind::Break: return "Break";
            case Statement::Kind::Continue: return "Continue";
            case Statement::Kind::Return: return "Return";
        }

        throw std::runtime_error("Unknown statement kind");
    }

    inline std::string toString(ScalarType::Kind kind)
    {
        switch (kind)
        {
            case ScalarType::Kind::Boolean: return "Boolean";
            case ScalarType::Kind::Integer: return "Integer";
            case ScalarType::Kind::FloatingPoint: return "FloatingPoint";
        }

        throw std::runtime_error("Unknown scalar type kind");
    }
}

#endif // STATEMENTS_HPP
