//
//  OSL
//

#include <iostream>
#include <stdexcept>
#include "Parser.hpp"

static const CompoundStatement* getMainBody(const Declaration* declaration)
{
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

static void testEmptyStatement()
{
    std::string code = R"OSL(
    void main()
    {
        ;
    }
    )OSL";

    std::vector<Token> tokens = tokenize(code);
    ASTContext context(tokens);

    auto topDeclarations = context.getDeclarations();
    if (topDeclarations.size() != 1)
        throw std::runtime_error("Expected the main function");

    auto mainCompoundStatement = getMainBody(topDeclarations.front());

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

    std::vector<Token> tokens = tokenize(code);
    ASTContext context(tokens);

    auto topDeclarations = context.getDeclarations();
    if (topDeclarations.size() != 1)
        throw std::runtime_error("Expected the main function");

    auto mainCompoundStatement = getMainBody(topDeclarations.front());

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

    if (!iVariableDeclaration->initialization ||
        iVariableDeclaration->initialization->getExpressionKind() != Expression::Kind::Literal)
        throw std::runtime_error("Expected an initialization with a literal");

    auto literalExpression = static_cast<const LiteralExpression*>(iVariableDeclaration->initialization);

    if (literalExpression->getLiteralKind() != LiteralExpression::Kind::Integer)
        throw std::runtime_error("Expected an integer literal");

    auto integerLiteralExpression = static_cast<const IntegerLiteralExpression*>(literalExpression);

    if (integerLiteralExpression->value != 3)
        throw std::runtime_error("Expected a literal 3");
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

    std::vector<Token> tokens = tokenize(code);
    ASTContext context(tokens);

    auto topDeclarations = context.getDeclarations();
    if (topDeclarations.size() != 1)
        throw std::runtime_error("Expected the main function");

    auto mainCompoundStatement = getMainBody(topDeclarations.front());

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

    if (condition->getExpressionKind() != Expression::Kind::Literal)
        throw std::runtime_error("Expected a literal condition");

    auto literalCondition = static_cast<const LiteralExpression*>(condition);

    if (literalCondition->getLiteralKind() != LiteralExpression::Kind::Boolean)
        throw std::runtime_error("Expected a bool literal condition");

    auto boolLiteralCondition = static_cast<const BooleanLiteralExpression*>(literalCondition);

    if (boolLiteralCondition->value != true)
        throw std::runtime_error("Expected a literal with a value \"true\"");

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

    std::vector<Token> tokens = tokenize(code);
    ASTContext context(tokens);

    auto topDeclarations = context.getDeclarations();
    if (topDeclarations.size() != 1)
        throw std::runtime_error("Expected the main function");

    auto mainCompoundStatement = getMainBody(topDeclarations.front());

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

    if (condition->getExpressionKind() != Expression::Kind::Literal)
        throw std::runtime_error("Expected a literal condition");

    auto literalCondition = static_cast<const LiteralExpression*>(condition);

    if (literalCondition->getLiteralKind() != LiteralExpression::Kind::Boolean)
        throw std::runtime_error("Expected a bool literal condition");

    auto boolLiteralCondition = static_cast<const BooleanLiteralExpression*>(literalCondition);

    if (boolLiteralCondition->value != true)
        throw std::runtime_error("Expected a literal with a value \"true\"");

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

    std::vector<Token> tokens = tokenize(code);
    ASTContext context(tokens);

    auto topDeclarations = context.getDeclarations();
    if (topDeclarations.size() != 1)
        throw std::runtime_error("Expected the main function");

    auto mainCompoundStatement = getMainBody(topDeclarations.front());

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

    if (condition->getExpressionKind() != Expression::Kind::Literal)
        throw std::runtime_error("Expected a literal condition");

    auto literalCondition = static_cast<const LiteralExpression*>(condition);

    if (literalCondition->getLiteralKind() != LiteralExpression::Kind::Boolean)
        throw std::runtime_error("Expected a bool literal condition");

    auto boolLiteralCondition = static_cast<const BooleanLiteralExpression*>(literalCondition);

    if (boolLiteralCondition->value != true)
        throw std::runtime_error("Expected a literal with a value \"true\"");

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

    std::vector<Token> tokens = tokenize(code);
    ASTContext context(tokens);

    auto topDeclarations = context.getDeclarations();
    if (topDeclarations.size() != 1)
        throw std::runtime_error("Expected the main function");

    auto mainCompoundStatement = getMainBody(topDeclarations.front());

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

    if (condition->getExpressionKind() != Expression::Kind::Literal)
        throw std::runtime_error("Expected a literal condition");

    auto literalCondition = static_cast<const LiteralExpression*>(condition);

    if (literalCondition->getLiteralKind() != LiteralExpression::Kind::Boolean)
        throw std::runtime_error("Expected a bool literal condition");

    auto boolLiteralCondition = static_cast<const BooleanLiteralExpression*>(literalCondition);

    if (boolLiteralCondition->value != true)
        throw std::runtime_error("Expected a literal with a value \"true\"");

    if (!forStatement->body ||
        forStatement->body->getStatementKind() != Statement::Kind::Compound)
        throw std::runtime_error("Expected a compound statement");
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
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Done\n";

    return EXIT_SUCCESS;
}
