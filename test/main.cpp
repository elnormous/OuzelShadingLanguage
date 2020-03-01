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

    const ouzel::CompoundStatement* getMainBody(const ouzel::ASTContext& context)
    {
        auto i = context.getDeclarations().begin();
        if (i == context.getDeclarations().end())
            throw TestError("Expected the main function");

        auto declaration = *i;

        if (!declaration ||
            declaration->name != "main" ||
            declaration->getDeclarationKind() != ouzel::Declaration::Kind::Callable)
            throw TestError("Expected a callable declaration of main");

        auto callableDeclaration = static_cast<const ouzel::CallableDeclaration*>(declaration);

        auto body = callableDeclaration->body;

        if (!body ||
            body->getStatementKind() != ouzel::Statement::Kind::Compound)
            throw TestError("Expected a compound statement");

        return static_cast<const ouzel::CompoundStatement*>(body);
    }

    const ouzel::Expression* getMainExpression(const ouzel::ASTContext& context)
    {
        auto mainCompoundStatement = getMainBody(context);

        auto i = mainCompoundStatement->statements.begin();
        if (i == mainCompoundStatement->statements.end())
            throw TestError("Expected a statement");

        auto statement = *i;

        if (!statement ||
            statement->getStatementKind() != ouzel::Statement::Kind::Expression)
            throw TestError("Expected an expression statement");

        auto expressionStatement = static_cast<const ouzel::ExpressionStatement*>(statement);

        return expressionStatement->expression;
    }

    const ouzel::LiteralExpression* getLiteralExpression(const ouzel::Expression* expression)
    {
        if (!expression)
            throw TestError("Expected an expression");

        if (expression->category != ouzel::Expression::Category::Rvalue)
            throw TestError("Expected must rvalue");

        if ((expression->qualifiedType.qualifiers & ouzel::Qualifiers::Const) != ouzel::Qualifiers::Const)
            throw TestError("Expected must be const");

        if (expression->getExpressionKind() != ouzel::Expression::Kind::Literal)
            throw TestError("Expected a literal expression");

        return static_cast<const ouzel::LiteralExpression*>(expression);
    }

    template <class T, typename std::enable_if<std::is_same<T, bool>::value>::type* = nullptr>
    void expectLiteral(const ouzel::Expression* expression, T value)
    {
        auto literalExpression = getLiteralExpression(expression);

        if (literalExpression->getLiteralKind() != ouzel::LiteralExpression::Kind::Boolean)
            throw TestError("Expected a boolean literal expression");

        auto booleanLiteralExpression = static_cast<const ouzel::BooleanLiteralExpression*>(literalExpression);

        if (booleanLiteralExpression->value != value)
            throw TestError("Wrong literal value");
    }

    template <class T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type* = nullptr>
    void expectLiteral(const ouzel::Expression* expression, T value)
    {
        auto literalExpression = getLiteralExpression(expression);

        if (literalExpression->getLiteralKind() != ouzel::LiteralExpression::Kind::Integer)
            throw TestError("Expected an integer literal expression");

        auto integerLiteralExpression = static_cast<const ouzel::IntegerLiteralExpression*>(literalExpression);

        if (integerLiteralExpression->value != value)
            throw TestError("Wrong literal value");
    }

    template <class T, typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
    void expectLiteral(const ouzel::Expression* expression, T value)
    {
        auto literalExpression = getLiteralExpression(expression);

        if (literalExpression->getLiteralKind() != ouzel::LiteralExpression::Kind::FloatingPoint)
            throw TestError("Expected a floating point literal expression");

        auto floatLiteralExpression = static_cast<const ouzel::FloatingPointLiteralExpression*>(literalExpression);

        if (floatLiteralExpression->value != value)
            throw TestError("Wrong literal value");
    }

    void testEmptyStatement()
    {
        std::string code = R"OSL(
        function main():void
        {
            ;
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        auto i = mainCompoundStatement->statements.begin();
        if (i == mainCompoundStatement->statements.end())
            throw TestError("Expected a statement");

        auto statement = *i;

        if (!statement ||
            statement->getStatementKind() != ouzel::Statement::Kind::Empty)
            throw TestError("Expected an empty statement");
    }

    void testDeclaration()
    {
        std::string code = R"OSL(
        function main():void
        {
            var i:int = 3;
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        auto i = mainCompoundStatement->statements.begin();
        if (i == mainCompoundStatement->statements.end())
            throw TestError("Expected a statement");

        auto statement = *i;

        if (!statement ||
            statement->getStatementKind() != ouzel::Statement::Kind::Declaration)
            throw TestError("Expected a declaration");

        auto iDeclarationStatement = static_cast<const ouzel::DeclarationStatement*>(statement);
        auto iDeclaration = iDeclarationStatement->declaration;

        if (!iDeclaration ||
            iDeclaration->name != "i" ||
            iDeclaration->getDeclarationKind() != ouzel::Declaration::Kind::Variable)
            throw TestError("Expected a variable declaration of i");

        auto iVariableDeclaration = static_cast<const ouzel::VariableDeclaration*>(iDeclaration);

        if (!iVariableDeclaration->qualifiedType.type ||
            iVariableDeclaration->qualifiedType.type->name != "int")
            throw TestError("Expected a declaration of a variable of type int");

        expectLiteral(iVariableDeclaration->initialization, 3);
    }

    void testIfStatement()
    {
        std::string code = R"OSL(
        function main():void
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

        ouzel::ASTContext context(ouzel::tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        auto i = mainCompoundStatement->statements.begin();
        if (i == mainCompoundStatement->statements.end())
            throw TestError("Expected a statement");

        const auto statement = *i;

        if (!statement ||
            statement->getStatementKind() != ouzel::Statement::Kind::If)
            throw TestError("Expected an if statement");

        auto ifStatement = static_cast<const ouzel::IfStatement*>(statement);

        if (!ifStatement->condition ||
            ifStatement->condition->getKind() != ouzel::Construct::Kind::Expression)
            throw TestError("Expected an expression condition");

        auto condition = static_cast<const ouzel::Expression*>(ifStatement->condition);

        expectLiteral(condition, true);

        if (!ifStatement->body ||
            ifStatement->body->getStatementKind() != ouzel::Statement::Kind::Compound)
            throw TestError("Expected a compound statement if part");

        if (!ifStatement->elseBody ||
            ifStatement->elseBody->getStatementKind() != ouzel::Statement::Kind::If)
            throw TestError("Expected a compound statement else if part");

        auto elseIfStatement = static_cast<const ouzel::IfStatement*>(ifStatement->elseBody);

        if (!elseIfStatement->condition ||
            elseIfStatement->condition->getKind() != ouzel::Construct::Kind::Expression)
            throw TestError("Expected an expression condition");

        auto elseIfCondition = static_cast<const ouzel::Expression*>(elseIfStatement->condition);

        expectLiteral(elseIfCondition, false);

        if (!elseIfStatement->elseBody ||
            elseIfStatement->elseBody->getStatementKind() != ouzel::Statement::Kind::Compound)
            throw TestError("Expected a compound statement else part");
    }

    void testWhileStatement()
    {
        std::string code = R"OSL(
        function main():void
        {
            while (true)
            {
            }
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        auto i = mainCompoundStatement->statements.begin();
        if (i == mainCompoundStatement->statements.end())
            throw TestError("Expected a statement");

        auto statement = *i;

        if (!statement ||
            statement->getStatementKind() != ouzel::Statement::Kind::While)
            throw TestError("Expected a while statement");

        auto whileStatement = static_cast<const ouzel::WhileStatement*>(statement);

        if (!whileStatement->condition ||
            whileStatement->condition->getKind() != ouzel::Construct::Kind::Expression)
            throw TestError("Expected an expression condition");

        auto condition = static_cast<const ouzel::Expression*>(whileStatement->condition);

        expectLiteral(condition, true);

        if (!whileStatement->body ||
            whileStatement->body->getStatementKind() != ouzel::Statement::Kind::Compound)
            throw TestError("Expected a compound statement");
    }

    void testDoStatement()
    {
        std::string code = R"OSL(
        function main():void
        {
            do
            {
            }
            while (true);
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        auto i = mainCompoundStatement->statements.begin();
        if (i == mainCompoundStatement->statements.end())
            throw TestError("Expected a statement");

        auto statement = *i;

        if (!statement ||
            statement->getStatementKind() != ouzel::Statement::Kind::Do)
            throw TestError("Expected a do statement");

        auto doStatement = static_cast<const ouzel::DoStatement*>(statement);

        if (!doStatement->condition ||
            doStatement->condition->getKind() != ouzel::Construct::Kind::Expression)
            throw TestError("Expected an expression condition");

        auto condition = static_cast<const ouzel::Expression*>(doStatement->condition);

        expectLiteral(condition, true);

        if (!doStatement->body ||
            doStatement->body->getStatementKind() != ouzel::Statement::Kind::Compound)
            throw TestError("Expected a compound statement");
    }

    void testForStatement()
    {
        std::string code = R"OSL(
        function main():void
        {
            for (; true;)
            {
            }
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        auto i = mainCompoundStatement->statements.begin();
        if (i == mainCompoundStatement->statements.end())
            throw TestError("Expected a statement");

        auto statement = *i;

        if (!statement ||
            statement->getStatementKind() != ouzel::Statement::Kind::For)
            throw TestError("Expected a for statement");

        auto forStatement = static_cast<const ouzel::ForStatement*>(statement);

        if (!forStatement->condition ||
            forStatement->condition->getKind() != ouzel::Construct::Kind::Expression)
            throw TestError("Expected an expression condition");

        auto condition = static_cast<const ouzel::Expression*>(forStatement->condition);

        expectLiteral(condition, true);

        if (!forStatement->body ||
            forStatement->body->getStatementKind() != ouzel::Statement::Kind::Compound)
            throw TestError("Expected a compound statement");
    }

    void testSwitchStatement()
    {
        std::string code = R"OSL(
        function main():void
        {
            switch (1)
            {
                case 1:;
                case 2: break;
                default:;
            }
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        auto i = mainCompoundStatement->statements.begin();
        if (i == mainCompoundStatement->statements.end())
            throw TestError("Expected a statement");

        auto statement = *i;

        if (!statement ||
            statement->getStatementKind() != ouzel::Statement::Kind::Switch)
            throw TestError("Expected a switch statement");

        auto switchStatement = static_cast<const ouzel::SwitchStatement*>(statement);

        if (!switchStatement->condition ||
            switchStatement->condition->getKind() != ouzel::Construct::Kind::Expression)
            throw TestError("Expected an expression condition");

        auto condition = static_cast<const ouzel::Expression*>(switchStatement->condition);

        expectLiteral(condition, 1);

        if (!switchStatement->body ||
            switchStatement->body->getStatementKind() != ouzel::Statement::Kind::Compound)
            throw TestError("Expected a compound statement");

        auto body = static_cast<const ouzel::CompoundStatement*>(switchStatement->body);

        if (body->statements.size() != 3)
            throw TestError("Expected 3 statements");

        if (body->statements[0]->getStatementKind() != ouzel::Statement::Kind::Case)
            throw TestError("Expected a case statement");

        auto firstCaseStatement = static_cast<const ouzel::CaseStatement*>(body->statements[0]);

        expectLiteral(firstCaseStatement->condition, 1);

        if (!firstCaseStatement->body ||
            firstCaseStatement->body->getStatementKind() != ouzel::Statement::Kind::Empty)
            throw TestError("Expected an empty statement");

        if (body->statements[1]->getStatementKind() != ouzel::Statement::Kind::Case)
            throw TestError("Expected a case statement");

        auto secondCaseStatement = static_cast<const ouzel::CaseStatement*>(body->statements[1]);

        expectLiteral(secondCaseStatement->condition, 2);

        if (!secondCaseStatement->body ||
            secondCaseStatement->body->getStatementKind() != ouzel::Statement::Kind::Break)
            throw TestError("Expected an break statement");

        if (body->statements[2]->getStatementKind() != ouzel::Statement::Kind::Default)
            throw TestError("Expected a default statement");

        auto defaultStatement = static_cast<const ouzel::DefaultStatement*>(body->statements[2]);

        if (!defaultStatement->body ||
            defaultStatement->body->getStatementKind() != ouzel::Statement::Kind::Empty)
            throw TestError("Expected an empty statement");
    }

    void testReturnStatement()
    {
        std::string code = R"OSL(
        function main():int
        {
            return 1;
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
        auto mainCompoundStatement = getMainBody(context);

        auto i = mainCompoundStatement->statements.begin();
        if (i == mainCompoundStatement->statements.end())
            throw TestError("Expected a statement");

        auto statement = *i;

        if (!statement ||
            statement->getStatementKind() != ouzel::Statement::Kind::Return)
            throw TestError("Expected a return statement");

        auto returnStatement = static_cast<const ouzel::ReturnStatement*>(statement);

        expectLiteral(returnStatement->result, 1);
    }

    void testBoolLiteral()
    {
        std::string code = R"OSL(
        function main():void
        {
            true;
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
        expectLiteral(getMainExpression(context), true);
    }

    void testIntLiteral()
    {
        std::string code = R"OSL(
        function main():void
        {
            1;
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
        expectLiteral(getMainExpression(context), 1);
    }

    void testFloatLiteral()
    {
        std::string code = R"OSL(
        function main():void
        {
            1.0F;
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
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
        struct Foo
        {
            var f:float;
        }

        function main()
        {
            var f:Foo;
            f.f = 1.0f;
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));

        auto i = context.getDeclarations().begin();
        if (i == context.getDeclarations().end())
            throw TestError("Expected a struct declaration");

        const auto structDefinition = *i;

        if (structDefinition->getDeclarationKind() != ouzel::Declaration::Kind::Type)
            throw TestError("Expected a type declaration");

        auto structTypeDefinition = static_cast<const ouzel::TypeDeclaration*>(structDefinition);

        if (structTypeDefinition->name != "Foo")
            throw TestError("Expected a declaration of Foo");

        auto structDefinitionType = structTypeDefinition->type;

        if (structTypeDefinition->name != structDefinitionType->name)
            throw TestError("Wrong type name");
    }

    void testFunction()
    {
        std::string code = R"OSL(
        function foo(a:float):float { return 1.0f; }
        function foo(a:int):float { return 1.0f; }

        function main():void
        {
            foo(1);
            foo(1.0f);
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
    }

    void testArray()
    {
        std::string code = R"OSL(
        function main():void
        {
            var a:float[4];
            a[0] = 1.0f;
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
    }

    void testFloat()
    {
        std::string code = R"OSL(
        function main():void
        {
            var f1:float = 1.0f;
            var f2:float = ++f1;
            var f3:float = f2++;
            var f4:float = float(1);
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
    }

    void testVector()
    {
        std::string code = R"OSL(
        function main():void
        {
            var v1:float4;
            var v2:float4;
            v1.xyzw = v2.xxxx;
            var f1:float = v1[0];
            v1[0] = f1;
            var v3:float4 = float4(v1.xyz, 1.0f);
            v3 = float4(v2);
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
    }

    void testMatrix()
    {
        std::string code = R"OSL(
        function main();
        function main()
        {
            var m1:float4x4;
            var v1:float4 = m1[0];
            var f1:float = m1[0][0];
            m1[0][0] = f1;
            var m2:float2x2 = float2x2(v1.xy, v1.zw);
            var m3:float4x4 = float4x4(m1);
            var v2:float4 = m1 * v1;
            return m1;
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
    }

    void testSemantics()
    {
        std::string code = R"OSL(
            struct Vertex
            {
                var position:float3 -> Position;
                var normal:float3 -> Normal;
                var texCoord0:float2 -> TextureCoordinates(0);
                var texCoord1:float2 -> TextureCoordinates(1);
            }
            struct Fragment
            {
                var depth:float3 -> Depth(0);
            }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
}

    void testInputModifiers()
    {
        std::string code = R"OSL(
        function fragmentMain(in i:float4, out o:float4):float
        {
            return 0.0f;
        }
        function vertexMain(inout io:float4):float
        {
            return 0.0f;
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));

        auto i = context.getDeclarations().begin();
        if (i == context.getDeclarations().end())
            throw TestError("Expected a function declaration");

        const auto fragmentMainDeclaration = *i;

        if (fragmentMainDeclaration->getDeclarationKind() != ouzel::Declaration::Kind::Callable)
            throw TestError("Expected a callable declaration");

        auto fragmentMainCallableDeclaration = static_cast<const ouzel::CallableDeclaration*>(fragmentMainDeclaration);

        if (fragmentMainCallableDeclaration->name != "fragmentMain")
            throw TestError("Expected a declaration of fragmentMain");

        if (fragmentMainCallableDeclaration->getCallableDeclarationKind() != ouzel::CallableDeclaration::Kind::Function)
            throw TestError("Expected a function declaration");

        auto fragmentMainFunctionDeclaration = static_cast<const ouzel::FunctionDeclaration*>(fragmentMainCallableDeclaration);

        {
            auto parameterIterator = fragmentMainFunctionDeclaration->parameterDeclarations.begin();

            if (parameterIterator == fragmentMainFunctionDeclaration->parameterDeclarations.end())
                throw TestError("Expected an i parameter");

            auto iParameterDeclaration = *parameterIterator;
            ++parameterIterator;

            if (iParameterDeclaration->name != "i")
                throw TestError("Expected an i parameter");

            if (iParameterDeclaration->inputModifier != ouzel::InputModifier::In)
                throw TestError("Expected i to be in");

            if (parameterIterator == fragmentMainFunctionDeclaration->parameterDeclarations.end())
                throw TestError("Expected an o parameter");

            auto oParameterDeclaration = *parameterIterator;

            if (oParameterDeclaration->name != "o")
                throw TestError("Expected an o parameter");

            if (oParameterDeclaration->inputModifier != ouzel::InputModifier::Out)
                throw TestError("Expected o to be out");
        }

        ++i;
        if (i == context.getDeclarations().end())
            throw TestError("Expected a function declaration");

        const auto vertexMainDeclaration = *i;

        if (vertexMainDeclaration->getDeclarationKind() != ouzel::Declaration::Kind::Callable)
            throw TestError("Expected a callable declaration");

        auto vertexMainCallableDeclaration = static_cast<const ouzel::CallableDeclaration*>(vertexMainDeclaration);

        if (vertexMainCallableDeclaration->name != "vertexMain")
            throw TestError("Expected a declaration of vertexMain");

        if (vertexMainCallableDeclaration->getCallableDeclarationKind() != ouzel::CallableDeclaration::Kind::Function)
            throw TestError("Expected a function declaration");

        auto vertexMainFunctionDeclaration = static_cast<const ouzel::FunctionDeclaration*>(vertexMainCallableDeclaration);

        {
            auto parameterIterator = vertexMainFunctionDeclaration->parameterDeclarations.begin();

            if (parameterIterator == vertexMainFunctionDeclaration->parameterDeclarations.end())
                throw TestError("Expected an i parameter");

            auto ioParameterDeclaration = *parameterIterator;
            ++parameterIterator;

            if (ioParameterDeclaration->name != "io")
                throw TestError("Expected an io parameter");

            if (ioParameterDeclaration->inputModifier != ouzel::InputModifier::Inout)
                throw TestError("Expected io to be in");
        }
    }

    void testOperators()
    {
        std::string code = R"OSL(
        function main():void
        {
            var a = 1.0f;
            var b:float = a * 1.0f;
            var v1:float4;
            var v2:float4 = v1;
            var v3:float4 = v1 + v2;
            var mat1:float4x4;
            var v4:float4 = v1 * mat1;
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));
    }

    void testExtern()
    {
        std::string code = R"OSL(
        extern color:float4;
        function main()
        {
        }
        )OSL";

        ouzel::ASTContext context(ouzel::tokenize(code));

        auto i = context.getDeclarations().begin();
        if (i == context.getDeclarations().end())
            throw TestError("Expected a variable declaration");

        const auto colorDeclaration = *i;

        if (colorDeclaration->getDeclarationKind() != ouzel::Declaration::Kind::Variable)
            throw TestError("Expected a variable declaration");

        auto colorVariableDeclaration = static_cast<const ouzel::VariableDeclaration*>(colorDeclaration);

        if (colorVariableDeclaration->name != "color")
            throw TestError("Expected a declaration of color");

        if (colorVariableDeclaration->storageClass != ouzel::StorageClass::Extern)
            throw TestError("Expected an extern storage class");
    }
}

int main()
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
    testRunner.run(testFunction);
    testRunner.run(testArray);
    testRunner.run(testFloat);
    testRunner.run(testVector);
    testRunner.run(testMatrix);
    testRunner.run(testSemantics);
    testRunner.run(testInputModifiers);
    testRunner.run(testOperators);
    testRunner.run(testExtern);

    if (testRunner.getResult())
        std::cout << "Success\n";

    return testRunner.getResult() ? EXIT_SUCCESS : EXIT_FAILURE;
}
