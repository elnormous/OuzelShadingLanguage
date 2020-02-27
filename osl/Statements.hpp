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
            Return
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

    class BadAccessError final: public std::runtime_error
    {
    public:
        explicit BadAccessError(const std::string& str): std::runtime_error(str) {}
        explicit BadAccessError(const char* str): std::runtime_error(str) {}
    };

    template <typename First, typename Second>
    class Variant final
    {
    public:
        Variant() noexcept = default;
        Variant(std::nullptr_t) noexcept: type(0) {}
        Variant(First* value) noexcept: type(1), pointer(value) {}
        Variant(Second* value) noexcept: type(2), pointer(value) {}

        Variant& operator=(std::nullptr_t) noexcept
        {
            type = 0;
            pointer = nullptr;
            return *this;
        }

        Variant& operator=(First* value) noexcept
        {
            type = 1;
            pointer = value;
            return *this;
        }

        Variant& operator=(Second* value) noexcept
        {
            type = 2;
            pointer = value;
            return *this;
        }

        template <class T, typename std::enable_if<std::is_same<T, First>::value>::type* = nullptr>
        bool is() const noexcept { return type == 1; }
        template <class T, typename std::enable_if<std::is_same<T, Second>::value>::type* = nullptr>
        bool is() const noexcept { return type == 2; }

        template <class T, typename std::enable_if<std::is_same<T, First>::value>::type* = nullptr>
        const First* get() const
        {
            if (type != 1) throw BadAccessError("Wrong type");
            return static_cast<const First*>(pointer);
        }

        template <class T, typename std::enable_if<std::is_same<T, Second>::value>::type* = nullptr>
        const Second* get() const
        {
            if (type != 2) throw BadAccessError("Wrong type");
            return static_cast<const Second*>(pointer);
        }

        template <class T, typename std::enable_if<std::is_same<T, First>::value>::type* = nullptr>
        First* get()
        {
            if (type != 1) throw BadAccessError("Wrong type");
            return static_cast<First*>(pointer);
        }

        template <class T, typename std::enable_if<std::is_same<T, Second>::value>::type* = nullptr>
        Second* get()
        {
            if (type != 2) throw BadAccessError("Wrong type");
            return static_cast<Second*>(pointer);
        }

    private:
        size_t type = 0;
        void* pointer = nullptr;
    };

    using DeclarationOrExpression = Variant<Declaration, Expression>;

    class IfStatement final: public Statement
    {
    public:
        IfStatement() noexcept: Statement(Statement::Kind::If) {}

        DeclarationOrExpression condition;
        const Statement* body = nullptr;
        const Statement* elseBody = nullptr;
    };

    class ForStatement final: public Statement
    {
    public:
        ForStatement() noexcept: Statement(Statement::Kind::For) {}

        DeclarationOrExpression initialization;
        DeclarationOrExpression condition;
        const Expression* increment = nullptr;
        const Statement* body = nullptr;
    };

    class SwitchStatement final: public Statement
    {
    public:
        SwitchStatement() noexcept: Statement(Statement::Kind::Switch) {}

        DeclarationOrExpression condition;
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

        DeclarationOrExpression condition;
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
}

#endif // STATEMENTS_HPP
