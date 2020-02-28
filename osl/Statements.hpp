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
        template <class T>
        Variant(T value) noexcept: type(TypeId<T>::id), pointer(value) {}

        template <class T>
        Variant& operator=(T value) noexcept
        {
            type = TypeId<T>::id;
            pointer = value;
            return *this;
        }

        template <class T>
        bool is() const noexcept { return type == TypeId<T>::id; }

        template <class T>
        const T get() const
        {
            if (type != TypeId<T>::id) throw BadAccessError("Wrong type");
            return static_cast<const T>(pointer);
        }

        template <class T>
        T get()
        {
            if (type != TypeId<T>::id) throw BadAccessError("Wrong type");
            return static_cast<T>(pointer);
        }

    private:
        template<typename T> struct TypeId;
        template<> struct TypeId<std::nullptr_t> { static constexpr size_t id = 0; };
        template<> struct TypeId<First> { static constexpr size_t id = 1; };
        template<> struct TypeId<Second> { static constexpr size_t id = 2; };

        size_t type = 0;
        void* pointer = nullptr;
    };
    
    using DeclarationOrExpression = Variant<DeclarationStatement*, Expression*>;

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
