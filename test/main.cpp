//
//  OSL
//

#include <iostream>
#include "Parser.hpp"

void testDeclaration()
{
    std::string code = R"CODE(
    int main()
    {
        int i = 3;
    }
    )CODE";

    std::vector<Token> tokens = tokenize(code);
    ASTContext context(tokens);

    auto topDeclarations = context.getDeclarations();
    if (topDeclarations.size() != 1)
        throw std::runtime_error("Expected the main function");

    auto mainDeclaration = topDeclarations.front();

    if (mainDeclaration->name != "main" ||
        mainDeclaration->getDeclarationKind() != Declaration::Kind::Callable)
        throw std::runtime_error("Expected a callable declaration of main");

    auto mainCallableDeclaration = static_cast<const CallableDeclaration*>(mainDeclaration);

    auto mainBody = mainCallableDeclaration->body;

    if (mainBody->getStatementKind() != Statement::Kind::Compound)
        throw std::runtime_error("Expected a compound statement");

    auto mainCompoundStatement = static_cast<const CompoundStatement*>(mainBody);

    if (mainCompoundStatement->statements.size() != 1)
        throw std::runtime_error("Expected a declaration");

    auto iStatement = mainCompoundStatement->statements.front();

    if (iStatement->getStatementKind() != Statement::Kind::Declaration)
        throw std::runtime_error("Expected a declaration");

    auto iDeclarationStatement = static_cast<const DeclarationStatement*>(iStatement);
    auto iDeclaration = iDeclarationStatement->declaration;

    if (iDeclaration->name != "i" ||
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

int main(int argc, const char * argv[])
{
    try
    {
        testDeclaration();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Done\n";

    return EXIT_SUCCESS;
}
