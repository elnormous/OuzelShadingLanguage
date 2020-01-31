//
//  OSL
//

#include <iostream>
#include <stdexcept>
#include <type_traits>
#include "Parser.hpp"

namespace
{
    class TestError final: public std::logic_error
    {
    public:
        explicit TestError(const std::string& str): std::logic_error(str) {}
        explicit TestError(const char* str): std::logic_error(str) {}
    };

    class TestRunner final
    {
    public:
        template <class T, class ...Args>
        void run(T test, Args ...args)
        {
            try
            {
                test(args...);
            }
            catch (std::exception& e)
            {
                std::cerr << e.what() << '\n';
                result = false;
            }
        }

        bool getResult() const noexcept { return result; }

    private:
        bool result = true;
    };

    const CompoundStatement* getMainBody(const ASTContext& context)
    {
        auto topDeclarations = context.getDeclarations();
        if (topDeclarations.size() != 1)
            throw TestError("Expected the main function");

        auto declaration = static_cast<const Declaration*>(topDeclarations.front());

        if (!declaration ||
            declaration->name != "main" ||
            declaration->getDeclarationKind() != Declaration::Kind::Callable)
            throw TestError("Expected a callable declaration of main");

        auto callableDeclaration = static_cast<const CallableDeclaration*>(declaration);

        auto body = callableDeclaration->body;

        if (!body ||
            body->getStatementKind() != Statement::Kind::Compound)
            throw TestError("Expected a compound statement");

        return static_cast<const CompoundStatement*>(body);
    }

    const Expression* getMainExpression(const ASTContext& context)
    {
        auto mainCompoundStatement = getMainBody(context);

        if (mainCompoundStatement->statements.size() != 1)
            throw TestError("Expected a statement");

        auto statement = mainCompoundStatement->statements.front();

        if (!statement ||
            statement->getStatementKind() != Statement::Kind::Expression)
            throw TestError("Expected an expression statement");

        auto expressionStatement = static_cast<const ExpressionStatement*>(statement);

        return expressionStatement->expression;
    }

    const LiteralExpression* getLiteralExpression(const Expression* expression)
    {
        if (!expression)
            throw TestError("Expected an expression");

        if (expression->category != Expression::Category::Rvalue)
            throw TestError("Expected must rvalue");

        if ((expression->qualifiedType.qualifiers & Qualifiers::Const) != Qualifiers::Const)
            throw TestError("Expected must be const");

        if (expression->getExpressionKind() != Expression::Kind::Literal)
            throw TestError("Expected a literal expression");

        return static_cast<const LiteralExpression*>(expression);
    }

    template <class T, typename std::enable_if<std::is_same<T, bool>::value>::type* = nullptr>
    void expectLiteral(const Expression* expression, T value)
    {
        auto literalExpression = getLiteralExpression(expression);

        if (literalExpression->getLiteralKind() != LiteralExpression::Kind::Boolean)
            throw TestError("Expected a boolean literal expression");

        auto booleanLiteralExpression = static_cast<const BooleanLiteralExpression*>(literalExpression);

        if (booleanLiteralExpression->value != value)
            throw TestError("Wrong literal value");
    }

    template <class T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type* = nullptr>
    void expectLiteral(const Expression* expression, T value)
    {
        auto literalExpression = getLiteralExpression(expression);

        if (literalExpression->getLiteralKind() != LiteralExpression::Kind::Integer)
            throw TestError("Expected an integer literal expression");

        auto integerLiteralExpression = static_cast<const IntegerLiteralExpression*>(literalExpression);

        if (integerLiteralExpression->value != value)
            throw TestError("Wrong literal value");
    }

    template <class T, typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
    void expectLiteral(const Expression* expression, T value)
    {
        auto literalExpression = getLiteralExpression(expression);

        if (literalExpression->getLiteralKind() != LiteralExpression::Kind::FloatingPoint)
            throw TestError("Expected a floating point literal expression");

        auto floatLiteralExpression = static_cast<const FloatingPointLiteralExpression*>(literalExpression);

        if (floatLiteralExpression->value != value)
            throw TestError("Wrong literal value");
    }

    void testEmptyStatement()
    {
        std::string code = R"OSL(
        void main()
        {
            ;
        }
        )OSL";

        ASTContext context(tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        if (mainCompoundStatement->statements.size() != 1)
            throw TestError("Expected a statement");

        auto statement = mainCompoundStatement->statements.front();

        if (!statement ||
            statement->getStatementKind() != Statement::Kind::Empty)
            throw TestError("Expected an empty statement");
    }

    void testDeclaration()
    {
        std::string code = R"OSL(
        void main()
        {
            int i = 3;
        }
        )OSL";

        ASTContext context(tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        if (mainCompoundStatement->statements.size() != 1)
            throw TestError("Expected a statement");

        auto statement = mainCompoundStatement->statements.front();

        if (!statement ||
            statement->getStatementKind() != Statement::Kind::Declaration)
            throw TestError("Expected a declaration");

        auto iDeclarationStatement = static_cast<const DeclarationStatement*>(statement);
        auto iDeclaration = iDeclarationStatement->declaration;

        if (!iDeclaration ||
            iDeclaration->name != "i" ||
            iDeclaration->getDeclarationKind() != Declaration::Kind::Variable)
            throw TestError("Expected a variable declaration of i");

        auto iVariableDeclaration = static_cast<const VariableDeclaration*>(iDeclaration);

        if (!iVariableDeclaration->qualifiedType.type ||
            iVariableDeclaration->qualifiedType.type->declaration ||
            iVariableDeclaration->qualifiedType.type->name != "int")
            throw TestError("Expected a declaration of a variable of type int");

        expectLiteral(iVariableDeclaration->initialization, 3);
    }

    void testIfStatement()
    {
        std::string code = R"OSL(
        void main()
        {
            if (true)
            {
            }
            else if (false)
            {
            }
            else
            {
            }
        }
        )OSL";

        ASTContext context(tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        if (mainCompoundStatement->statements.size() != 1)
            throw TestError("Expected a statement");

        const auto statement = mainCompoundStatement->statements.front();

        if (!statement ||
            statement->getStatementKind() != Statement::Kind::If)
            throw TestError("Expected an if statement");

        auto ifStatement = static_cast<const IfStatement*>(statement);

        if (!ifStatement->condition ||
            ifStatement->condition->getKind() != Construct::Kind::Expression)
            throw TestError("Expected an expression condition");

        auto condition = static_cast<const Expression*>(ifStatement->condition);

        expectLiteral(condition, true);

        if (!ifStatement->body ||
            ifStatement->body->getStatementKind() != Statement::Kind::Compound)
            throw TestError("Expected a compound statement if part");

        if (!ifStatement->elseBody ||
            ifStatement->elseBody->getStatementKind() != Statement::Kind::If)
            throw TestError("Expected a compound statement else if part");

        auto elseIfStatement = static_cast<const IfStatement*>(ifStatement->elseBody);

        if (!elseIfStatement->condition ||
            elseIfStatement->condition->getKind() != Construct::Kind::Expression)
            throw TestError("Expected an expression condition");

        auto elseIfCondition = static_cast<const Expression*>(elseIfStatement->condition);

        expectLiteral(elseIfCondition, false);

        if (!elseIfStatement->elseBody ||
            elseIfStatement->elseBody->getStatementKind() != Statement::Kind::Compound)
            throw TestError("Expected a compound statement else part");
    }

    void testWhileStatement()
    {
        std::string code = R"OSL(
        void main()
        {
            while (true)
            {
            }
        }
        )OSL";

        ASTContext context(tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        if (mainCompoundStatement->statements.size() != 1)
            throw TestError("Expected a statement");

        auto statement = mainCompoundStatement->statements.front();

        if (!statement ||
            statement->getStatementKind() != Statement::Kind::While)
            throw TestError("Expected a while statement");

        auto whileStatement = static_cast<const WhileStatement*>(statement);

        if (!whileStatement->condition ||
            whileStatement->condition->getKind() != Construct::Kind::Expression)
            throw TestError("Expected an expression condition");

        auto condition = static_cast<const Expression*>(whileStatement->condition);

        expectLiteral(condition, true);

        if (!whileStatement->body ||
            whileStatement->body->getStatementKind() != Statement::Kind::Compound)
            throw TestError("Expected a compound statement");
    }

    void testDoStatement()
    {
        std::string code = R"OSL(
        void main()
        {
            do
            {
            }
            while (true);
        }
        )OSL";

        ASTContext context(tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        if (mainCompoundStatement->statements.size() != 1)
            throw TestError("Expected a statement");

        auto statement = mainCompoundStatement->statements.front();

        if (!statement ||
            statement->getStatementKind() != Statement::Kind::Do)
            throw TestError("Expected a do statement");

        auto doStatement = static_cast<const DoStatement*>(statement);

        if (!doStatement->condition ||
            doStatement->condition->getKind() != Construct::Kind::Expression)
            throw TestError("Expected an expression condition");

        auto condition = static_cast<const Expression*>(doStatement->condition);

        expectLiteral(condition, true);

        if (!doStatement->body ||
            doStatement->body->getStatementKind() != Statement::Kind::Compound)
            throw TestError("Expected a compound statement");
    }

    void testForStatement()
    {
        std::string code = R"OSL(
        void main()
        {
            for (; true;)
            {
            }
        }
        )OSL";

        ASTContext context(tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        if (mainCompoundStatement->statements.size() != 1)
            throw TestError("Expected a statement");

        auto statement = mainCompoundStatement->statements.front();

        if (!statement ||
            statement->getStatementKind() != Statement::Kind::For)
            throw TestError("Expected a for statement");

        auto forStatement = static_cast<const ForStatement*>(statement);

        if (!forStatement->condition ||
            forStatement->condition->getKind() != Construct::Kind::Expression)
            throw TestError("Expected an expression condition");

        auto condition = static_cast<const Expression*>(forStatement->condition);

        expectLiteral(condition, true);

        if (!forStatement->body ||
            forStatement->body->getStatementKind() != Statement::Kind::Compound)
            throw TestError("Expected a compound statement");
    }

    void testSwitchStatement()
    {
        std::string code = R"OSL(
        void main()
        {
            switch (1)
            {
                case 1:;
                case 2: break;
                default:;
            }
        }
        )OSL";

        ASTContext context(tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        if (mainCompoundStatement->statements.size() != 1)
            throw TestError("Expected a statement");

        auto statement = mainCompoundStatement->statements.front();

        if (!statement ||
            statement->getStatementKind() != Statement::Kind::Switch)
            throw TestError("Expected a switch statement");

        auto switchStatement = static_cast<const SwitchStatement*>(statement);

        if (!switchStatement->condition ||
            switchStatement->condition->getKind() != Construct::Kind::Expression)
            throw TestError("Expected an expression condition");

        auto condition = static_cast<const Expression*>(switchStatement->condition);

        expectLiteral(condition, 1);

        if (!switchStatement->body ||
            switchStatement->body->getStatementKind() != Statement::Kind::Compound)
            throw TestError("Expected a compound statement");

        auto body = static_cast<const CompoundStatement*>(switchStatement->body);

        if (body->statements.size() != 3)
            throw TestError("Expected 3 statements");

        if (body->statements[0]->getStatementKind() != Statement::Kind::Case)
            throw TestError("Expected a case statement");

        auto firstCaseStatement = static_cast<const CaseStatement*>(body->statements[0]);

        expectLiteral(firstCaseStatement->condition, 1);

        if (!firstCaseStatement->body ||
            firstCaseStatement->body->getStatementKind() != Statement::Kind::Empty)
            throw TestError("Expected an empty statement");

        if (body->statements[1]->getStatementKind() != Statement::Kind::Case)
            throw TestError("Expected a case statement");

        auto secondCaseStatement = static_cast<const CaseStatement*>(body->statements[1]);

        expectLiteral(secondCaseStatement->condition, 2);

        if (!secondCaseStatement->body ||
            secondCaseStatement->body->getStatementKind() != Statement::Kind::Break)
            throw TestError("Expected an break statement");

        if (body->statements[2]->getStatementKind() != Statement::Kind::Default)
            throw TestError("Expected a default statement");

        auto defaultStatement = static_cast<const DefaultStatement*>(body->statements[2]);

        if (!defaultStatement->body ||
            defaultStatement->body->getStatementKind() != Statement::Kind::Empty)
            throw TestError("Expected an empty statement");
    }

    void testReturnStatement()
    {
        std::string code = R"OSL(
        int main()
        {
            return 1;
        }
        )OSL";

        ASTContext context(tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        if (mainCompoundStatement->statements.size() != 1)
            throw TestError("Expected a statement");

        auto statement = mainCompoundStatement->statements.front();

        if (!statement ||
            statement->getStatementKind() != Statement::Kind::Return)
            throw TestError("Expected a return statement");

        auto returnStatement = static_cast<const ReturnStatement*>(statement);

        expectLiteral(returnStatement->result, 1);
    }

    void testBoolLiteral()
    {
        std::string code = R"OSL(
        void main()
        {
            true;
        }
        )OSL";

        ASTContext context(tokenize(code));
        expectLiteral(getMainExpression(context), true);
    }

    void testIntLiteral()
    {
        std::string code = R"OSL(
        void main()
        {
            1;
        }
        )OSL";

        ASTContext context(tokenize(code));
        expectLiteral(getMainExpression(context), 1);
    }

    void testFloatLiteral()
    {
        std::string code = R"OSL(
        void main()
        {
            1.0F;
        }
        )OSL";

        ASTContext context(tokenize(code));
        expectLiteral(getMainExpression(context), 1.0F);
    }

    void testLiteralExpressions()
    {
        testBoolLiteral();
        testIntLiteral();
        testFloatLiteral();
    }

    void testExpressions()
    {
        testLiteralExpressions();
    }

    void testStructDeclaration()
    {
        std::string code = R"OSL(
        struct Foo;

        struct Foo
        {
            float f;
        };

        void main()
        {
        }
        )OSL";

        ASTContext context(tokenize(code));

        auto i = context.getDeclarations().begin();
        if (i == context.getDeclarations().end())
            throw TestError("Expected a struct declaration");

        const auto structDeclaration = *i;

        if (structDeclaration->getDeclarationKind() != Declaration::Kind::Type)
            throw TestError("Expected a type declaration");

        auto structTypeDeclaration = static_cast<const TypeDeclaration*>(structDeclaration);

        if (structTypeDeclaration->name != "Foo")
            throw TestError("Expected a declaration of Foo");

        auto structDeclarationType = structTypeDeclaration->type;

        if (!structDeclarationType || structDeclarationType->name != "Foo")
            throw TestError("Expected a type named Foo");

        if (structDeclarationType->declaration != structTypeDeclaration)
            throw TestError("Wrong declaration");

        if (++i == context.getDeclarations().end())
            throw TestError("Expected a struct definition");

        const auto structDefinition = *i;

        if (structDefinition->getDeclarationKind() != Declaration::Kind::Type)
            throw TestError("Expected a type declaration");

        auto structTypeDefinition = static_cast<const TypeDeclaration*>(structDefinition);

        if (structTypeDefinition->name != "Foo")
            throw TestError("Expected a declaration of Foo");

        auto structDefinitionType = structTypeDefinition->type;

        if (structDeclarationType != structDefinitionType)
            throw TestError("Expected the same type for declaration and definition");

        if (structDefinitionType->definition != structTypeDefinition)
            throw TestError("Wrong definition");
    }

    void testVector()
    {
        std::string code = R"OSL(
        void main()
        {
            float4 v1;
            float4 v2;
            v1.xyzw = v2.xxxx;
            float v3 = v1[0];
        }
        )OSL";

        ASTContext context(tokenize(code));
    }

    void testMatrix()
    {
        std::string code = R"OSL(
        void main()
        {
            float4x4 m1;
            float4 v1 = m1[0];
        }
        )OSL";

        ASTContext context(tokenize(code));
    }
}

int main(int argc, const char * argv[])
{
    TestRunner testRunner;
    testRunner.run(testEmptyStatement);
    testRunner.run(testDeclaration);
    testRunner.run(testIfStatement);
    testRunner.run(testWhileStatement);
    testRunner.run(testDoStatement);
    testRunner.run(testForStatement);
    testRunner.run(testSwitchStatement);
    testRunner.run(testReturnStatement);
    testRunner.run(testExpressions);
    testRunner.run(testStructDeclaration);
    testRunner.run(testVector);
    testRunner.run(testMatrix);

    if (testRunner.getResult())
        std::cout << "Success\n";

    return testRunner.getResult() ? EXIT_SUCCESS : EXIT_FAILURE;
}
