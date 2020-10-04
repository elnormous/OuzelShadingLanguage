//
//  OSL
//

#include <type_traits>
#include "catch2/catch.hpp"
#include "Parser.hpp"

namespace
{
    const ouzel::CompoundStatement* getMainBody(const ouzel::Context& context)
    {
        auto i = context.getDeclarations().begin();
        assert(i != context.getDeclarations().end());

        auto declaration = *i;
        assert(declaration && declaration->name == "main" &&
               declaration->declarationKind == ouzel::Declaration::Kind::Callable);

        auto callableDeclaration = static_cast<const ouzel::CallableDeclaration*>(declaration);

        auto body = callableDeclaration->body;
        assert(body && body->statementKind == ouzel::Statement::Kind::Compound);

        return static_cast<const ouzel::CompoundStatement*>(body);
    }

    const ouzel::Expression& getMainExpression(const ouzel::Context& context)
    {
        auto mainCompoundStatement = getMainBody(context);

        auto i = mainCompoundStatement->statements.begin();
        assert(i != mainCompoundStatement->statements.end());

        const ouzel::Statement& statement = *i;
        assert(statement.statementKind == ouzel::Statement::Kind::Expression);

        auto& expressionStatement = static_cast<const ouzel::ExpressionStatement&>(statement);

        return expressionStatement.expression;
    }

    const ouzel::LiteralExpression* getLiteralExpression(const ouzel::Expression* expression)
    {
        assert(expression);
        assert(expression->category == ouzel::Expression::Category::Rvalue);
        assert((expression->qualifiedType.qualifiers & ouzel::Type::Qualifiers::Const) == ouzel::Type::Qualifiers::Const);
        assert(expression->expressionKind == ouzel::Expression::Kind::Literal);

        return static_cast<const ouzel::LiteralExpression*>(expression);
    }

    template <class T, typename std::enable_if<std::is_same<T, bool>::value>::type* = nullptr>
    void expectLiteral(const ouzel::Expression* expression, T value)
    {
        auto literalExpression = getLiteralExpression(expression);
        assert(literalExpression->literalKind == ouzel::LiteralExpression::Kind::Boolean);

        auto booleanLiteralExpression = static_cast<const ouzel::BooleanLiteralExpression*>(literalExpression);
        assert(booleanLiteralExpression->value == value);
    }

    template <class T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type* = nullptr>
    void expectLiteral(const ouzel::Expression* expression, T value)
    {
        auto literalExpression = getLiteralExpression(expression);
        assert(literalExpression->literalKind == ouzel::LiteralExpression::Kind::Integer);

        auto integerLiteralExpression = static_cast<const ouzel::IntegerLiteralExpression*>(literalExpression);
        assert(integerLiteralExpression->value == value);
    }

    template <class T, typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
    void expectLiteral(const ouzel::Expression* expression, T value)
    {
        auto literalExpression = getLiteralExpression(expression);
        assert(literalExpression->literalKind == ouzel::LiteralExpression::Kind::FloatingPoint);

        auto floatLiteralExpression = static_cast<const ouzel::FloatingPointLiteralExpression*>(literalExpression);
        assert(floatLiteralExpression->value == value);
    }
}

TEST_CASE("EmptyStatement", "[empty_statement]")
{
    std::string code = R"OSL(
    function main():void
    {
        ;
    }
    )OSL";

    ouzel::Context context(ouzel::tokenize(code));
    auto mainCompoundStatement = getMainBody(context);

    auto i = mainCompoundStatement->statements.begin();
    REQUIRE(i != mainCompoundStatement->statements.end());

    const ouzel::Statement& statement = *i;

    REQUIRE(statement.statementKind == ouzel::Statement::Kind::Empty);
}

TEST_CASE("Declaration", "[declaration]")
{
    std::string code = R"OSL(
    function main():void
    {
        var i:int = 3;
    }
    )OSL";

    ouzel::Context context(ouzel::tokenize(code));
    auto mainCompoundStatement = getMainBody(context);

    auto i = mainCompoundStatement->statements.begin();
    REQUIRE(i != mainCompoundStatement->statements.end());

    const ouzel::Statement& statement = *i;
    REQUIRE(statement.statementKind == ouzel::Statement::Kind::Declaration);

    auto& iDeclarationStatement = static_cast<const ouzel::DeclarationStatement&>(statement);
    auto& iDeclaration = iDeclarationStatement.declaration;
    REQUIRE(iDeclaration.name == "i");
    REQUIRE(iDeclaration.declarationKind == ouzel::Declaration::Kind::Variable);

    auto& iVariableDeclaration = static_cast<const ouzel::VariableDeclaration&>(iDeclaration);
    REQUIRE(iVariableDeclaration.qualifiedType.type.name == "int");

    expectLiteral(iVariableDeclaration.initialization, 3);
}

TEST_CASE("IfStatement", "[if_statement]")
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

    ouzel::Context context(ouzel::tokenize(code));
    auto mainCompoundStatement = getMainBody(context);

    auto i = mainCompoundStatement->statements.begin();
    REQUIRE(i != mainCompoundStatement->statements.end());

    const ouzel::Statement& statement = *i;
    REQUIRE(statement.statementKind == ouzel::Statement::Kind::If);

    auto& ifStatement = static_cast<const ouzel::IfStatement&>(statement);
    REQUIRE(ifStatement.condition.kind == ouzel::Construct::Kind::Expression);

    auto& condition = static_cast<const ouzel::Expression&>(ifStatement.condition);
    expectLiteral(&condition, true);
    REQUIRE(ifStatement.body.statementKind == ouzel::Statement::Kind::Compound);

    REQUIRE(ifStatement.elseBody);
    REQUIRE(ifStatement.elseBody->statementKind == ouzel::Statement::Kind::If);

    auto elseIfStatement = static_cast<const ouzel::IfStatement*>(ifStatement.elseBody);

    REQUIRE(elseIfStatement->condition.kind == ouzel::Construct::Kind::Expression);

    auto& elseIfCondition = static_cast<const ouzel::Expression&>(elseIfStatement->condition);

    expectLiteral(&elseIfCondition, false);

    REQUIRE(elseIfStatement->elseBody);
    REQUIRE(elseIfStatement->elseBody->statementKind == ouzel::Statement::Kind::Compound);
}

TEST_CASE("WhileStatement", "[while_statement]")
{
    std::string code = R"OSL(
    function main():void
    {
        while (true)
        {
        }
    }
    )OSL";

    ouzel::Context context(ouzel::tokenize(code));
    auto mainCompoundStatement = getMainBody(context);

    auto i = mainCompoundStatement->statements.begin();
    REQUIRE(i != mainCompoundStatement->statements.end());

    const ouzel::Statement& statement = *i;
    REQUIRE(statement.statementKind == ouzel::Statement::Kind::While);

    auto& whileStatement = static_cast<const ouzel::WhileStatement&>(statement);
    REQUIRE(whileStatement.condition.kind == ouzel::Construct::Kind::Expression);

    auto& condition = static_cast<const ouzel::Expression&>(whileStatement.condition);
    expectLiteral(&condition, true);
    REQUIRE(whileStatement.body.statementKind == ouzel::Statement::Kind::Compound);
}

TEST_CASE("DoStatement", "[do_statement]")
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

    ouzel::Context context(ouzel::tokenize(code));
    auto mainCompoundStatement = getMainBody(context);

    auto i = mainCompoundStatement->statements.begin();
    REQUIRE(i != mainCompoundStatement->statements.end());

    const ouzel::Statement& statement = *i;
    REQUIRE(statement.statementKind == ouzel::Statement::Kind::Do);

    auto& doStatement = static_cast<const ouzel::DoStatement&>(statement);
    REQUIRE(doStatement.condition.kind == ouzel::Construct::Kind::Expression);

    auto& condition = static_cast<const ouzel::Expression&>(doStatement.condition);
    expectLiteral(&condition, true);
    REQUIRE(doStatement.body.statementKind == ouzel::Statement::Kind::Compound);
}

TEST_CASE("ForStatement", "[for_statement]")
{
    std::string code = R"OSL(
    function main():void
    {
        for (; true;)
        {
        }
    }
    )OSL";

    ouzel::Context context(ouzel::tokenize(code));
    auto mainCompoundStatement = getMainBody(context);

    auto i = mainCompoundStatement->statements.begin();
    REQUIRE(i != mainCompoundStatement->statements.end());

    const ouzel::Statement& statement = *i;
    REQUIRE(statement.statementKind == ouzel::Statement::Kind::For);

    auto& forStatement = static_cast<const ouzel::ForStatement&>(statement);
    REQUIRE(forStatement.condition);
    REQUIRE(forStatement.condition->kind == ouzel::Construct::Kind::Expression);

    auto condition = static_cast<const ouzel::Expression*>(forStatement.condition);
    expectLiteral(condition, true);
    REQUIRE(forStatement.body.statementKind == ouzel::Statement::Kind::Compound);
}

TEST_CASE("SwitchStatement", "[switch_statement]")
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

    ouzel::Context context(ouzel::tokenize(code));
    auto mainCompoundStatement = getMainBody(context);

    auto i = mainCompoundStatement->statements.begin();
    REQUIRE(i != mainCompoundStatement->statements.end());

    const ouzel::Statement& statement = *i;
    REQUIRE(statement.statementKind == ouzel::Statement::Kind::Switch);

    auto& switchStatement = static_cast<const ouzel::SwitchStatement&>(statement);
    REQUIRE(switchStatement.condition.kind == ouzel::Construct::Kind::Expression);

    auto& condition = static_cast<const ouzel::Expression&>(switchStatement.condition);
    expectLiteral(&condition, 1);
    REQUIRE(switchStatement.body.statementKind == ouzel::Statement::Kind::Compound);

    auto& body = static_cast<const ouzel::CompoundStatement&>(switchStatement.body);

    const auto firstStatementIterator = body.statements.begin();
    REQUIRE(firstStatementIterator != body.statements.end());

    const ouzel::Statement& firstStatement = *firstStatementIterator;
    REQUIRE(firstStatement.statementKind == ouzel::Statement::Kind::Case);

    auto& firstCaseStatement = static_cast<const ouzel::CaseStatement&>(firstStatement);
    expectLiteral(&firstCaseStatement.condition, 1);
    REQUIRE(firstCaseStatement.body.statementKind == ouzel::Statement::Kind::Empty);

    auto secondStatementIterator = firstStatementIterator + 1;
    REQUIRE(secondStatementIterator != body.statements.end());

    const ouzel::Statement& secondStatement = *secondStatementIterator;
    REQUIRE(secondStatement.statementKind == ouzel::Statement::Kind::Case);

    auto& secondCaseStatement = static_cast<const ouzel::CaseStatement&>(secondStatement);
    expectLiteral(&secondCaseStatement.condition, 2);
    REQUIRE(secondCaseStatement.body.statementKind == ouzel::Statement::Kind::Break);

    auto thirdStatementIterator = secondStatementIterator + 1;
    REQUIRE(thirdStatementIterator != body.statements.end());

    const ouzel::Statement& thirdStatement = *thirdStatementIterator;
    REQUIRE(thirdStatement.statementKind == ouzel::Statement::Kind::Default);

    auto& defaultStatement = static_cast<const ouzel::DefaultStatement&>(thirdStatement);
    REQUIRE(defaultStatement.body.statementKind == ouzel::Statement::Kind::Empty);
}

TEST_CASE("ReturnStatement", "[return_statement]")
{
    std::string code = R"OSL(
    function main():int
    {
        return 1;
    }
    )OSL";

    ouzel::Context context(ouzel::tokenize(code));
    auto mainCompoundStatement = getMainBody(context);

    auto i = mainCompoundStatement->statements.begin();
    REQUIRE(i != mainCompoundStatement->statements.end());

    const ouzel::Statement& statement = *i;
    REQUIRE(statement.statementKind == ouzel::Statement::Kind::Return);

    auto& returnStatement = static_cast<const ouzel::ReturnStatement&>(statement);
    expectLiteral(returnStatement.result, 1);
}

TEST_CASE("BoolLiteral", "[bool_literal]")
{
    std::string code = R"OSL(
    function main():void
    {
        true;
        return;
    }
    )OSL";

    ouzel::Context context(ouzel::tokenize(code));
    expectLiteral(&getMainExpression(context), true);
}

TEST_CASE("IntLiteral", "[int_literal]")
{
    std::string code = R"OSL(
    function main():void
    {
        1;
    }
    )OSL";

    ouzel::Context context(ouzel::tokenize(code));
    expectLiteral(&getMainExpression(context), 1);
}

TEST_CASE("FloatLiteral", "[float_literal]")
{
    std::string code = R"OSL(
    function main():void
    {
        1.0F;
    }
    )OSL";

    ouzel::Context context(ouzel::tokenize(code));
    expectLiteral(&getMainExpression(context), 1.0F);
}

TEST_CASE("StructDeclaration", "[struct_declaration]")
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

    ouzel::Context context(ouzel::tokenize(code));

    auto i = context.getDeclarations().begin();
    REQUIRE(i != context.getDeclarations().end());

    const auto structDefinition = *i;
    REQUIRE(structDefinition->declarationKind == ouzel::Declaration::Kind::Type);

    auto structTypeDefinition = static_cast<const ouzel::TypeDeclaration*>(structDefinition);
    REQUIRE(structTypeDefinition->name == "Foo");

    auto& structDefinitionType = structTypeDefinition->type;
    REQUIRE(structTypeDefinition->name == structDefinitionType.name);
}

TEST_CASE("Function", "[function]")
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

    ouzel::Context context(ouzel::tokenize(code));
}

TEST_CASE("Array", "[array]")
{
    std::string code = R"OSL(
    function main():void
    {
        var a:float[4];
        a[0] = 1.0f;
    }
    )OSL";

    ouzel::Context context(ouzel::tokenize(code));
}

TEST_CASE("Float", "[float]")
{
    std::string code = R"OSL(
    function main():void
    {
        var f1:float = 1.0f;
        f1 = 1.0e-3F;
        f1 = 1.0e3f;
        var f2:float = ++f1;
        var f3:float = f2++;
        var f4:float = float(1);
    }
    )OSL";

    ouzel::Context context(ouzel::tokenize(code));
}

TEST_CASE("Vector", "[vector]")
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

    ouzel::Context context(ouzel::tokenize(code));
}

TEST_CASE("Matrix", "[matrix]")
{
    std::string code = R"OSL(
    function main():float4x4;
    function main():float4x4
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

    ouzel::Context context(ouzel::tokenize(code));
}

TEST_CASE("Semantics", "[semantics]")
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

    ouzel::Context context(ouzel::tokenize(code));
}

TEST_CASE("InputModifiers", "[input_modifiers]")
{
    std::string code = R"OSL(
    fragment fragmentMain(in i:float4, out o:float4):float
    {
        return 0.0f;
    }
    vertex vertexMain(inout io:float4):float
    {
        return 0.0f;
    }
    )OSL";

    ouzel::Context context(ouzel::tokenize(code));

    auto i = context.getDeclarations().begin();
    REQUIRE(i != context.getDeclarations().end());

    const auto fragmentMainDeclaration = *i;
    REQUIRE(fragmentMainDeclaration->declarationKind == ouzel::Declaration::Kind::Callable);

    auto fragmentMainCallableDeclaration = static_cast<const ouzel::CallableDeclaration*>(fragmentMainDeclaration);
    REQUIRE(fragmentMainCallableDeclaration->name == "fragmentMain");
    REQUIRE(fragmentMainCallableDeclaration->callableDeclarationKind == ouzel::CallableDeclaration::Kind::Function);

    auto fragmentMainFunctionDeclaration = static_cast<const ouzel::FunctionDeclaration*>(fragmentMainCallableDeclaration);

    {
        auto parameterIterator = fragmentMainFunctionDeclaration->parameterDeclarations.begin();
        REQUIRE(parameterIterator != fragmentMainFunctionDeclaration->parameterDeclarations.end());

        auto iParameterDeclaration = *parameterIterator;
        ++parameterIterator;

        REQUIRE(iParameterDeclaration->name == "i");
        REQUIRE(iParameterDeclaration->inputModifier == ouzel::InputModifier::In);
        REQUIRE(parameterIterator != fragmentMainFunctionDeclaration->parameterDeclarations.end());

        auto oParameterDeclaration = *parameterIterator;
        REQUIRE(oParameterDeclaration->name == "o");
        REQUIRE(oParameterDeclaration->inputModifier == ouzel::InputModifier::Out);
    }

    ++i;
    REQUIRE(i != context.getDeclarations().end());

    const auto vertexMainDeclaration = *i;
    REQUIRE(vertexMainDeclaration->declarationKind == ouzel::Declaration::Kind::Callable);

    auto vertexMainCallableDeclaration = static_cast<const ouzel::CallableDeclaration*>(vertexMainDeclaration);
    REQUIRE(vertexMainCallableDeclaration->name == "vertexMain");
    REQUIRE(vertexMainCallableDeclaration->callableDeclarationKind == ouzel::CallableDeclaration::Kind::Function);

    auto vertexMainFunctionDeclaration = static_cast<const ouzel::FunctionDeclaration*>(vertexMainCallableDeclaration);

    {
        auto parameterIterator = vertexMainFunctionDeclaration->parameterDeclarations.begin();
        REQUIRE(parameterIterator != vertexMainFunctionDeclaration->parameterDeclarations.end());

        auto ioParameterDeclaration = *parameterIterator;
        ++parameterIterator;

        REQUIRE(ioParameterDeclaration->name == "io");
        REQUIRE(ioParameterDeclaration->inputModifier == ouzel::InputModifier::Inout);
    }
}

TEST_CASE("Operators", "[operators]")
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

    ouzel::Context context(ouzel::tokenize(code));
}

TEST_CASE("Extern", "[extern]")
{
    std::string code = R"OSL(
    extern color:float4;
    function main()
    {
    }
    )OSL";

    ouzel::Context context(ouzel::tokenize(code));

    auto i = context.getDeclarations().begin();
    REQUIRE(i != context.getDeclarations().end());

    const auto colorDeclaration = *i;
    REQUIRE(colorDeclaration->declarationKind == ouzel::Declaration::Kind::Variable);

    auto colorVariableDeclaration = static_cast<const ouzel::VariableDeclaration*>(colorDeclaration);

    REQUIRE(colorVariableDeclaration->name == "color");
    REQUIRE(colorVariableDeclaration->storageClass == ouzel::StorageClass::Extern);
}
