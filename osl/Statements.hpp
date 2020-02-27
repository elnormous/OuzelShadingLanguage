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

    protected:
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

    class DeclarationOrExpression final
    {
    public:
        DeclarationOrExpression() noexcept = default;
        DeclarationOrExpression(std::nullptr_t) noexcept:
            type(Type::None) {}
        DeclarationOrExpression(Declaration* declaration) noexcept:
            type(Type::Declaration), pointer(declaration) {}
        DeclarationOrExpression(Expression* expression) noexcept:
            type(Type::Expression), pointer(expression) {}

        DeclarationOrExpression& operator=(std::nullptr_t) noexcept
        {
            type = Type::None;
            pointer = nullptr;
            return *this;
        }

        DeclarationOrExpression& operator=(Declaration* declaration) noexcept
        {
            type = Type::Declaration;
            pointer = declaration;
            return *this;
        }

        DeclarationOrExpression& operator=(Expression* expression) noexcept
        {
            type = Type::Expression;
            pointer = expression;
            return *this;
        }

        template <class T, typename std::enable_if<std::is_same<T, Declaration>::value>::type* = nullptr>
        bool is() const noexcept { return type == Type::Declaration; }
        template <class T, typename std::enable_if<std::is_same<T, Expression>::value>::type* = nullptr>
        bool is() const noexcept { return type == Type::Expression; }

        template <class T, typename std::enable_if<std::is_same<T, Declaration>::value>::type* = nullptr>
        const Declaration* get() const
        {
            if (type != Type::Declaration) throw BadAccessError("Not a Declaration");
            return static_cast<const Declaration*>(pointer);
        }

        template <class T, typename std::enable_if<std::is_same<T, Expression>::value>::type* = nullptr>
        const Expression* get() const
        {
            if (type != Type::Expression) throw BadAccessError("Not a Statement");
            return static_cast<const Expression*>(pointer);
        }

        template <class T, typename std::enable_if<std::is_same<T, Declaration>::value>::type* = nullptr>
        Declaration* get()
        {
            if (type != Type::Declaration) throw BadAccessError("Not a Declaration");
            return static_cast<Declaration*>(pointer);
        }

        template <class T, typename std::enable_if<std::is_same<T, Expression>::value>::type* = nullptr>
        Expression* get()
        {
            if (type != Type::Expression) throw BadAccessError("Not a Statement");
            return static_cast<Expression*>(pointer);
        }

    private:
        enum class Type
        {
            None,
            Declaration,
            Expression
        };
        Type type = Type::None;
        void* pointer = nullptr;
    };

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
