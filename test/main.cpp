//
//  OSL
//

#include <iostream>
#include <stdexcept>
#include <type_traits>
#include "Parser.hpp"

static const CompoundStatement* getMainBody(const ASTContext& context)
{
    auto topDeclarations = context.getDeclarations();
    if (topDeclarations.size() != 1)
        throw std::runtime_error("Expected the main function");

    auto declaration = static_cast<const Declaration*>(topDeclarations.front());

    if (!declaration ||
        declaration->name != "main" ||
        declaration->getDeclarationKind() != Declaration::Kind::Callable)
        throw std::runtime_error("Expected a callable declaration of main");

    auto callableDeclaration = static_cast<const CallableDeclaration*>(declaration);

    auto body = callableDeclaration->body;

    if (!body ||
        body->getStatementKind() != Statement::Kind::Compound)
        throw std::runtime_error("Expected a compound statement");

    return static_cast<const CompoundStatement*>(body);
}

template <class T, typename std::enable_if<std::is_same<T, bool>::value>::type* = nullptr>
static void expectLiteral(const Expression* expression, T value)
{
    if (!expression)
        throw std::runtime_error("Expected an expression");

    if (expression->category != Expression::Category::Rvalue)
        throw std::runtime_error("Expected must rvalue");

    if (!expression->qualifiedType.isConst)
        throw std::runtime_error("Expected must be const");

    if (expression->getExpressionKind() != Expression::Kind::Literal)
        throw std::runtime_error("Expected a literal expression");

    auto literalExpression = static_cast<const LiteralExpression*>(expression);

    if (literalExpression->getLiteralKind() != LiteralExpression::Kind::Boolean)
        throw std::runtime_error("Expected a boolean literal expression");

    auto booleanLiteralExpression = static_cast<const BooleanLiteralExpression*>(literalExpression);

    if (booleanLiteralExpression->value != value)
        throw std::runtime_error("Wrong literal value");
}

template <class T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type* = nullptr>
static void expectLiteral(const Expression* expression, T value)
{
    if (!expression)
        throw std::runtime_error("Expected an expression");

    if (expression->getExpressionKind() != Expression::Kind::Literal)
        throw std::runtime_error("Expected a literal expression");

    auto literalExpression = static_cast<const LiteralExpression*>(expression);

    if (literalExpression->getLiteralKind() != LiteralExpression::Kind::Integer)
        throw std::runtime_error("Expected an integer literal expression");

    auto integerLiteralExpression = static_cast<const IntegerLiteralExpression*>(literalExpression);

    if (integerLiteralExpression->value != value)
        throw std::runtime_error("Wrong literal value");
}

static void testEmptyStatement()
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
        throw std::runtime_error("Expected a statement");

    auto statement = mainCompoundStatement->statements.front();

    if (!statement ||
        statement->getStatementKind() != Statement::Kind::Empty)
        throw std::runtime_error("Expected an empty statement");
}

static void testDeclaration()
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
        throw std::runtime_error("Expected a statement");

    auto statement = mainCompoundStatement->statements.front();

    if (!statement ||
        statement->getStatementKind() != Statement::Kind::Declaration)
        throw std::runtime_error("Expected a declaration");

    auto iDeclarationStatement = static_cast<const DeclarationStatement*>(statement);
    auto iDeclaration = iDeclarationStatement->declaration;

    if (!iDeclaration ||
        iDeclaration->name != "i" ||
        iDeclaration->getDeclarationKind() != Declaration::Kind::Variable)
        throw std::runtime_error("Expected a variable declaration of i");

    auto iVariableDeclaration = static_cast<const VariableDeclaration*>(iDeclaration);

    if (!iVariableDeclaration->qualifiedType.typeDeclaration ||
        !iVariableDeclaration->qualifiedType.typeDeclaration->isBuiltin ||
        iVariableDeclaration->qualifiedType.typeDeclaration->name != "int")
        throw std::runtime_error("Expected a declaration of a variable of type int");

    expectLiteral(iVariableDeclaration->initialization, 3);
}

static void testIfStatement()
{
    std::string code = R"OSL(
    void main()
    {
        if (true)
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
        throw std::runtime_error("Expected a statement");

    auto statement = mainCompoundStatement->statements.front();

    if (!statement ||
        statement->getStatementKind() != Statement::Kind::If)
        throw std::runtime_error("Expected an if statement");

    auto ifStatement = static_cast<const IfStatement*>(statement);

    if (!ifStatement->condition ||
        ifStatement->condition->getKind() != Construct::Kind::Expression)
        throw std::runtime_error("Expected an expression condition");

    auto condition = static_cast<const Expression*>(ifStatement->condition);

    expectLiteral(condition, true);

    if (!ifStatement->body ||
        ifStatement->body->getStatementKind() != Statement::Kind::Compound)
        throw std::runtime_error("Expected a compound statement if part");

    if (ifStatement->elseBody->getStatementKind() != Statement::Kind::Compound)
        throw std::runtime_error("Expected a compound statement else part");
}

static void testWhileStatement()
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
        throw std::runtime_error("Expected a statement");

    auto statement = mainCompoundStatement->statements.front();

    if (!statement ||
        statement->getStatementKind() != Statement::Kind::While)
        throw std::runtime_error("Expected a while statement");

    auto whileStatement = static_cast<const WhileStatement*>(statement);

    if (!whileStatement->condition ||
        whileStatement->condition->getKind() != Construct::Kind::Expression)
        throw std::runtime_error("Expected an expression condition");

    auto condition = static_cast<const Expression*>(whileStatement->condition);

    expectLiteral(condition, true);

    if (!whileStatement->body ||
        whileStatement->body->getStatementKind() != Statement::Kind::Compound)
        throw std::runtime_error("Expected a compound statement");
}

static void testDoStatement()
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
        throw std::runtime_error("Expected a statement");

    auto statement = mainCompoundStatement->statements.front();

    if (!statement ||
        statement->getStatementKind() != Statement::Kind::Do)
        throw std::runtime_error("Expected a do statement");

    auto doStatement = static_cast<const DoStatement*>(statement);

    if (!doStatement->condition ||
        doStatement->condition->getKind() != Construct::Kind::Expression)
        throw std::runtime_error("Expected an expression condition");

    auto condition = static_cast<const Expression*>(doStatement->condition);

    expectLiteral(condition, true);

    if (!doStatement->body ||
        doStatement->body->getStatementKind() != Statement::Kind::Compound)
        throw std::runtime_error("Expected a compound statement");
}

static void testForStatement()
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
        throw std::runtime_error("Expected a statement");

    auto statement = mainCompoundStatement->statements.front();

    if (!statement ||
        statement->getStatementKind() != Statement::Kind::For)
        throw std::runtime_error("Expected a for statement");

    auto forStatement = static_cast<const ForStatement*>(statement);

    if (!forStatement->condition ||
        forStatement->condition->getKind() != Construct::Kind::Expression)
        throw std::runtime_error("Expected an expression condition");

    auto condition = static_cast<const Expression*>(forStatement->condition);

    expectLiteral(condition, true);

    if (!forStatement->body ||
        forStatement->body->getStatementKind() != Statement::Kind::Compound)
        throw std::runtime_error("Expected a compound statement");
}

static void testSwitchStatement()
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
        throw std::runtime_error("Expected a statement");

    auto statement = mainCompoundStatement->statements.front();

    if (!statement ||
        statement->getStatementKind() != Statement::Kind::Switch)
        throw std::runtime_error("Expected a switch statement");

    auto switchStatement = static_cast<const SwitchStatement*>(statement);

    if (!switchStatement->condition ||
        switchStatement->condition->getKind() != Construct::Kind::Expression)
        throw std::runtime_error("Expected an expression condition");

    auto condition = static_cast<const Expression*>(switchStatement->condition);

    expectLiteral(condition, 1);

    if (!switchStatement->body ||
        switchStatement->body->getStatementKind() != Statement::Kind::Compound)
        throw std::runtime_error("Expected a compound statement");

    auto body = static_cast<const CompoundStatement*>(switchStatement->body);

    if (body->statements.size() != 3)
        throw std::runtime_error("Expected 3 statements");

    if (body->statements[0]->getStatementKind() != Statement::Kind::Case)
        throw std::runtime_error("Expected a case statement");

    auto firstCaseStatement = static_cast<const CaseStatement*>(body->statements[0]);

    expectLiteral(firstCaseStatement->condition, 1);

    if (!firstCaseStatement->body ||
        firstCaseStatement->body->getStatementKind() != Statement::Kind::Empty)
        throw std::runtime_error("Expected an empty statement");

    if (body->statements[1]->getStatementKind() != Statement::Kind::Case)
        throw std::runtime_error("Expected a case statement");

    auto secondCaseStatement = static_cast<const CaseStatement*>(body->statements[1]);

    expectLiteral(secondCaseStatement->condition, 2);

    if (!secondCaseStatement->body ||
        secondCaseStatement->body->getStatementKind() != Statement::Kind::Break)
        throw std::runtime_error("Expected an break statement");

    if (body->statements[2]->getStatementKind() != Statement::Kind::Default)
        throw std::runtime_error("Expected a default statement");

    auto defaultStatement = static_cast<const DefaultStatement*>(body->statements[2]);

    if (!defaultStatement->body ||
        defaultStatement->body->getStatementKind() != Statement::Kind::Empty)
        throw std::runtime_error("Expected an empty statement");
}

static void testReturnStatement()
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
        throw std::runtime_error("Expected a statement");

    auto statement = mainCompoundStatement->statements.front();

    if (!statement ||
        statement->getStatementKind() != Statement::Kind::Return)
        throw std::runtime_error("Expected a return statement");

    auto returnStatement = static_cast<const ReturnStatement*>(statement);

    expectLiteral(returnStatement->result, 1);
}
int main(int argc, const char * argv[])
{
    try
    {
        testEmptyStatement();
        testDeclaration();
        testIfStatement();
        testWhileStatement();
        testDoStatement();
        testForStatement();
        testSwitchStatement();
        testReturnStatement();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Done\n";

    return EXIT_SUCCESS;
}
