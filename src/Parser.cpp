//
//  OSL
//

#include <algorithm>
#include <iostream>
#include "Parser.hpp"

ASTContext::ASTContext()
{
}

ASTContext::ASTContext(const std::vector<Token>& tokens)
{
    std::vector<std::vector<Declaration*>> declarationScopes;
    declarationScopes.push_back(std::vector<Declaration*>());

    boolTypeDeclaration = addScalarTypeDeclaration("bool", ScalarTypeDeclaration::Kind::Boolean, 1, false, declarationScopes);
    addOperatorDeclaration(Operator::Negation, boolTypeDeclaration, {boolTypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::Or, boolTypeDeclaration, {boolTypeDeclaration, boolTypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::And, boolTypeDeclaration, {boolTypeDeclaration, boolTypeDeclaration}, declarationScopes);

    intTypeDeclaration = addScalarTypeDeclaration("int", ScalarTypeDeclaration::Kind::Integer, 4, false, declarationScopes);
    unsignedIntTypeDeclaration = addScalarTypeDeclaration("unsigned int", ScalarTypeDeclaration::Kind::Integer, 4, true, declarationScopes);
    floatTypeDeclaration = addScalarTypeDeclaration("float", ScalarTypeDeclaration::Kind::FloatingPoint, 4, false, declarationScopes);

    for (ScalarTypeDeclaration* scalarTypeDeclaration : {intTypeDeclaration, unsignedIntTypeDeclaration, floatTypeDeclaration})
    {
        // binary operators
        for (ScalarTypeDeclaration* secondScalarTypeDeclaration : {intTypeDeclaration, unsignedIntTypeDeclaration, floatTypeDeclaration})
        {
            addOperatorDeclaration(Operator::Assignment, scalarTypeDeclaration, {scalarTypeDeclaration, secondScalarTypeDeclaration}, declarationScopes);

            addOperatorDeclaration(Operator::Addition, scalarTypeDeclaration, {scalarTypeDeclaration, secondScalarTypeDeclaration}, declarationScopes);
            addOperatorDeclaration(Operator::AdditionAssignment, scalarTypeDeclaration, {scalarTypeDeclaration, secondScalarTypeDeclaration}, declarationScopes);

            addOperatorDeclaration(Operator::Subtraction, scalarTypeDeclaration, {scalarTypeDeclaration, secondScalarTypeDeclaration}, declarationScopes);
            addOperatorDeclaration(Operator::SubtractAssignment, scalarTypeDeclaration, {scalarTypeDeclaration, secondScalarTypeDeclaration}, declarationScopes);

            addOperatorDeclaration(Operator::Multiplication, scalarTypeDeclaration, {scalarTypeDeclaration, secondScalarTypeDeclaration}, declarationScopes);
            addOperatorDeclaration(Operator::MultiplicationAssignment, scalarTypeDeclaration, {scalarTypeDeclaration, secondScalarTypeDeclaration}, declarationScopes);

            addOperatorDeclaration(Operator::Division, scalarTypeDeclaration, {scalarTypeDeclaration, secondScalarTypeDeclaration}, declarationScopes);
            addOperatorDeclaration(Operator::DivisionAssignment, scalarTypeDeclaration, {scalarTypeDeclaration, secondScalarTypeDeclaration}, declarationScopes);

            addOperatorDeclaration(Operator::LessThan, scalarTypeDeclaration, {scalarTypeDeclaration, secondScalarTypeDeclaration}, declarationScopes);
            addOperatorDeclaration(Operator::LessThanEqual, scalarTypeDeclaration, {scalarTypeDeclaration, secondScalarTypeDeclaration}, declarationScopes);

            addOperatorDeclaration(Operator::GreaterThan, scalarTypeDeclaration, {scalarTypeDeclaration, secondScalarTypeDeclaration}, declarationScopes);
            addOperatorDeclaration(Operator::GraterThanEqual, scalarTypeDeclaration, {scalarTypeDeclaration, secondScalarTypeDeclaration}, declarationScopes);
        }

        // unary operators
        addOperatorDeclaration(Operator::Positive, scalarTypeDeclaration, {scalarTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::Negative, scalarTypeDeclaration, {scalarTypeDeclaration}, declarationScopes);
    }

    StructDeclaration* float2TypeDeclaration = addStructDeclaration("float2", 8, declarationScopes);
    StructDeclaration* float3TypeDeclaration = addStructDeclaration("float3", 12, declarationScopes);
    StructDeclaration* float4TypeDeclaration = addStructDeclaration("float4", 16, declarationScopes);

    for (StructDeclaration* vectorTypeDeclaration : {float2TypeDeclaration, float3TypeDeclaration, float4TypeDeclaration})
    {
        addOperatorDeclaration(Operator::Assignment, vectorTypeDeclaration, {vectorTypeDeclaration, vectorTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::Addition, vectorTypeDeclaration, {vectorTypeDeclaration, vectorTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::AdditionAssignment, vectorTypeDeclaration, {vectorTypeDeclaration, vectorTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::Subtraction, vectorTypeDeclaration, {vectorTypeDeclaration, vectorTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::SubtractAssignment, vectorTypeDeclaration, {vectorTypeDeclaration, vectorTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::Multiplication, vectorTypeDeclaration, {vectorTypeDeclaration, vectorTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::MultiplicationAssignment, vectorTypeDeclaration, {vectorTypeDeclaration, vectorTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::Division, vectorTypeDeclaration, {vectorTypeDeclaration, vectorTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::DivisionAssignment, vectorTypeDeclaration, {vectorTypeDeclaration, vectorTypeDeclaration}, declarationScopes);

        addOperatorDeclaration(Operator::Positive, vectorTypeDeclaration, {vectorTypeDeclaration}, declarationScopes);
        addOperatorDeclaration(Operator::Negative, vectorTypeDeclaration, {vectorTypeDeclaration}, declarationScopes);
    }

    std::vector<std::pair<StructDeclaration*, std::vector<TypeDeclaration*>>> constructors = {
        {float2TypeDeclaration, {floatTypeDeclaration, floatTypeDeclaration}},
        {float2TypeDeclaration, {float2TypeDeclaration}},

        {float3TypeDeclaration, {floatTypeDeclaration, floatTypeDeclaration, floatTypeDeclaration}},
        {float3TypeDeclaration, {floatTypeDeclaration, float2TypeDeclaration}},
        {float3TypeDeclaration, {float2TypeDeclaration, floatTypeDeclaration}},
        {float3TypeDeclaration, {float3TypeDeclaration}},

        {float4TypeDeclaration, {floatTypeDeclaration, floatTypeDeclaration, floatTypeDeclaration, floatTypeDeclaration}},
        {float4TypeDeclaration, {floatTypeDeclaration, floatTypeDeclaration, float2TypeDeclaration}},
        {float4TypeDeclaration, {floatTypeDeclaration, float2TypeDeclaration, floatTypeDeclaration}},
        {float4TypeDeclaration, {floatTypeDeclaration, float3TypeDeclaration}},
        {float4TypeDeclaration, {float2TypeDeclaration, floatTypeDeclaration, floatTypeDeclaration}},
        {float4TypeDeclaration, {float2TypeDeclaration, float2TypeDeclaration}},
        {float4TypeDeclaration, {float3TypeDeclaration, floatTypeDeclaration}},
        {float4TypeDeclaration, {float4TypeDeclaration}}
    };

    for (auto& constructor : constructors)
    {
        ConstructorDeclaration* constructorDeclaration;
        constructs.push_back(std::unique_ptr<Construct>(constructorDeclaration = new ConstructorDeclaration()));

        constructorDeclaration->parent = constructor.first;
        constructorDeclaration->definition = constructorDeclaration;

        for (auto& parameter : constructor.second)
        {
            ParameterDeclaration* parameterDeclaration;
            constructs.push_back(std::unique_ptr<Construct>(parameterDeclaration = new ParameterDeclaration()));

            parameterDeclaration->parent = constructorDeclaration;
            parameterDeclaration->qualifiedType.typeDeclaration = parameter;
            constructorDeclaration->parameterDeclarations.push_back(parameterDeclaration);
        }

        constructor.first->memberDeclarations.push_back(constructorDeclaration);
    }

    std::vector<std::pair<StructDeclaration*, std::vector<char>>> types = {
        {float2TypeDeclaration, {'x', 'y'}},
        {float2TypeDeclaration, {'r', 'g'}},
        {float3TypeDeclaration, {'x', 'y', 'z'}},
        {float3TypeDeclaration, {'r', 'g', 'b'}},
        {float4TypeDeclaration, {'x', 'y', 'z', 'w'}},
        {float4TypeDeclaration, {'r', 'g', 'b', 'a'}}
    };

    for (auto& type : types)
    {
        declarationScopes.push_back(std::vector<Declaration*>());

        for (char first : type.second)
        {
            addFieldDeclaration(type.first, {first}, floatTypeDeclaration, false, declarationScopes);

            for (char second : type.second)
            {
                bool secondConst = (second == first);

                addFieldDeclaration(type.first, {first, second}, float2TypeDeclaration, secondConst, declarationScopes);

                for (char third : type.second)
                {
                    bool thirdConst = (secondConst || third == first || third == second);

                    addFieldDeclaration(type.first, {first, second, third}, float3TypeDeclaration, thirdConst, declarationScopes);

                    for (char fourth : type.second)
                    {
                        bool fourthConst = (thirdConst || fourth == first || fourth == second || fourth == third);

                        addFieldDeclaration(type.first, {first, second, third, fourth}, float4TypeDeclaration, fourthConst, declarationScopes);
                    }
                }
            }
        }

        declarationScopes.pop_back();
    }

    StructDeclaration* float2x2TypeDeclaration = addStructDeclaration("float2x2", 16, declarationScopes);
    StructDeclaration* float3x3TypeDeclaration = addStructDeclaration("float3x3", 36, declarationScopes);
    StructDeclaration* float4x4TypeDeclaration = addStructDeclaration("float4x4", 64, declarationScopes);
    stringTypeDeclaration = addStructDeclaration("string", 8, declarationScopes);
    StructDeclaration* texture2DTypeDeclaration = addStructDeclaration("Texture2D", 0, declarationScopes);

    addFunctionDeclaration("sample", float4TypeDeclaration, {texture2DTypeDeclaration, float2TypeDeclaration}, declarationScopes);

    StructDeclaration* texture2DMSTypeDeclaration = addStructDeclaration("Texture2DMS", 0, declarationScopes);

    addFunctionDeclaration("load", float4TypeDeclaration, {texture2DMSTypeDeclaration, float2TypeDeclaration}, declarationScopes);

    // TODO: add other arithmetic operators
    // float2x2
    addOperatorDeclaration(Operator::Multiplication, float2x2TypeDeclaration, {float2x2TypeDeclaration, float2x2TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::Multiplication, float2TypeDeclaration, {float2x2TypeDeclaration, float2TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::Multiplication, float2TypeDeclaration, {float2TypeDeclaration, float2x2TypeDeclaration}, declarationScopes);

    addOperatorDeclaration(Operator::Division, float2x2TypeDeclaration, {float2x2TypeDeclaration, float2x2TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::Division, float2TypeDeclaration, {float2x2TypeDeclaration, float2TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::Division, float2TypeDeclaration, {float2TypeDeclaration, float2x2TypeDeclaration}, declarationScopes);

    // float3x3
    addOperatorDeclaration(Operator::Multiplication, float3x3TypeDeclaration, {float3x3TypeDeclaration, float3x3TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::Multiplication, float3TypeDeclaration, {float3x3TypeDeclaration, float3TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::Multiplication, float3TypeDeclaration, {float3TypeDeclaration, float3x3TypeDeclaration}, declarationScopes);

    addOperatorDeclaration(Operator::Division, float3x3TypeDeclaration, {float3x3TypeDeclaration, float3x3TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::Division, float3TypeDeclaration, {float3x3TypeDeclaration, float3TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::Division, float3TypeDeclaration, {float3TypeDeclaration, float3x3TypeDeclaration}, declarationScopes);

    // float4x4
    addOperatorDeclaration(Operator::Multiplication, float4x4TypeDeclaration, {float4x4TypeDeclaration, float4x4TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::Multiplication, float4TypeDeclaration, {float4x4TypeDeclaration, float4TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::Multiplication, float4TypeDeclaration, {float4TypeDeclaration, float4x4TypeDeclaration}, declarationScopes);
    
    addOperatorDeclaration(Operator::Division, float4x4TypeDeclaration, {float4x4TypeDeclaration, float4x4TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::Division, float4TypeDeclaration, {float4x4TypeDeclaration, float4TypeDeclaration}, declarationScopes);
    addOperatorDeclaration(Operator::Division, float4TypeDeclaration, {float4TypeDeclaration, float4x4TypeDeclaration}, declarationScopes);

    for (auto iterator = tokens.begin(); iterator != tokens.end();)
    {
        Declaration* declaration = parseTopLevelDeclaration(iterator, tokens.end(), declarationScopes);
        declarations.push_back(declaration);
    }
}

enum Rank
{
    NoRank = 0,
    Identity = 1,
    Promotion = 2,
    Conversion = 3
};

static Rank getRank(const QualifiedType& parameterType,
                    const QualifiedType& argumentType)
{
    if (!parameterType.typeDeclaration)
        throw std::runtime_error("Parameter does not have a type");

    if (!argumentType.typeDeclaration)
        throw std::runtime_error("Argument does not have a type");

    if (argumentType.typeDeclaration->getTypeKind() == parameterType.typeDeclaration->getTypeKind())
    {
        if (parameterType.typeDeclaration == argumentType.typeDeclaration)
            return Rank::Identity;
        else if (argumentType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::Array)
        {
            auto argumentTypeDeclaration = static_cast<const ArrayTypeDeclaration*>(argumentType.typeDeclaration);
            auto parameterTypeDeclaration = static_cast<const ArrayTypeDeclaration*>(parameterType.typeDeclaration);

            if (argumentTypeDeclaration->size == parameterTypeDeclaration->size)
                return Rank::Identity;
            else
                return Rank::NoRank;
        }
        else if (argumentType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::Scalar)
        {
            auto argumentTypeDeclaration = static_cast<const ScalarTypeDeclaration*>(argumentType.typeDeclaration);
            auto parameterTypeDeclaration = static_cast<const ScalarTypeDeclaration*>(parameterType.typeDeclaration);

            if (argumentTypeDeclaration->getScalarTypeKind() == parameterTypeDeclaration->getScalarTypeKind() &&
                argumentTypeDeclaration->size == parameterTypeDeclaration->size &&
                argumentTypeDeclaration->isUnsigned == parameterTypeDeclaration->isUnsigned)
                return Rank::Identity;
            else if (argumentTypeDeclaration->size == parameterTypeDeclaration->size &&
                     argumentTypeDeclaration->isUnsigned == parameterTypeDeclaration->isUnsigned)
                return Rank::Promotion;
            else
                return Rank::Conversion;
        }
    }

    return Rank::NoRank;
}

const CallableDeclaration* ASTContext::compareCallableDeclarations(const CallableDeclaration* callableDeclaration1,
                                                             const CallableDeclaration* callableDeclaration2,
                                                             const std::vector<QualifiedType>& arguments)
{
    const CallableDeclaration* result = nullptr;

    if (!arguments.empty() && // both functions should have arguments
        arguments.size() == callableDeclaration1->parameterDeclarations.size() &&
        arguments.size() == callableDeclaration2->parameterDeclarations.size()) // they should have the same number of parameters
    {
        for (uint32_t i = 0; i < arguments.size(); ++i)
        {
            const QualifiedType& argument = arguments[i];
            const QualifiedType& parameter1 = callableDeclaration1->parameterDeclarations[i]->qualifiedType;
            const QualifiedType& parameter2 = callableDeclaration2->parameterDeclarations[i]->qualifiedType;

            Rank rank1 = getRank(parameter1, argument);
            Rank rank2 = getRank(parameter2, argument);

            if (rank1 == NoRank && rank2 == NoRank) // no valid rank for both
                return nullptr;
            else if (rank1 == rank2) // equal ranks
                continue;
            else if (rank1 > rank2)
            {
                if (result == nullptr)
                    result = callableDeclaration1;
                else if (result != callableDeclaration1)
                    return nullptr;
            }
            else if (rank2 > rank1)
            {
                if (result == nullptr)
                    result = callableDeclaration2;
                else if (result != callableDeclaration2)
                    return nullptr;
            }
        }
    }

    return result;
}

FunctionDeclaration* ASTContext::resolveFunctionDeclaration(const std::string& name,
                                                            const std::vector<std::vector<Declaration*>>& declarationScopes,
                                                            const std::vector<QualifiedType>& arguments)
{
    std::vector<FunctionDeclaration*> candidateFunctionDeclarations;

    for (auto scopeIterator = declarationScopes.crbegin(); scopeIterator != declarationScopes.crend(); ++scopeIterator)
    {
        for (auto declarationIterator = scopeIterator->crbegin(); declarationIterator != scopeIterator->crend(); ++declarationIterator)
        {
            if ((*declarationIterator)->name == name)
            {
                if ((*declarationIterator)->getDeclarationKind() != Declaration::Kind::Callable) return nullptr;

                CallableDeclaration* callableDeclaration = static_cast<CallableDeclaration*>(*declarationIterator);

                if (callableDeclaration->getCallableDeclarationKind() != CallableDeclaration::Kind::Function) return nullptr;

                FunctionDeclaration* functionDeclaration = static_cast<FunctionDeclaration*>(callableDeclaration->getFirstDeclaration());

                if (std::find(candidateFunctionDeclarations.begin(), candidateFunctionDeclarations.end(), functionDeclaration) == candidateFunctionDeclarations.end())
                    candidateFunctionDeclarations.push_back(functionDeclaration);
            }
        }
    }

    std::vector<FunctionDeclaration*> viableFunctionDeclarations;

    for (FunctionDeclaration* functionDeclaration : candidateFunctionDeclarations)
    {
        if (functionDeclaration->parameterDeclarations.size() == arguments.size())
        {
            if (std::equal(arguments.begin(), arguments.end(),
                           functionDeclaration->parameterDeclarations.begin(),
                           [](const QualifiedType& qualifiedType,
                              const ParameterDeclaration* parameterDeclaration) {
                               bool scalar = qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::Scalar &&
                               qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::Scalar;

                               return (scalar || qualifiedType.typeDeclaration->getFirstDeclaration() == parameterDeclaration->qualifiedType.typeDeclaration->getFirstDeclaration());
                           }))
            {
                viableFunctionDeclarations.push_back(functionDeclaration);
            }
        }
    }

    if (viableFunctionDeclarations.empty())
        throw std::runtime_error("No matching function to call " + name + " found");
    else if (viableFunctionDeclarations.size() == 1)
        return *viableFunctionDeclarations.begin();
    else
    {
        if (arguments.empty()) // two or more functions with zero parameters
            throw std::runtime_error("Ambiguous call to " + name);

        for (auto first = viableFunctionDeclarations.begin(); first != viableFunctionDeclarations.end(); ++first)
        {
            bool best = true;
            for (auto second = viableFunctionDeclarations.begin(); second != viableFunctionDeclarations.end(); ++second)
            {
                if (first != second &&
                    compareCallableDeclarations(*first, *second, arguments) != *first)
                {
                    best = false;
                    break;
                }
            }

            if (best) return *first;
        };

        throw std::runtime_error("Ambiguous call to " + name);
    }

    return nullptr;
}

static std::string toString(Operator op)
{
    switch (op)
    {
        case Operator::Negation: return "Negation";
        case Operator::Positive: return "Positive";
        case Operator::Negative: return "Negative";
        case Operator::Addition: return "Addition";
        case Operator::Subtraction: return "Subtraction";
        case Operator::Multiplication: return "Multiplication";
        case Operator::Division: return "Division";
        case Operator::AdditionAssignment: return "AdditionAssignment";
        case Operator::SubtractAssignment: return "SubtractAssignment";
        case Operator::MultiplicationAssignment: return "MultiplicationAssignment";
        case Operator::DivisionAssignment: return "DivisionAssignment";
        case Operator::LessThan: return "LessThan";
        case Operator::LessThanEqual: return "LessThanEqual";
        case Operator::GreaterThan: return "GreaterThan";
        case Operator::GraterThanEqual: return "GraterThanEqual";
        case Operator::Equality: return "Equality";
        case Operator::Inequality: return "Inequality";
        case Operator::Assignment: return "Assignment";
        case Operator::Or: return "Or";
        case Operator::And: return "And";
        case Operator::Comma: return "Comma";
        case Operator::Conditional: return "Conditional";
        default: return "Unknown";
    }
}

OperatorDeclaration* ASTContext::resolveOperatorDeclaration(Operator op,
                                                            const std::vector<std::vector<Declaration*>>& declarationScopes,
                                                            const std::vector<QualifiedType>& parameters)
{
    std::vector<OperatorDeclaration*> candidateOperatorDeclarations;

    for (auto scopeIterator = declarationScopes.crbegin(); scopeIterator != declarationScopes.crend(); ++scopeIterator)
    {
        for (auto declarationIterator = scopeIterator->crbegin(); declarationIterator != scopeIterator->crend(); ++declarationIterator)
        {
            if ((*declarationIterator)->getDeclarationKind() == Declaration::Kind::Callable)
            {
                CallableDeclaration* callableDeclaration = static_cast<CallableDeclaration*>(*declarationIterator);

                if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::Operator)
                {
                    OperatorDeclaration* operatorDeclaration = static_cast<OperatorDeclaration*>(callableDeclaration->getFirstDeclaration());

                    if (operatorDeclaration->op == op &&
                        std::find(candidateOperatorDeclarations.begin(), candidateOperatorDeclarations.end(), operatorDeclaration) == candidateOperatorDeclarations.end())
                        candidateOperatorDeclarations.push_back(operatorDeclaration);
                }
            }
        }
    }

    std::vector<OperatorDeclaration*> viableOperatorDeclarations;

    for (OperatorDeclaration* operatorDeclaration : candidateOperatorDeclarations)
    {
        if (operatorDeclaration->parameterDeclarations.size() == parameters.size())
        {
            if (std::equal(parameters.begin(), parameters.end(),
                           operatorDeclaration->parameterDeclarations.begin(),
                           [](const QualifiedType& qualifiedType,
                              const ParameterDeclaration* parameterDeclaration) {

                               if (!qualifiedType.typeDeclaration) return true; // any type

                               bool scalar = qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::Scalar &&
                                   qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::Scalar;

                               return (scalar || qualifiedType.typeDeclaration->getFirstDeclaration() == parameterDeclaration->qualifiedType.typeDeclaration->getFirstDeclaration());
                           }))
            {
                viableOperatorDeclarations.push_back(operatorDeclaration);
            }
        }
    }

    if (viableOperatorDeclarations.empty())
        throw std::runtime_error("No matching function operator " + toString(op) + " found");
    else if (viableOperatorDeclarations.size() == 1)
        return *viableOperatorDeclarations.begin();
    else
    {
        if (parameters.empty()) // two or more functions with zero parameters
            throw std::runtime_error("Ambiguous call to operator " + toString(op));

        for (auto first = viableOperatorDeclarations.begin(); first != viableOperatorDeclarations.end(); ++first)
        {
            bool best = true;
            for (auto second = viableOperatorDeclarations.begin(); second != viableOperatorDeclarations.end(); ++second)
            {
                if (first != second &&
                    compareCallableDeclarations(*first, *second, parameters) != *first)
                {
                    best = false;
                    break;
                }
            }

            if (best) return *first;
        };

        throw std::runtime_error("Ambiguous call to operator " + toString(op));
    }

    return nullptr;
}

ArrayTypeDeclaration* ASTContext::getArrayTypeDeclaration(QualifiedType qualifiedType, uint32_t size)
{
    auto i = arrayTypeDeclarations.find(std::make_pair(qualifiedType, size));

    if (i != arrayTypeDeclarations.end())
    {
        return i->second;
    }
    else
    {
        ArrayTypeDeclaration* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new ArrayTypeDeclaration()));
        result->elementType = qualifiedType;
        result->size = size;

        arrayTypeDeclarations[std::make_pair(qualifiedType, size)] = result;
        return result;
    }
}

bool ASTContext::isType(std::vector<Token>::const_iterator iterator,
                        std::vector<Token>::const_iterator end,
                        std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (iterator == end)
        throw std::runtime_error("Unexpected end of file");

    return iterator->type == Token::Type::Bool ||
        iterator->type == Token::Type::Int ||
        iterator->type == Token::Type::Float ||
        iterator->type == Token::Type::Double ||
        (iterator->type == Token::Type::Identifier &&
         findTypeDeclaration(iterator->value, declarationScopes));
}

TypeDeclaration* ASTContext::parseType(std::vector<Token>::const_iterator& iterator,
                                       std::vector<Token>::const_iterator end,
                                       std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (iterator == end)
        throw std::runtime_error("Unexpected end of file");

    TypeDeclaration* result;

    if (iterator->type == Token::Type::Bool)
        result = boolTypeDeclaration;
    else if (iterator->type == Token::Type::Int)
        result = intTypeDeclaration;
    else if (iterator->type == Token::Type::Float)
        result = floatTypeDeclaration;
    else if (iterator->type == Token::Type::Double)
        throw std::runtime_error("Double precision floating point numbers are not supported");
    else if (iterator->type == Token::Type::Identifier)
    {
        if (!(result = findTypeDeclaration(iterator->value, declarationScopes)))
            throw std::runtime_error("Invalid type: " + iterator->value);
    }
    else
        throw std::runtime_error("Expected a type name");

    ++iterator;

    return result;
}

bool ASTContext::isDeclaration(std::vector<Token>::const_iterator iterator,
                               std::vector<Token>::const_iterator end,
                               std::vector<std::vector<Declaration*>>& declarationScopes)
{
    if (iterator == end)
        throw std::runtime_error("Unexpected end of file");

    return iterator->type == Token::Type::Const ||
        iterator->type == Token::Type::Static ||
        iterator->type == Token::Type::Volatile ||
        iterator->type == Token::Type::Inline ||
        iterator->type == Token::Type::Signed ||
        iterator->type == Token::Type::Unsigned ||
        iterator->type == Token::Type::Struct ||
        isType(iterator, end, declarationScopes);
}

Declaration* ASTContext::parseTopLevelDeclaration(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes)
{
    Declaration* declaration;
    if (!(declaration = parseDeclaration(iterator, end, declarationScopes, nullptr)))
        throw std::runtime_error("Failed to parse a declaration");

    if (declaration->getDeclarationKind() == Declaration::Kind::Callable)
    {
        CallableDeclaration* callableDeclaration = static_cast<CallableDeclaration*>(declaration);

        // semicolon is not needed after a function definition
        if (!callableDeclaration->body)
        {
            expectToken(Token::Type::Semicolon, iterator, end);

            ++iterator;
        }
    }
    else
    {
        expectToken(Token::Type::Semicolon, iterator, end);

        ++iterator;
    }

    return declaration;
}

ASTContext::Specifiers ASTContext::parseSpecifiers(std::vector<Token>::const_iterator& iterator,
                                                   std::vector<Token>::const_iterator end)
{
    ASTContext::Specifiers result;
    result.isConst = false;
    result.isInline = false;
    result.isStatic = false;
    result.isVolatile = false;

    for (;;)
    {
        if (isToken(Token::Type::Const, iterator, end))
        {
            ++iterator;
            result.isConst = true;
        }
        else if (isToken(Token::Type::Extern, iterator, end))
        {
            ++iterator;
            result.isExtern = true;
        }
        else if (isToken(Token::Type::Inline, iterator, end))
        {
            ++iterator;
            result.isInline = true;
        }
        else if (isToken(Token::Type::Static, iterator, end))
        {
            ++iterator;
            result.isStatic = true;
        }
        else if (isToken(Token::Type::Volatile, iterator, end))
        {
            ++iterator;
            result.isVolatile = true;
        }
        else break;
    }

    return result;
}

std::vector<std::pair<std::string, std::vector<std::string>>> ASTContext::parseAttributes(std::vector<Token>::const_iterator& iterator,
                                                                                          std::vector<Token>::const_iterator end)
{
    std::vector<std::pair<std::string, std::vector<std::string>>> result;

    while (isToken(Token::Type::LeftBracket, iterator, end) &&
           isToken(Token::Type::LeftBracket, iterator + 1, end))
    {
        ++iterator;
        ++iterator;

        expectToken(Token::Type::Identifier, iterator, end);

        std::pair<std::string, std::vector<std::string>> attribute;

        attribute.first = iterator->value;

        ++iterator;

        if (isToken(Token::Type::LeftParenthesis, iterator, end))
        {
            ++iterator;

            for (;;)
            {
                expectToken({Token::Type::IntLiteral, Token::Type::FloatLiteral, Token::Type::DoubleLiteral, Token::Type::CharLiteral, Token::Type::StringLiteral}, iterator, end);

                attribute.second.push_back(iterator->value);
                ++iterator;

                if (!isToken(Token::Type::Comma, iterator, end))
                    break;
            }

            expectToken(Token::Type::RightParenthesis, iterator, end);

            ++iterator;
        }

        expectToken(Token::Type::RightBracket, iterator, end);

        ++iterator;

        expectToken(Token::Type::RightBracket, iterator, end);

        ++iterator;

        result.push_back(attribute);
    }

    return result;
}

Declaration* ASTContext::parseDeclaration(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent)
{
    if (isToken(Token::Type::Semicolon, iterator, end))
    {
        Declaration* declaration;
        constructs.push_back(std::unique_ptr<Construct>(declaration = new Declaration(Declaration::Kind::Empty)));

        return declaration;
    }
    else if (isToken(Token::Type::Struct, iterator, end))
    {
        ++iterator;

        StructDeclaration* declaration;
        if (!(declaration = parseStructDeclaration(iterator, end, declarationScopes, parent)))
            throw std::runtime_error("Failed to parse a structure declaration");

        return declaration;
    }
    /*else if (isToken(Token::Type::KEYWORD_TYPEDEF, iterator, end))
    {
        ++iterator;

        TypeDefinitionDeclaration* declaration;
        if (!(declaration = parseTypeDefinitionDeclaration(iterator, end, declarationScopes, parent)))
            throw std::runtime_error("Failed to parse a type definition declaration");

        return declaration;
    }*/
    else
    {
        std::vector<std::pair<std::string, std::vector<std::string>>> attributes = parseAttributes(iterator, end);
        ASTContext::Specifiers specifiers = parseSpecifiers(iterator, end);

        QualifiedType qualifiedType;
        qualifiedType.isConst = specifiers.isConst;
        qualifiedType.isVolatile = specifiers.isVolatile;

        bool isExtern = specifiers.isExtern;
        bool isInline = specifiers.isInline;
        bool isStatic = specifiers.isStatic;

        if (isToken(Token::Type::Void, iterator, end))
        {
            ++iterator;
        }
        else
        {
            if (!(qualifiedType.typeDeclaration = parseType(iterator, end, declarationScopes)))
                return nullptr;

            if (qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::Struct)
            {
                StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(qualifiedType.typeDeclaration);

                if (!structDeclaration->definition)
                    throw std::runtime_error("Incomplete type " + qualifiedType.typeDeclaration->name);
            }
        }

        specifiers = parseSpecifiers(iterator, end);

        if (specifiers.isConst) qualifiedType.isConst = true;
        if (specifiers.isVolatile) qualifiedType.isVolatile = true;

        if (specifiers.isExtern) isExtern = true;
        if (specifiers.isInline) isInline = true;
        if (specifiers.isStatic) isStatic = true;

        if (isToken(Token::Type::Operator, iterator, end))
            throw std::runtime_error("Operator overloads are not supported");

        expectToken(Token::Type::Identifier, iterator, end);

        std::string name = iterator->value;

        ++iterator;

        if (isToken(Token::Type::LeftParenthesis, iterator, end) &&
            (isToken(Token::Type::RightParenthesis, iterator + 1, end) ||
             isToken(Token::Type::Void, iterator + 1, end) ||
             isDeclaration(iterator + 1, end, declarationScopes)))  // function declaration
        {
            ++iterator;

            FunctionDeclaration* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new FunctionDeclaration()));
            result->parent = parent;
            result->qualifiedType = qualifiedType;
            result->isStatic = isStatic;
            result->isInline = isInline;
            result->name = name;

            std::vector<QualifiedType> parameters;

            if (isToken(Token::Type::Void, iterator, end))
            {
                ++iterator;
            }
            else if (!isToken(Token::Type::RightParenthesis, iterator, end))
            {
                for (;;)
                {
                    ParameterDeclaration* parameterDeclaration;
                    if (!(parameterDeclaration = parseParameterDeclaration(iterator, end, declarationScopes, result)))
                        return nullptr;

                    result->parameterDeclarations.push_back(parameterDeclaration);
                    parameters.push_back(parameterDeclaration->qualifiedType);

                    if (!isToken(Token::Type::Comma, iterator, end))
                        break;

                    ++iterator;
                }
            }

            expectToken(Token::Type::RightParenthesis, iterator, end);

            ++iterator;

            result->previousDeclaration = findFunctionDeclaration(name, declarationScopes, parameters);

            attributes = parseAttributes(iterator, end);

            for (std::pair<std::string, std::vector<std::string>>& attribute : attributes)
            {
                if (attribute.first == "program")
                {
                    if (attribute.second.size() == 1)
                    {
                        result->isProgram = true;
                        if (attribute.second.front() == "fragment") result->program = Program::Fragment;
                        else if (attribute.second.front() == "vertex") result->program = Program::Vertex;
                        else
                            throw std::runtime_error("Invalid program" + attribute.second.front());
                    }
                    else
                        throw std::runtime_error("Invalid parameters for attribute " + attribute.first);
                }
                else
                    throw std::runtime_error("Invalid attribute " + attribute.first);
            }

            declarationScopes.back().push_back(result);

            // set the definition of the previous declaration
            if (result->previousDeclaration) result->definition = result->previousDeclaration->definition;

            if (isToken(Token::Type::LeftBrace, iterator, end))
            {
                // check if only one definition exists
                if (result->definition)
                    throw std::runtime_error("Redefinition of " + result->name);

                // set the definition pointer of all previous declarations
                Declaration* previousDeclaration = result->previousDeclaration;
                while (previousDeclaration)
                {
                    previousDeclaration->definition = result;
                    previousDeclaration = previousDeclaration->previousDeclaration;
                }

                declarationScopes.push_back(std::vector<Declaration*>()); // add scope for parameters

                for (ParameterDeclaration* parameterDeclaration : result->parameterDeclarations)
                    declarationScopes.back().push_back(parameterDeclaration);

                // parse body
                if (!(result->body = parseCompoundStatement(iterator, end, declarationScopes, result)))
                    throw std::runtime_error("Failed to parse a compound statement");

                declarationScopes.pop_back();
            }

            return result;
        }
        else // variable declaration
        {
            if (isInline)
                throw std::runtime_error("Variables can not be inline");

            if (findDeclaration(name, declarationScopes.back()))
                throw std::runtime_error("Redefinition of " + name);

            VariableDeclaration* result;
            constructs.push_back(std::unique_ptr<VariableDeclaration>(result = new VariableDeclaration()));
            result->parent = parent;
            result->qualifiedType = qualifiedType;
            if (isExtern) result->storageClass = VariableDeclaration::StorageClass::Extern;
            else if (isStatic) result->storageClass = VariableDeclaration::StorageClass::Static;
            result->name = name;

            while (isToken(Token::Type::LeftBracket, iterator, end))
            {
                ++iterator;

                expectToken(Token::Type::IntLiteral, iterator, end);

                int size = std::stoi(iterator->value);

                ++iterator;

                if (size <= 0)
                    throw std::runtime_error("Array size must be greater than zero");

                result->qualifiedType.typeDeclaration = getArrayTypeDeclaration(result->qualifiedType, static_cast<uint32_t>(size));

                expectToken(Token::Type::RightBracket, iterator, end);

                ++iterator;
            }

            if (isToken(Token::Type::LeftParenthesis, iterator, end))
            {
                ++iterator;

                if (!(result->initialization = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, result)))
                    return nullptr;

                if (!result->initialization->qualifiedType.typeDeclaration)
                    throw std::runtime_error("Initialization with a void type");

                // TODO: check for comma and parse multiple expressions

                expectToken(Token::Type::RightParenthesis, iterator, end);

                ++iterator;
            }
            else if (isToken(Token::Type::Assignment, iterator, end))
            {
                ++iterator;

                if (!(result->initialization = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, result)))
                    return nullptr;

                if (!result->initialization->qualifiedType.typeDeclaration)
                    throw std::runtime_error("Initialization with a void type");
            }

            declarationScopes.back().push_back(result);

            return result;
        }
    }

    return nullptr;
}

StructDeclaration* ASTContext::parseStructDeclaration(std::vector<Token>::const_iterator& iterator,
                                                      std::vector<Token>::const_iterator end,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                                      Construct* parent)
{
    expectToken(Token::Type::Identifier, iterator, end);

    StructDeclaration* result;
    constructs.push_back(std::unique_ptr<Construct>(result = new StructDeclaration()));
    result->parent = parent;
    result->name = iterator->value;
    result->previousDeclaration = findStructDeclaration(iterator->value, declarationScopes);

    ++iterator;

    // set the definition of the previous declaration
    if (result->previousDeclaration) result->definition = result->previousDeclaration->definition;

    if (isToken(Token::Type::LeftBrace, iterator, end))
    {
        ++iterator;

        // check if only one definition exists
        if (result->definition)
            throw std::runtime_error("Redefinition of " + result->name);

        result->definition = result;

        // set the definition pointer of all previous declarations
        Declaration* previousDeclaration = result->previousDeclaration;
        while (previousDeclaration)
        {
            previousDeclaration->definition = result;
            previousDeclaration = previousDeclaration->previousDeclaration;
        }

        for (;;)
        {
            if (isToken(Token::Type::RightBrace, iterator, end))
            {
                ++iterator;

                declarationScopes.back().push_back(result);
                break;
            }
            else
            {
                Declaration* memberDeclaration;
                if (!(memberDeclaration = parseMemberDeclaration(iterator, end, declarationScopes, result)))
                    return nullptr;

                expectToken(Token::Type::Semicolon, iterator, end);

                if (result->findMemberDeclaration(memberDeclaration->name))
                    throw std::runtime_error("Redefinition of member " + memberDeclaration->name);

                ++iterator;

                memberDeclaration->parent = result;

                result->memberDeclarations.push_back(memberDeclaration);
            }
        }
    }

    declarationScopes.back().push_back(result);

    addOperatorDeclaration(Operator::Comma, result, {nullptr, result}, declarationScopes);
    addOperatorDeclaration(Operator::Assignment, result, {result, result}, declarationScopes);
    addOperatorDeclaration(Operator::Equality, result, {result, result}, declarationScopes);
    addOperatorDeclaration(Operator::Inequality, result, {result, result}, declarationScopes);

    return result;
}

Declaration* ASTContext::parseMemberDeclaration(std::vector<Token>::const_iterator& iterator,
                                                std::vector<Token>::const_iterator end,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                Construct* parent)
{
    if (isToken(Token::Type::Semicolon, iterator, end))
    {
        Declaration* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new Declaration(Declaration::Kind::Empty)));
        result->parent = parent;

        return result;
    }
    else
    {
        FieldDeclaration* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new FieldDeclaration()));
        result->parent = parent;

        std::vector<std::pair<std::string, std::vector<std::string>>> attributes = parseAttributes(iterator, end);
        ASTContext::Specifiers specifiers = parseSpecifiers(iterator, end);

        result->qualifiedType.isConst = specifiers.isConst;
        result->qualifiedType.isVolatile = specifiers.isVolatile;

        bool isStatic = specifiers.isStatic;
        bool isInline = specifiers.isInline;

        if (!(result->qualifiedType.typeDeclaration = parseType(iterator, end, declarationScopes)))
            return nullptr;

        if (result->qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::Struct)
        {
            StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->qualifiedType.typeDeclaration);

            if (!structDeclaration->definition)
                throw std::runtime_error("Incomplete type " + result->qualifiedType.typeDeclaration->name);
        }

        specifiers = parseSpecifiers(iterator, end);

        if (specifiers.isConst) result->qualifiedType.isConst = true;
        if (specifiers.isVolatile) result->qualifiedType.isVolatile = true;

        if (specifiers.isStatic) isStatic = true;
        if (specifiers.isInline) isInline = true;

        if (isStatic)
            throw std::runtime_error("Members can not be static");

        if (isInline)
            throw std::runtime_error("Members can not be inline");

        expectToken(Token::Type::Identifier, iterator, end);

        result->name = iterator->value;

        ++iterator;

        attributes = parseAttributes(iterator, end);

        while (isToken(Token::Type::LeftBracket, iterator, end))
        {
            ++iterator;

            expectToken(Token::Type::IntLiteral, iterator, end);

            int size = std::stoi(iterator->value);

            ++iterator;

            if (size <= 0)
                throw std::runtime_error("Array size must be greater than zero");

            result->qualifiedType.typeDeclaration = getArrayTypeDeclaration(result->qualifiedType, static_cast<uint32_t>(size));

            expectToken(Token::Type::RightBracket, iterator, end);

            ++iterator;
        }

        attributes = parseAttributes(iterator, end);

        for (std::pair<std::string, std::vector<std::string>>& attribute : attributes)
        {
            if (attribute.first == "semantic")
            {
                if (attribute.second.size() == 1)
                {
                    Semantic semantic = Semantic::None;

                    // TODO: find slot number
                    if (attribute.second.front() == "binormal") semantic = Semantic::Binormal;
                    else if (attribute.second.front() == "blend_indices") semantic = Semantic::BlendIndices;
                    else if (attribute.second.front() == "blend_weight") semantic = Semantic::BlendWeight;
                    else if (attribute.second.front() == "color") semantic = Semantic::Color;
                    else if (attribute.second.front() == "normal") semantic = Semantic::Normal;
                    else if (attribute.second.front() == "position") semantic = Semantic::Position;
                    else if (attribute.second.front() == "position_transformed") semantic = Semantic::PositionTransformed;
                    else if (attribute.second.front() == "point_size") semantic = Semantic::PointSize;
                    else if (attribute.second.front() == "tangent") semantic = Semantic::Tangent;
                    else if (attribute.second.front() == "texture_coordinates") semantic = Semantic::TextureCoordinates;
                    else
                        throw std::runtime_error("Invalid semantic " + attribute.second.front());

                    result->semantic = semantic;
                }
                else
                    throw std::runtime_error("Invalid parameters for attribute " + attribute.first);
            }
            else
                throw std::runtime_error("Invalid attribute " + attribute.first);
        }

        return result;
    }

    return nullptr;
}

ParameterDeclaration* ASTContext::parseParameterDeclaration(std::vector<Token>::const_iterator& iterator,
                                                            std::vector<Token>::const_iterator end,
                                                            std::vector<std::vector<Declaration*>>& declarationScopes,
                                                            Construct* parent)
{
    ParameterDeclaration* result;
    constructs.push_back(std::unique_ptr<Construct>(result = new ParameterDeclaration()));
    result->parent = parent;

    std::vector<std::pair<std::string, std::vector<std::string>>> attributes = parseAttributes(iterator, end);

    ASTContext::Specifiers specifiers = parseSpecifiers(iterator, end);

    result->qualifiedType.isConst = specifiers.isConst;
    result->qualifiedType.isVolatile = specifiers.isVolatile;

    bool isStatic = specifiers.isStatic;
    bool isInline = specifiers.isInline;

    if (!(result->qualifiedType.typeDeclaration = parseType(iterator, end, declarationScopes)))
        return nullptr;

    if (result->qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::Struct)
    {
        StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->qualifiedType.typeDeclaration);

        if (!structDeclaration->definition)
            throw std::runtime_error("Incomplete type " + result->qualifiedType.typeDeclaration->name);
    }

    specifiers = parseSpecifiers(iterator, end);

    if (specifiers.isConst) result->qualifiedType.isConst = true;
    if (specifiers.isVolatile) result->qualifiedType.isVolatile = true;

    if (specifiers.isStatic) isStatic = true;
    if (specifiers.isInline) isInline = true;

    if (isStatic)
        throw std::runtime_error("Parameters can not be static");

    if (isInline)
        throw std::runtime_error("Parameters can not be inline");

    if (isToken(Token::Type::Identifier, iterator, end))
    {
        result->name = iterator->value;

        ++iterator;
    }

    attributes = parseAttributes(iterator, end);

    while (isToken(Token::Type::LeftBracket, iterator, end))
    {
        ++iterator;

        expectToken(Token::Type::IntLiteral, iterator, end);

        int size = std::stoi(iterator->value);

        ++iterator;

        if (size <= 0)
            throw std::runtime_error("Array size must be greater than zero");

        result->qualifiedType.typeDeclaration = getArrayTypeDeclaration(result->qualifiedType, static_cast<uint32_t>(size));

        expectToken(Token::Type::RightBracket, iterator, end);

        ++iterator;
    }

    attributes = parseAttributes(iterator, end);

    return result;
}

/*TypeDefinitionDeclaration* ASTContext::parseTypeDefinitionDeclaration(std::vector<Token>::const_iterator& iterator,
                                                                        std::vector<Token>::const_iterator end,
                                                                        std::vector<std::vector<Declaration*>>& declarationScopes,
                                                                        Construct* parent)
{
    throw std::runtime_error("Typedef is not supported");
    return nullptr;

    TypeDefinitionDeclaration* result;
    constructs.push_back(std::unique_ptr<Construct>(result = new TypeDefinitionDeclaration()));
    result->parent = parent;
    result->kind = Construct::Kind::Declaration;
    result->declarationKind = Declaration::Kind::Type;
    result->typeKind = TypeDeclaration::Kind::TypeDefinition;
    result->Declaration = findTypeDeclaration(iterator->value, declarationScopes);

    if (!(result->qualifiedType.typeDeclaration = parseType(iterator, end, declarationScopes)))
        return nullptr;

    if (result->qualifiedType.typeDeclaration->getTypeKind() == TypeDeclaration::Kind::STRUCT)
    {
        StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->qualifiedType.typeDeclaration);

        if (!structDeclaration->hasDefinition)
            throw std::runtime_error("Incomplete type " + result->qualifiedType.typeDeclaration->name);
    }

    expectToken(Token::Type::IDENTIFIER, iterator, end);

    result->name = iterator->value;

    ++iterator;

    expectToken(Token::Type::SEMICOLON, iterator, end);

    ++iterator;

    return result;
}*/

Statement* ASTContext::parseStatement(std::vector<Token>::const_iterator& iterator,
                                      std::vector<Token>::const_iterator end,
                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                      Construct* parent)
{
    if (isToken(Token::Type::LeftBrace, iterator, end))
    {
        return parseCompoundStatement(iterator, end, declarationScopes, parent);
    }
    else if (isToken(Token::Type::If, iterator, end))
    {
        return parseIfStatement(iterator, end, declarationScopes, parent);
    }
    else if (isToken(Token::Type::For, iterator, end))
    {
        return parseForStatement(iterator, end, declarationScopes, parent);
    }
    else if (isToken(Token::Type::Switch, iterator, end))
    {
        return parseSwitchStatement(iterator, end, declarationScopes, parent);
    }
    else if (isToken(Token::Type::Case, iterator, end))
    {
        return parseCaseStatement(iterator, end, declarationScopes, parent);
    }
    else if (isToken(Token::Type::Default, iterator, end))
    {
        return parseDefaultStatement(iterator, end, declarationScopes, parent);
    }
    else if (isToken(Token::Type::While, iterator, end))
    {
        return parseWhileStatement(iterator, end, declarationScopes, parent);
    }
    else if (isToken(Token::Type::Do, iterator, end))
    {
        return parseDoStatement(iterator, end, declarationScopes, parent);
    }
    else if (isToken(Token::Type::Break, iterator, end))
    {
        ++iterator;

        BreakStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new BreakStatement()));
        result->parent = parent;

        expectToken(Token::Type::Semicolon, iterator, end);

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::Continue, iterator, end))
    {
        ++iterator;

        ContinueStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new ContinueStatement()));
        result->parent = parent;

        expectToken(Token::Type::Semicolon, iterator, end);

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::Return, iterator, end))
    {
        ++iterator;

        ReturnStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new ReturnStatement()));
        result->parent = parent;

        if (!(result->result = parseExpression(iterator, end, declarationScopes, result)))
            throw std::runtime_error("Expected an expression");

        Construct* currentParent = parent;
        CallableDeclaration* callableDeclaration = nullptr;

        while (currentParent)
        {
            if (currentParent->getKind() == Construct::Kind::Declaration)
            {
                Declaration* declaration = static_cast<Declaration*>(currentParent);
                if (declaration->getDeclarationKind() == Declaration::Kind::Callable)
                {
                    callableDeclaration = static_cast<CallableDeclaration*>(currentParent);
                    break;
                }
            }

            currentParent = currentParent->parent;
        }

        if (!callableDeclaration)
            throw std::runtime_error("Return statement outside of a function");

        if (callableDeclaration->qualifiedType.typeDeclaration != result->result->qualifiedType.typeDeclaration)
            throw std::runtime_error("Invalid type for a return statement");

        expectToken(Token::Type::Semicolon, iterator, end);

        ++iterator;

        return result;
    }
    else if (isDeclaration(iterator, end, declarationScopes))
    {
        DeclarationStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new DeclarationStatement()));
        result->parent = parent;

        if (!(result->declaration = parseDeclaration(iterator, end, declarationScopes, result)))
            return nullptr;

        if (result->declaration->getDeclarationKind() != Declaration::Kind::Variable)
            throw std::runtime_error("Expected a variable declaration");

        expectToken(Token::Type::Semicolon, iterator, end);

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::Semicolon, iterator, end))
    {
        ++iterator;

        Statement* statement = new Statement(Statement::Kind::Empty);
        constructs.push_back(std::unique_ptr<Construct>(statement));
        statement->parent = parent;

        return statement;
    }
    else if (isToken(Token::Type::Asm, iterator, end))
    {
        throw std::runtime_error("asm statements are not supported");
    }
    else if (isToken(Token::Type::Goto, iterator, end))
    {
        throw std::runtime_error("goto statements are not supported");
    }
    else if (isToken({Token::Type::Try,
        Token::Type::Catch,
        Token::Type::Throw}, iterator, end))
    {
        throw std::runtime_error("Exceptions are not supported");
    }
    else
    {
        ExpressionStatement* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new ExpressionStatement()));
        result->parent = parent;

        if (!(result->expression = parseExpression(iterator, end, declarationScopes, result)))
            return nullptr;

        expectToken(Token::Type::Semicolon, iterator, end);

        ++iterator;

        return result;
    }

    return nullptr;
}

CompoundStatement* ASTContext::parseCompoundStatement(std::vector<Token>::const_iterator& iterator,
                                                      std::vector<Token>::const_iterator end,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                                      Construct* parent)
{
    expectToken(Token::Type::LeftBrace, iterator, end);

    ++iterator;

    declarationScopes.push_back(std::vector<Declaration*>());

    CompoundStatement* result;
    constructs.push_back(std::unique_ptr<Construct>(result = new CompoundStatement()));
    result->parent = parent;

    for (;;)
    {
        if (isToken(Token::Type::RightBrace, iterator, end))
        {
            ++iterator;
            break;
        }
        else
        {
            Statement* statement;
            if (!(statement = parseStatement(iterator, end, declarationScopes, result)))
                throw std::runtime_error("Failed to parse a statement");

            result->statements.push_back(statement);
        }
    }

    declarationScopes.pop_back();

    return result;
}

IfStatement* ASTContext::parseIfStatement(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent)
{
    expectToken(Token::Type::If, iterator, end);

    ++iterator;

    IfStatement* result;
    constructs.push_back(std::unique_ptr<Construct>(result = new IfStatement()));
    result->parent = parent;

    expectToken(Token::Type::LeftParenthesis, iterator, end);

    ++iterator;

    // TODO: add implicit cast to bool
    if (isDeclaration(iterator, end, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(iterator, end, declarationScopes, result)))
            return nullptr;

        if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
            declaration->getDeclarationKind() != Declaration::Kind::Parameter)
            throw std::runtime_error("Expected a variable declaration");

        result->condition = declaration;
    }
    else
    {
        if (!(result->condition = parseExpression(iterator, end, declarationScopes, result)))
            return nullptr;
    }

    expectToken(Token::Type::RightParenthesis, iterator, end);

    ++iterator;

    Statement* statement;
    if (!(statement = parseStatement(iterator, end, declarationScopes, result)))
        throw std::runtime_error("Failed to parse the statement");

    result->body = statement;

    if (isToken(Token::Type::Else, iterator, end))
    {
        ++iterator;

        if (!(statement = parseStatement(iterator, end, declarationScopes, result)))
            throw std::runtime_error("Failed to parse the statement");

        result->elseBody = statement;
    }

    return result;
}

ForStatement* ASTContext::parseForStatement(std::vector<Token>::const_iterator& iterator,
                                            std::vector<Token>::const_iterator end,
                                            std::vector<std::vector<Declaration*>>& declarationScopes,
                                            Construct* parent)
{
    expectToken(Token::Type::For, iterator, end);

    ++iterator;

    ForStatement* result;
    constructs.push_back(std::unique_ptr<Construct>(result = new ForStatement()));
    result->parent = parent;

    expectToken(Token::Type::LeftParenthesis, iterator, end);

    ++iterator;

    if (isDeclaration(iterator, end, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(iterator, end, declarationScopes, result)))
            return nullptr;

        if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
            declaration->getDeclarationKind() != Declaration::Kind::Parameter)
            throw std::runtime_error("Expected a variable declaration");

        result->condition = declaration;

        expectToken(Token::Type::Semicolon, iterator, end);

        ++iterator;
    }
    else if (isToken(Token::Type::Semicolon, iterator, end))
    {
        ++iterator;

        result->initialization = nullptr;
    }
    else
    {
        if (!(result->initialization = parseExpression(iterator, end, declarationScopes, result)))
            return nullptr;

        expectToken(Token::Type::Semicolon, iterator, end);

        ++iterator;
    }

    // TODO: add implicit cast to bool
    if (isDeclaration(iterator, end, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(iterator, end, declarationScopes, result)))
            return nullptr;

        if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
            declaration->getDeclarationKind() != Declaration::Kind::Parameter)
            throw std::runtime_error("Expected a variable declaration");

        result->condition = declaration;

        expectToken(Token::Type::Semicolon, iterator, end);

        ++iterator;
    }
    else if (isToken(Token::Type::Semicolon, iterator, end))
    {
        ++iterator;

        result->condition = nullptr;
    }
    else
    {
        if (!(result->condition = parseExpression(iterator, end, declarationScopes, result)))
            return nullptr;

        expectToken(Token::Type::Semicolon, iterator, end);

        ++iterator;
    }

    if (isToken(Token::Type::RightParenthesis, iterator, end))
    {
        ++iterator;

        result->increment = nullptr;
    }
    else
    {
        if (!(result->increment = parseExpression(iterator, end, declarationScopes, result)))
            return nullptr;

        expectToken(Token::Type::RightParenthesis, iterator, end);

        ++iterator;
    }

    if (!(result->body = parseStatement(iterator, end, declarationScopes, result)))
        return nullptr;

    return result;
}

SwitchStatement* ASTContext::parseSwitchStatement(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  Construct* parent)
{
    expectToken(Token::Type::Switch, iterator, end);

    ++iterator;

    SwitchStatement* result;
    constructs.push_back(std::unique_ptr<Construct>(result = new SwitchStatement()));
    result->parent = parent;

    expectToken(Token::Type::LeftParenthesis, iterator, end);

    ++iterator;

    // TODO: add implicit cast to int
    if (isDeclaration(iterator, end, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(iterator, end, declarationScopes, result)))
            return nullptr;

        if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
            declaration->getDeclarationKind() != Declaration::Kind::Parameter)
            throw std::runtime_error("Expected a variable declaration");

        result->condition = declaration;
    }
    else
    {
        if (!(result->condition = parseExpression(iterator, end, declarationScopes, result)))
            return nullptr;
    }

    expectToken(Token::Type::RightParenthesis, iterator, end);

    ++iterator;

    if (!(result->body = parseStatement(iterator, end, declarationScopes, result)))
        return nullptr;

    return result;
}

CaseStatement* ASTContext::parseCaseStatement(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent)
{
    expectToken(Token::Type::Case, iterator, end);

    ++iterator;

    CaseStatement* result;
    constructs.push_back(std::unique_ptr<Construct>(result = new CaseStatement()));
    result->parent = parent;

    if (!(result->condition = parseExpression(iterator, end, declarationScopes, result)))
        throw std::runtime_error("Expected an expression");

    expectToken(Token::Type::Colon, iterator, end);

    ++iterator;

    if (!(result->body = parseStatement(iterator, end, declarationScopes, result)))
        return nullptr;

    return result;
}

DefaultStatement* ASTContext::parseDefaultStatement(std::vector<Token>::const_iterator& iterator,
                                                    std::vector<Token>::const_iterator end,
                                                    std::vector<std::vector<Declaration*>>& declarationScopes,
                                                    Construct* parent)
{
    expectToken(Token::Type::Default, iterator, end);

    ++iterator;

    DefaultStatement* result;
    constructs.push_back(std::unique_ptr<Construct>(result = new DefaultStatement()));
    result->parent = parent;

    expectToken(Token::Type::Colon, iterator, end);

    ++iterator;

    if (!(result->body = parseStatement(iterator, end, declarationScopes, result)))
        return nullptr;

    return result;
}

WhileStatement* ASTContext::parseWhileStatement(std::vector<Token>::const_iterator& iterator,
                                                std::vector<Token>::const_iterator end,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                Construct* parent)
{
    expectToken(Token::Type::While, iterator, end);

    ++iterator;

    WhileStatement* result;
    constructs.push_back(std::unique_ptr<Construct>(result = new WhileStatement()));
    result->parent = parent;

    expectToken(Token::Type::LeftParenthesis, iterator, end);

    ++iterator;

    // TODO: add implicit cast to bool
    if (isDeclaration(iterator, end, declarationScopes))
    {
        Declaration* declaration;
        if (!(declaration = parseDeclaration(iterator, end, declarationScopes, result)))
            return nullptr;

        if (declaration->getDeclarationKind() != Declaration::Kind::Variable &&
            declaration->getDeclarationKind() != Declaration::Kind::Parameter)
            throw std::runtime_error("Expected a variable declaration");

        result->condition = declaration;
    }
    else
    {
        if (!(result->condition = parseExpression(iterator, end, declarationScopes, result)))
            return nullptr;
    }

    expectToken(Token::Type::RightParenthesis, iterator, end);

    ++iterator;

    if (!(result->body = parseStatement(iterator, end, declarationScopes, result)))
        return nullptr;

    return result;
}

DoStatement* ASTContext::parseDoStatement(std::vector<Token>::const_iterator& iterator,
                                          std::vector<Token>::const_iterator end,
                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                          Construct* parent)
{
    expectToken(Token::Type::Do, iterator, end);

    ++iterator;

    DoStatement* result;
    constructs.push_back(std::unique_ptr<Construct>(result = new DoStatement()));
    result->parent = parent;

    if (!(result->body = parseStatement(iterator, end, declarationScopes, result)))
        return nullptr;

    expectToken(Token::Type::While, iterator, end);

    ++iterator;

    expectToken(Token::Type::LeftParenthesis, iterator, end);

    ++iterator;

    // TODO: add implicit cast to bool
    // expression
    if (!(result->condition = parseExpression(iterator, end, declarationScopes, result)))
        return nullptr;

    expectToken(Token::Type::RightParenthesis, iterator, end);

    ++iterator;

    expectToken(Token::Type::Semicolon, iterator, end);

    ++iterator;

    return result;
}

CastExpression* ASTContext::addImplicitCast(Expression* expression,
                                            TypeDeclaration* typeDeclaration,
                                            Expression::Category category)
{
    CastExpression* result = new CastExpression(CastExpression::Kind::Implicit);
    constructs.push_back(std::unique_ptr<Construct>(result));
    result->parent = expression->parent;
    result->qualifiedType.typeDeclaration = typeDeclaration;
    result->category = category;

    result->expression = expression;
    expression->parent = result;

    return result;
}

Expression* ASTContext::parsePrimaryExpression(std::vector<Token>::const_iterator& iterator,
                                               std::vector<Token>::const_iterator end,
                                               std::vector<std::vector<Declaration*>>& declarationScopes,
                                               Construct* parent)
{
    if (isToken(Token::Type::IntLiteral, iterator, end))
    {
        IntegerLiteralExpression* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new IntegerLiteralExpression()));
        result->parent = parent;
        result->qualifiedType.typeDeclaration = intTypeDeclaration;
        result->category = Expression::Category::Rvalue;
        result->value = strtoll(iterator->value.c_str(), nullptr, 0);

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::FloatLiteral, iterator, end))
    {
        FloatingPointLiteralExpression* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new FloatingPointLiteralExpression()));
        result->parent = parent;
        result->qualifiedType.typeDeclaration = floatTypeDeclaration;
        result->category = Expression::Category::Rvalue;
        result->value = strtod(iterator->value.c_str(), nullptr);

        ++iterator;

        return result;
    }
    else if (isToken({Token::Type::DoubleLiteral, Token::Type::Double}, iterator, end))
    {
        throw std::runtime_error("Double precision floating point numbers are not supported");
    }
    else if (isToken(Token::Type::StringLiteral, iterator, end))
    {
        StringLiteralExpression* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new StringLiteralExpression()));
        result->parent = parent;
        result->qualifiedType.typeDeclaration = stringTypeDeclaration;
        result->category = Expression::Category::Rvalue;
        result->value = iterator->value;

        ++iterator;

        return result;
    }
    else if (isToken({Token::Type::True, Token::Type::False}, iterator, end))
    {
        BooleanLiteralExpression* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new BooleanLiteralExpression()));
        result->parent = parent;
        result->qualifiedType.typeDeclaration = boolTypeDeclaration;
        result->category = Expression::Category::Rvalue;
        result->value = (iterator->type == Token::Type::True);

        ++iterator;

        return result;
    }
    else if (isToken({Token::Type::Bool, Token::Type::Int, Token::Type::Float}, iterator, end))
    {
        // TODO: parse type and fix precedence
        CastExpression* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new CastExpression(CastExpression::Kind::Functional)));

        if (isToken(Token::Type::Bool, iterator, end)) result->qualifiedType.typeDeclaration = boolTypeDeclaration;
        else if(isToken(Token::Type::Int, iterator, end)) result->qualifiedType.typeDeclaration = intTypeDeclaration;
        else if(isToken(Token::Type::Float, iterator, end)) result->qualifiedType.typeDeclaration = floatTypeDeclaration;

        ++iterator;

        expectToken(Token::Type::LeftParenthesis, iterator, end);

        ++iterator;

        result->expression = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, result);

        expectToken(Token::Type::RightParenthesis, iterator, end);

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::LeftBrace, iterator, end))
    {
        ++iterator;

        InitializerListExpression* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new InitializerListExpression()));

        QualifiedType qualifiedType;

        for (;;)
        {
            Expression* expression;
            if (!(expression = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, result)))
                return nullptr;

            if (!qualifiedType.typeDeclaration)
            {
                qualifiedType.typeDeclaration = expression->qualifiedType.typeDeclaration;
            }
            else
            {
                if (qualifiedType.typeDeclaration != expression->qualifiedType.typeDeclaration)
                {
                    // TODO: implement type narrowing
                    throw std::runtime_error("Expression type does not match previous expressions in initializer list");
                }
            }

            result->expressions.push_back(expression);

            if (!isToken(Token::Type::Comma, iterator, end))
                break;

            ++iterator;
        }

        expectToken(Token::Type::RightBrace, iterator, end);

        result->qualifiedType.typeDeclaration = getArrayTypeDeclaration(qualifiedType, static_cast<uint32_t>(result->expressions.size()));

        ++iterator;

        return result;
    }
    else if (isToken(Token::Type::Identifier, iterator, end))
    {
        std::string name = iterator->value;

        ++iterator;

        if (isToken(Token::Type::LeftParenthesis, iterator, end))
        {
            ++iterator;

            TypeDeclaration* typeDeclaration;

            if ((typeDeclaration = findTypeDeclaration(name, declarationScopes)))
            {
                TemporaryObjectExpression* result;
                constructs.push_back(std::unique_ptr<Construct>(result = new TemporaryObjectExpression()));
                result->parent = parent;
                result->qualifiedType.typeDeclaration = typeDeclaration;
                result->category = Expression::Category::Rvalue;

                std::vector<QualifiedType> parameters;

                if (isToken(Token::Type::RightParenthesis, iterator, end)) // no arguments
                {
                    ++iterator;
                }
                else
                {
                    for (;;)
                    {
                        Expression* parameter;

                        if (!(parameter = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, result)))
                            return nullptr;

                        result->parameters.push_back(parameter);
                        parameters.push_back(parameter->qualifiedType);

                        if (isToken(Token::Type::Comma, iterator, end))
                            ++iterator;
                        else
                            break;
                    }

                    expectToken(Token::Type::RightParenthesis, iterator, end);

                    ++iterator;
                }

                if (typeDeclaration->getTypeKind() != TypeDeclaration::Kind::Struct)
                    throw std::runtime_error("Expected a struct type");

                StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(typeDeclaration);

                if (!(result->constructorDeclaration = structDeclaration->findConstructorDeclaration(parameters)))
                    throw std::runtime_error("No matching constructor found");

                return result;
            }
            else
            {
                CallExpression* result;
                constructs.push_back(std::unique_ptr<Construct>(result = new CallExpression()));
                result->parent = parent;

                std::vector<QualifiedType> arguments;

                if (isToken(Token::Type::RightParenthesis, iterator, end)) // no arguments
                {
                    ++iterator;
                }
                else
                {
                    for (;;)
                    {
                        Expression* argument;

                        if (!(argument = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, result)))
                            return nullptr;

                        result->arguments.push_back(argument);
                        arguments.push_back(argument->qualifiedType);

                        if (isToken(Token::Type::Comma, iterator, end))
                            ++iterator;
                        else
                            break;
                    }

                    expectToken(Token::Type::RightParenthesis, iterator, end);

                    ++iterator;
                }

                DeclarationReferenceExpression* declRefExpression;
                constructs.push_back(std::unique_ptr<Construct>(declRefExpression = new DeclarationReferenceExpression()));
                declRefExpression->parent = result;

                FunctionDeclaration* functionDeclaration = resolveFunctionDeclaration(name, declarationScopes, arguments);

                if (!functionDeclaration)
                    throw std::runtime_error("Invalid function reference: " + name);

                declRefExpression->declaration = functionDeclaration;
                declRefExpression->qualifiedType = functionDeclaration->qualifiedType;
                declRefExpression->category = Expression::Category::Lvalue;
                result->declarationReference = declRefExpression;
                result->qualifiedType = functionDeclaration->qualifiedType;
                result->category = Expression::Category::Rvalue;

                return result;
            }
        }
        else
        {
            DeclarationReferenceExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new DeclarationReferenceExpression()));
            result->parent = parent;
            result->declaration = findDeclaration(name, declarationScopes);

            if (!result->declaration)
                throw std::runtime_error("Invalid declaration reference: " + name);

            switch (result->declaration->getDeclarationKind())
            {
                case Declaration::Kind::Type:
                {
                    TypeDeclaration* typeDeclaration = static_cast<TypeDeclaration*>(result->declaration);
                    result->qualifiedType.typeDeclaration = typeDeclaration;
                    result->category = Expression::Category::Rvalue;
                    break;
                }
                case Declaration::Kind::Variable:
                {
                    VariableDeclaration* variableDeclaration = static_cast<VariableDeclaration*>(result->declaration);
                    result->qualifiedType = variableDeclaration->qualifiedType;
                    result->category = Expression::Category::Lvalue;
                    break;
                }
                case Declaration::Kind::Parameter:
                {
                    ParameterDeclaration* parameterDeclaration = static_cast<ParameterDeclaration*>(result->declaration);
                    result->qualifiedType = parameterDeclaration->qualifiedType;
                    result->category = Expression::Category::Rvalue;
                    break;
                }
                default:
                    throw std::runtime_error("Invalid declaration reference " + name);
            }

            return result;
        }
    }
    else if (isToken(Token::Type::LeftParenthesis, iterator, end))
    {
        ++iterator;

        if (isType(iterator, end, declarationScopes))
        {
            CastExpression* result = new CastExpression(CastExpression::Kind::CStyle);
            constructs.push_back(std::unique_ptr<Construct>(result));
            result->parent = parent;

            // TODO: parse qualifiers
            result->qualifiedType.typeDeclaration = parseType(iterator, end, declarationScopes);

            expectToken(Token::Type::RightParenthesis, iterator, end);
            ++iterator;

            result->expression = parseExpression(iterator, end, declarationScopes, result);
            result->category = Expression::Category::Rvalue;

            return result;
        }
        else
        {
            ParenExpression* result;
            constructs.push_back(std::unique_ptr<Construct>(result = new ParenExpression()));
            result->parent = parent;

            if (!(result->expression = parseExpression(iterator, end, declarationScopes, result)))
                return nullptr;

            expectToken(Token::Type::RightParenthesis, iterator, end);

            ++iterator;

            result->qualifiedType = result->expression->qualifiedType;
            result->category = result->expression->category;

            return result;
        }
    }
    else if (isToken({Token::Type::ConstCast,
        Token::Type::DynamicCast,
        Token::Type::ReinterpretCast,
        Token::Type::StaticCast}, iterator, end))
    {
        CastExpression::Kind castKind;
        
        switch (iterator->type)
        {
            case Token::Type::ConstCast: castKind = CastExpression::Kind::Const; break;
            case Token::Type::DynamicCast: castKind = CastExpression::Kind::Dynamic; break;
            case Token::Type::ReinterpretCast: castKind = CastExpression::Kind::Reinterpet; break;
            case Token::Type::StaticCast: castKind = CastExpression::Kind::Static; break;
            default: throw std::runtime_error("Invalid cast");
        }
        
        CastExpression* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new CastExpression(castKind)));
        result->parent = parent;
        
        ++iterator;
        
        expectToken(Token::Type::LessThan, iterator, end);
        ++iterator;
        
        // TODO: parse qualifiers
        result->qualifiedType.typeDeclaration = parseType(iterator, end, declarationScopes);
        
        expectToken(Token::Type::GreaterThan, iterator, end);
        ++iterator;
        
        expectToken(Token::Type::LeftParenthesis, iterator, end);
        ++iterator;
        
        if (!(result->expression = parseExpression(iterator, end, declarationScopes, result)))
            return nullptr;
        
        expectToken(Token::Type::RightParenthesis, iterator, end);
        ++iterator;
        
        result->category = Expression::Category::Rvalue;
        
        return result;
    }
    else if (isToken(Token::Type::This, iterator, end))
    {
        // TODO: implement
        throw std::runtime_error("Expression \"this\" is not supported");
    }
    else
        throw std::runtime_error("Expected an expression");

    return nullptr;
}

Expression* ASTContext::parseSubscriptExpression(std::vector<Token>::const_iterator& iterator,
                                                 std::vector<Token>::const_iterator end,
                                                 std::vector<std::vector<Declaration*>>& declarationScopes,
                                                 Construct* parent)
{
    Expression* result;
    if (!(result = parsePrimaryExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken(Token::Type::LeftBracket, iterator, end))
    {
        ++iterator;

        ArraySubscriptExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new ArraySubscriptExpression()));
        expression->parent = parent;
        expression->expression = result;

        if (!result->qualifiedType.typeDeclaration)
            throw std::runtime_error("Subscript expression with a void type");

        if (result->qualifiedType.typeDeclaration->getTypeKind() != TypeDeclaration::Kind::Array)
            throw std::runtime_error("Subscript value is not an array");

        if (!(expression->subscript = parseExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        if (!expression->subscript->qualifiedType.typeDeclaration ||
            expression->subscript->qualifiedType.typeDeclaration->getTypeKind() != TypeDeclaration::Kind::Scalar)
            throw std::runtime_error("Subscript is not an integer");

        ScalarTypeDeclaration* scalarType = static_cast<ScalarTypeDeclaration*>(expression->subscript->qualifiedType.typeDeclaration);

        if (scalarType->getScalarTypeKind() != ScalarTypeDeclaration::Kind::Boolean &&
            scalarType->getScalarTypeKind() != ScalarTypeDeclaration::Kind::Integer)
            throw std::runtime_error("Subscript is not an integer");

        if (scalarType->getScalarTypeKind() != ScalarTypeDeclaration::Kind::Integer)
            expression->subscript = addImplicitCast(expression->subscript,
                                                    intTypeDeclaration,
                                                    expression->subscript->category);

        expectToken(Token::Type::RightBracket, iterator, end);

        ++iterator;

        ArrayTypeDeclaration* arrayTypeDeclaration = static_cast<ArrayTypeDeclaration*>(result->qualifiedType.typeDeclaration);

        expression->qualifiedType = arrayTypeDeclaration->elementType;
        expression->category = Expression::Category::Lvalue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseMemberExpression(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent)
{
    Expression* result;
    if (!(result = parseSubscriptExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::Dot, Token::Type::Arrow}, iterator, end))
    {
        if (isToken(Token::Type::Arrow, iterator, end))
            throw std::runtime_error("Pointer member access is not supported");

        ++iterator;

        MemberExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new MemberExpression()));
        expression->parent = parent;
        expression->expression = result;

        if (!result->qualifiedType.typeDeclaration)
            throw std::runtime_error("Expression has a void type");

        if (result->qualifiedType.typeDeclaration->getTypeKind() != TypeDeclaration::Kind::Struct)
        {
            throw std::runtime_error(result->qualifiedType.typeDeclaration->name + " is not a structure");
            return nullptr;
        }

        StructDeclaration* structDeclaration = static_cast<StructDeclaration*>(result->qualifiedType.typeDeclaration);

        expectToken(Token::Type::Identifier, iterator, end);

        Declaration* memberDeclaration = structDeclaration->findMemberDeclaration(iterator->value);

        if (!memberDeclaration)
        {
            throw std::runtime_error("Structure " + structDeclaration->name +  " has no member " + iterator->value);
            return nullptr;
        }

        if (memberDeclaration->getDeclarationKind() != Declaration::Kind::Field)
            throw std::runtime_error(iterator->value + " is not a field");

        expression->fieldDeclaration = static_cast<FieldDeclaration*>(memberDeclaration);

        ++iterator;

        expression->qualifiedType = expression->fieldDeclaration->qualifiedType;
        if (result->qualifiedType.isConst) expression->qualifiedType.isConst = true;
        expression->category = Expression::Category::Lvalue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseSignExpression(std::vector<Token>::const_iterator& iterator,
                                            std::vector<Token>::const_iterator end,
                                            std::vector<std::vector<Declaration*>>& declarationScopes,
                                            Construct* parent)
{
    if (isToken({Token::Type::Plus, Token::Type::Minus}, iterator, end))
    {
        UnaryOperatorExpression* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new UnaryOperatorExpression()));
        result->parent = parent;

        Operator op;

        if (iterator->type == Token::Type::Plus)
            op = Operator::Positive;
        else if (iterator->type == Token::Type::Minus)
            op = Operator::Negative;
        else
            throw std::runtime_error("Invalid operator");

        ++iterator;

        if (!(result->expression = parseMemberExpression(iterator, end, declarationScopes, result)))
            return nullptr;

        result->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes, {result->expression->qualifiedType});

        if (result->expression->qualifiedType.typeDeclaration == boolTypeDeclaration)
            result->expression = addImplicitCast(result->expression,
                                                 intTypeDeclaration,
                                                 result->expression->category);

        result->qualifiedType = result->operatorDeclaration->qualifiedType;
        result->category = Expression::Category::Rvalue;

        return result;
    }
    else
    {
        Expression* result;
        if (!(result = parseMemberExpression(iterator, end, declarationScopes, parent)))
            return nullptr;

        return result;
    }
}

Expression* ASTContext::parseNotExpression(std::vector<Token>::const_iterator& iterator,
                                           std::vector<Token>::const_iterator end,
                                           std::vector<std::vector<Declaration*>>& declarationScopes,
                                           Construct* parent)
{
    if (isToken(Token::Type::Not, iterator, end))
    {
        UnaryOperatorExpression* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new UnaryOperatorExpression()));
        result->parent = parent;

        Operator op = Operator::Negation;

        ++iterator;

        if (!(result->expression = parseExpression(iterator, end, declarationScopes, result)))
            return nullptr;

        result->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes, {result->expression->qualifiedType});

        if (result->expression->qualifiedType.typeDeclaration != boolTypeDeclaration)
            result->expression = addImplicitCast(result->expression,
                                                 boolTypeDeclaration,
                                                 result->expression->category);

        result->qualifiedType = result->operatorDeclaration->qualifiedType;
        result->category = Expression::Category::Rvalue;

        return result;
    }
    else
    {
        Expression* result;
        if (!(result = parseSignExpression(iterator, end, declarationScopes, parent)))
            return nullptr;
        return result;
    }
}

Expression* ASTContext::parseSizeofExpression(std::vector<Token>::const_iterator& iterator,
                                              std::vector<Token>::const_iterator end,
                                              std::vector<std::vector<Declaration*>>& declarationScopes,
                                              Construct* parent)
{
    if (isToken(Token::Type::Sizeof, iterator, end))
    {
        SizeofExpression* result;
        constructs.push_back(std::unique_ptr<Construct>(result = new SizeofExpression()));
        result->parent = parent;
        
        ++iterator;
        
        expectToken(Token::Type::LeftParenthesis, iterator, end);
        ++iterator;
        
        if (isType(iterator, end, declarationScopes))
        {
            if (!(result->type = parseType(iterator, end, declarationScopes)))
                return nullptr;
        }
        else
        {
            if (!(result->expression = parseExpression(iterator, end, declarationScopes, result)))
                return nullptr;
        }
        
        expectToken(Token::Type::RightParenthesis, iterator, end);
        ++iterator;
        
        result->qualifiedType.typeDeclaration = unsignedIntTypeDeclaration;
        result->category = Expression::Category::Rvalue;
        
        return result;
    }
    else
    {
        Expression* result;
        if (!(result = parseNotExpression(iterator, end, declarationScopes, parent)))
            return nullptr;
        return result;
    }
}

Expression* ASTContext::parseMultiplicationExpression(std::vector<Token>::const_iterator& iterator,
                                                      std::vector<Token>::const_iterator end,
                                                      std::vector<std::vector<Declaration*>>& declarationScopes,
                                                      Construct* parent)
{
    Expression* result;
    if (!(result = parseSizeofExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::Multiply, Token::Type::Divide}, iterator, end))
    {
        BinaryOperatorExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression()));
        expression->parent = parent;

        Operator op;

        if (iterator->type == Token::Type::Multiply)
            op = Operator::Multiplication;
        else if (iterator->type == Token::Type::Divide)
            op = Operator::Division;
        else
            throw std::runtime_error("Invalid operator");

        expression->leftExpression = result;

        ++iterator;

        if (!(expression->rightExpression = parseSizeofExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->category = Expression::Category::Rvalue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseAdditionExpression(std::vector<Token>::const_iterator& iterator,
                                                std::vector<Token>::const_iterator end,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                Construct* parent)
{
    Expression* result;
    if (!(result = parseMultiplicationExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::Plus, Token::Type::Minus}, iterator, end))
    {
        BinaryOperatorExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression()));
        expression->parent = parent;

        Operator op;

        if (iterator->type == Token::Type::Plus)
            op = Operator::Addition;
        else if (iterator->type == Token::Type::Minus)
            op = Operator::Subtraction;
        else
            throw std::runtime_error("Invalid operator");

        expression->leftExpression = result;

        ++iterator;

        if (!(expression->rightExpression = parseMultiplicationExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->category = Expression::Category::Rvalue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseLessThanExpression(std::vector<Token>::const_iterator& iterator,
                                                std::vector<Token>::const_iterator end,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                Construct* parent)
{
    Expression* result;
    if (!(result = parseAdditionExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::LessThan, Token::Type::LessThanEqual}, iterator, end))
    {
        BinaryOperatorExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression()));
        expression->parent = parent;

        Operator op;

        if (iterator->type == Token::Type::LessThan)
            op = Operator::LessThan;
        else if (iterator->type == Token::Type::LessThanEqual)
            op = Operator::LessThanEqual;
        else
            throw std::runtime_error("Invalid operator");

        ++iterator;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseAdditionExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->category = Expression::Category::Rvalue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseGreaterThanExpression(std::vector<Token>::const_iterator& iterator,
                                                   std::vector<Token>::const_iterator end,
                                                   std::vector<std::vector<Declaration*>>& declarationScopes,
                                                   Construct* parent)
{
    Expression* result;
    if (!(result = parseLessThanExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::GreaterThan, Token::Type::GreaterThanEqual}, iterator, end))
    {
        BinaryOperatorExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression()));
        expression->parent = parent;

        Operator op;

        if (iterator->type == Token::Type::GreaterThan)
            op = Operator::GreaterThan;
        else if (iterator->type == Token::Type::GreaterThanEqual)
            op = Operator::GraterThanEqual;
        else
            throw std::runtime_error("Invalid operator");

        ++iterator;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseLessThanExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->category = Expression::Category::Rvalue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseEqualityExpression(std::vector<Token>::const_iterator& iterator,
                                                std::vector<Token>::const_iterator end,
                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                Construct* parent)
{
    Expression* result;
    if (!(result = parseGreaterThanExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::Equal, Token::Type::NotEq}, iterator, end))
    {
        BinaryOperatorExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression()));
        expression->parent = parent;

        Operator op;

        if (iterator->type == Token::Type::Equal)
            op = Operator::Equality;
        else if (iterator->type == Token::Type::NotEq)
            op = Operator::Inequality;
        else
            throw std::runtime_error("Invalid operator");

        ++iterator;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseGreaterThanExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->category = Expression::Category::Rvalue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseLogicalAndExpression(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  Construct* parent)
{
    Expression* result;
    if (!(result = parseEqualityExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken(Token::Type::And, iterator, end))
    {
        ++iterator;

        BinaryOperatorExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression()));
        expression->parent = parent;

        Operator op = Operator::And;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseEqualityExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        // TODO: check if both sides ar scalar
        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->category = Expression::Category::Rvalue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseLogicalOrExpression(std::vector<Token>::const_iterator& iterator,
                                                 std::vector<Token>::const_iterator end,
                                                 std::vector<std::vector<Declaration*>>& declarationScopes,
                                                 Construct* parent)
{
    Expression* result;
    if (!(result = parseLogicalAndExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken(Token::Type::Or, iterator, end))
    {
        ++iterator;

        BinaryOperatorExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression()));
        expression->parent = parent;

        Operator op = Operator::Or;

        expression->leftExpression = result;

        if (!(expression->rightExpression = parseLogicalAndExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        // TODO: check if both sides ar scalar
        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->category = Expression::Category::Rvalue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseTernaryExpression(std::vector<Token>::const_iterator& iterator,
                                               std::vector<Token>::const_iterator end,
                                               std::vector<std::vector<Declaration*>>& declarationScopes,
                                               Construct* parent)
{
    Expression* result;
    if (!(result = parseLogicalOrExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken(Token::Type::Conditional, iterator, end))
    {
        ++iterator;

        TernaryOperatorExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new TernaryOperatorExpression()));
        expression->parent = parent;
        expression->condition = result;

        if (!expression->condition->qualifiedType.typeDeclaration)
            throw std::runtime_error("Ternary expression with a void condition");

        if (!(expression->leftExpression = parseTernaryExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        expectToken(Token::Type::Colon, iterator, end);

        ++iterator;

        if (!(expression->rightExpression = parseTernaryExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        // TODO: fix this
        expression->qualifiedType = expression->leftExpression->qualifiedType;
        expression->category = (expression->leftExpression->category == Expression::Category::Lvalue &&
                                expression->rightExpression->category == Expression::Category::Lvalue) ?
                                Expression::Category::Lvalue : Expression::Category::Rvalue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                                  std::vector<Token>::const_iterator end,
                                                  std::vector<std::vector<Declaration*>>& declarationScopes,
                                                  Construct* parent)
{
    Expression* result;
    if (!(result = parseTernaryExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken(Token::Type::Assignment, iterator, end))
    {
        ++iterator;

        BinaryOperatorExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression()));
        expression->parent = parent;
        expression->leftExpression = result;

        if (expression->leftExpression->qualifiedType.isConst)
            throw std::runtime_error("Cannot assign to const variable");

        if (expression->leftExpression->category != Expression::Category::Lvalue)
            throw std::runtime_error("Expression is not assignable");

        if (!(expression->rightExpression = parseTernaryExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(Operator::Assignment,
                                                                     declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->category = Expression::Category::Lvalue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseAdditionAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                                          std::vector<Token>::const_iterator end,
                                                          std::vector<std::vector<Declaration*>>& declarationScopes,
                                                          Construct* parent)
{
    Expression* result;
    if (!(result = parseAssignmentExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::PlusAssignment, Token::Type::MinusAssignment}, iterator, end))
    {
        BinaryOperatorExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression()));
        expression->parent = parent;

        Operator op;

        if (iterator->type == Token::Type::PlusAssignment)
            op = Operator::AdditionAssignment;
        else if (iterator->type == Token::Type::MinusAssignment)
            op = Operator::SubtractAssignment;
        else
            throw std::runtime_error("Invalid operator");

        ++iterator;

        expression->leftExpression = result;

        if (expression->leftExpression->qualifiedType.isConst)
            throw std::runtime_error("Cannot assign to const variable");

        if (expression->leftExpression->category != Expression::Category::Lvalue)
            throw std::runtime_error("Expression is not assignable");

        if (!(expression->rightExpression = parseAssignmentExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->category = Expression::Category::Lvalue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseMultiplicationAssignmentExpression(std::vector<Token>::const_iterator& iterator,
                                                                std::vector<Token>::const_iterator end,
                                                                std::vector<std::vector<Declaration*>>& declarationScopes,
                                                                Construct* parent)
{
    Expression* result;
    if (!(result = parseAdditionAssignmentExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken({Token::Type::Multiply, Token::Type::DivideAssignment}, iterator, end))
    {
        BinaryOperatorExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression()));
        expression->parent = parent;

        Operator op;

        if (iterator->type == Token::Type::Multiply)
            op = Operator::MultiplicationAssignment;
        else if (iterator->type == Token::Type::DivideAssignment)
            op = Operator::DivisionAssignment;
        else
            throw std::runtime_error("Invalid operator");

        ++iterator;

        expression->leftExpression = result;

        if (expression->leftExpression->qualifiedType.isConst)
            throw std::runtime_error("Cannot assign to const variable");

        if (expression->leftExpression->category != Expression::Category::Lvalue)
            throw std::runtime_error("Expression is not assignable");

        std::unique_ptr<Construct> right;
        if (!(expression->rightExpression = parseAdditionAssignmentExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(op, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->category = Expression::Category::Lvalue;

        result->parent = expression;
        result = expression;
    }

    return result;
}

Expression* ASTContext::parseCommaExpression(std::vector<Token>::const_iterator& iterator,
                                             std::vector<Token>::const_iterator end,
                                             std::vector<std::vector<Declaration*>>& declarationScopes,
                                             Construct* parent)
{
    Expression* result;
    if (!(result = parseMultiplicationAssignmentExpression(iterator, end, declarationScopes, parent)))
        return nullptr;

    while (isToken(Token::Type::Comma, iterator, end))
    {
        BinaryOperatorExpression* expression;
        constructs.push_back(std::unique_ptr<Construct>(expression = new BinaryOperatorExpression()));
        expression->parent = parent;
        expression->leftExpression = result;

        ++iterator;

        std::unique_ptr<Construct> right;
        if (!(expression->rightExpression = parseAdditionAssignmentExpression(iterator, end, declarationScopes, expression)))
            return nullptr;

        expression->operatorDeclaration = resolveOperatorDeclaration(Operator::Comma, declarationScopes,
                                                                     {expression->leftExpression->qualifiedType, expression->rightExpression->qualifiedType});

        expression->qualifiedType = expression->operatorDeclaration->qualifiedType;
        expression->category = expression->rightExpression->category;

        result->parent = expression;
        result = expression;
    }

    return result;
}

static std::string toString(Construct::Kind kind)
{
    switch (kind)
    {
        case Construct::Kind::Declaration: return "Declaration";
        case Construct::Kind::Statement: return "Statement";
        case Construct::Kind::Expression: return "Expression";
        default: return "Unknown";
    }
}

static std::string toString(Statement::Kind kind)
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
        default: return "Unknown";
    }
}

static std::string toString(Expression::Kind kind)
{
    switch (kind)
    {
        case Expression::Kind::Call: return "Call";
        case Expression::Kind::Literal: return "Literal";
        case Expression::Kind::DeclarationReference: return "DeclarationReference";
        case Expression::Kind::Paren: return "Paren";
        case Expression::Kind::Member: return "Member";
        case Expression::Kind::ArraySubscript: return "ArraySubscript";
        case Expression::Kind::UnaryOperator: return "UnaryOperator";
        case Expression::Kind::BinaryOperator: return "BinaryOperator";
        case Expression::Kind::TernaryOperator: return "TernaryOperator";
        case Expression::Kind::TemporaryObject: return "TemporaryObject";
        case Expression::Kind::InitializerList: return "InitializerList";
        case Expression::Kind::Cast: return "Cast";
        case Expression::Kind::Sizeof: return "Sizeof";
        default: return "Unknown";
    }
}

static std::string toString(Declaration::Kind kind)
{
    switch (kind)
    {
        case Declaration::Kind::Empty: return "Empty";
        case Declaration::Kind::Type: return "Type";
        case Declaration::Kind::Field: return "Field";
        case Declaration::Kind::Callable: return "Callable";
        case Declaration::Kind::Variable: return "Variable";
        case Declaration::Kind::Parameter: return "Parameter";
        default: return "Unknown";
    }
}

static std::string toString(TypeDeclaration::Kind kind)
{
    switch (kind)
    {
        case TypeDeclaration::Kind::Array: return "Array";
        case TypeDeclaration::Kind::Scalar: return "Scalar";
        case TypeDeclaration::Kind::Struct: return "Struct";
        //case TypeDeclaration::Kind::TypeDefinition: return "TypeDefinition";
        default: return "Unknown";
    }
}

static std::string toString(ScalarTypeDeclaration::Kind kind)
{
    switch (kind)
    {
        case ScalarTypeDeclaration::Kind::Boolean: return "Boolean";
        case ScalarTypeDeclaration::Kind::Integer: return "Integer";
        case ScalarTypeDeclaration::Kind::FloatingPoint: return "FloatingPoint";
        default: return "Unknown";
    }
}

static std::string toString(CallableDeclaration::Kind kind)
{
    switch (kind)
    {
        case CallableDeclaration::Kind::Function: return "Function";
        case CallableDeclaration::Kind::Constructor: return "Constructor";
        case CallableDeclaration::Kind::Method: return "Method";
        case CallableDeclaration::Kind::Operator: return "Operator";
        default: return "Unknown";
    }
}

static std::string toString(LiteralExpression::Kind kind)
{
    switch (kind)
    {
        case LiteralExpression::Kind::Boolean: return "Boolean";
        case LiteralExpression::Kind::Integer: return "Integer";
        case LiteralExpression::Kind::FloatingPoint: return "FloatingPoint";
        case LiteralExpression::Kind::String: return "String";
        default: return "Unknown";
    }
}

static std::string toString(CastExpression::Kind kind)
{
    switch (kind)
    {
        case CastExpression::Kind::Implicit: return "Implicit";
        case CastExpression::Kind::CStyle: return "CStyle";
        case CastExpression::Kind::Functional: return "Functional";
        case CastExpression::Kind::Dynamic: return "Dynamic";
        case CastExpression::Kind::Reinterpet: return "Reinterpet";
        case CastExpression::Kind::Static: return "Static";
        default: return "Unknown";
    }
}

void ASTContext::dump() const
{
    for (Declaration* declaration : declarations)
        dumpConstruct(declaration);
}

static std::string getPrintableTypeName(const QualifiedType& qualifiedType)
{
    std::string result;

    if (qualifiedType.isVolatile) result += "volatile ";
    if (qualifiedType.isConst) result += "const ";

    if (!qualifiedType.typeDeclaration)
    {
        result += "void";
    }
    else
    {
        TypeDeclaration* typeDeclaration = qualifiedType.typeDeclaration;
        while (typeDeclaration->getTypeKind() == TypeDeclaration::Kind::Array)
        {
            ArrayTypeDeclaration* arrayTypeDeclaration = static_cast<ArrayTypeDeclaration*>(typeDeclaration);

            result = "[" + std::to_string(arrayTypeDeclaration->size) + "]" + result;

            typeDeclaration = arrayTypeDeclaration->elementType.typeDeclaration;
        }

        result = typeDeclaration->name + result;
    }

    return result;
}

void ASTContext::dumpDeclaration(const Declaration* declaration, std::string indent) const
{
    std::cout << " " << toString(declaration->getDeclarationKind());

    switch (declaration->getDeclarationKind())
    {
        case Declaration::Kind::Empty:
        {
            std::cout << std::endl;
            break;
        }

        case Declaration::Kind::Type:
        {
            const TypeDeclaration* typeDeclaration = static_cast<const TypeDeclaration*>(declaration);

            std::cout << " " << toString(typeDeclaration->getTypeKind());

            switch (typeDeclaration->getTypeKind())
            {
                case TypeDeclaration::Kind::Array: // array types can not be declared in code
                {
                    break;
                }

                case TypeDeclaration::Kind::Struct:
                {
                    const StructDeclaration* structDeclaration = static_cast<const StructDeclaration*>(typeDeclaration);
                    std::cout << ", name: " << structDeclaration->name;

                    if (structDeclaration->previousDeclaration)
                        std::cout << ", previous declaration: " << structDeclaration->previousDeclaration;

                    if (structDeclaration->definition)
                        std::cout << ", definition: " << structDeclaration->definition;

                    std::cout << std::endl;

                    for (const Declaration* memberDeclaration : structDeclaration->memberDeclarations)
                        dumpConstruct(memberDeclaration, indent + "  ");

                    break;
                }

                case TypeDeclaration::Kind::Scalar:
                {
                    const ScalarTypeDeclaration* scalarTypeDeclaration = static_cast<const ScalarTypeDeclaration*>(typeDeclaration);
                    std::cout << ", name: " << scalarTypeDeclaration->name << ", scalar type kind: " << toString(scalarTypeDeclaration->getScalarTypeKind());
                    break;
                }
            }
            break;
        }

        case Declaration::Kind::Field:
        {
            const FieldDeclaration* fieldDeclaration = static_cast<const FieldDeclaration*>(declaration);

            std::cout << ", name: " << fieldDeclaration->name << ", type: " << getPrintableTypeName(fieldDeclaration->qualifiedType);

            if (fieldDeclaration->semantic != Semantic::None)
                std::cout << ", semantic: " << toString(fieldDeclaration->semantic);

            std::cout << std::endl;
            break;
        }

        case Declaration::Kind::Callable:
        {
            const CallableDeclaration* callableDeclaration = static_cast<const CallableDeclaration*>(declaration);

            std::cout << ", callable kind: " << toString(callableDeclaration->getCallableDeclarationKind()) << ", name: " << callableDeclaration->name << ", result type: " << getPrintableTypeName(callableDeclaration->qualifiedType);

            if (callableDeclaration->getCallableDeclarationKind() == CallableDeclaration::Kind::Function)
            {
                const FunctionDeclaration* functionDeclaration = static_cast<const FunctionDeclaration*>(callableDeclaration);

                if (functionDeclaration->isStatic) std::cout << " static";
                if (functionDeclaration->isInline) std::cout << " inline";

                if (functionDeclaration->isProgram)
                    std::cout << ", program: " << toString(functionDeclaration->program);
            }

            if (callableDeclaration->previousDeclaration)
                std::cout << ", previous declaration: " << callableDeclaration->previousDeclaration;

            if (callableDeclaration->definition)
                std::cout << ", definition: " << callableDeclaration->definition;

            std::cout << std::endl;

            for (ParameterDeclaration* parameter : callableDeclaration->parameterDeclarations)
                dumpConstruct(parameter, indent + "  ");

            if (callableDeclaration->body)
                dumpConstruct(callableDeclaration->body, indent + "  ");

            break;
        }

        case Declaration::Kind::Variable:
        {
            const VariableDeclaration* variableDeclaration = static_cast<const VariableDeclaration*>(declaration);
            std::cout << ", name: " << variableDeclaration->name << ", type: " << getPrintableTypeName(variableDeclaration->qualifiedType) << std::endl;

            if (variableDeclaration->initialization)
            {
                dumpConstruct(variableDeclaration->initialization, indent + "  ");
            }

            break;
        }

        case Declaration::Kind::Parameter:
        {
            const ParameterDeclaration* parameterDeclaration = static_cast<const ParameterDeclaration*>(declaration);
            std::cout << ", name: " << parameterDeclaration->name << ", type: " << getPrintableTypeName(parameterDeclaration->qualifiedType) << std::endl;
            break;
        }

        default:
            break;
    }
}

void ASTContext::dumpStatement(const Statement* statement, std::string indent) const
{
    std::cout << " " << toString(statement->getStatementKind());

    switch (statement->getStatementKind())
    {
        case Statement::Kind::Empty:
        {
            std::cout << std::endl;
            break;
        }

        case Statement::Kind::Expression:
        {
            const ExpressionStatement* expressionStatement = static_cast<const ExpressionStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(expressionStatement->expression, indent + "  ");
            break;
        }

        case Statement::Kind::Declaration:
        {
            const DeclarationStatement* declarationStatement = static_cast<const DeclarationStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(declarationStatement->declaration, indent + "  ");
            break;
        }

        case Statement::Kind::Compound:
        {
            const CompoundStatement* compoundStatement = static_cast<const CompoundStatement*>(statement);

            std::cout << std::endl;

            for (Statement* subSstatement : compoundStatement->statements)
                dumpConstruct(subSstatement, indent + "  ");

            break;
        }

        case Statement::Kind::If:
        {
            const IfStatement* ifStatement = static_cast<const IfStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(ifStatement->condition, indent + "  ");
            dumpConstruct(ifStatement->body, indent + "  ");
            if (ifStatement->elseBody) dumpConstruct(ifStatement->elseBody, indent + "  ");
            break;
        }

        case Statement::Kind::For:
        {
            const ForStatement* forStatement = static_cast<const ForStatement*>(statement);

            std::cout << std::endl;

            if (forStatement->initialization) dumpConstruct(forStatement->initialization, indent + "  ");
            if (forStatement->condition) dumpConstruct(forStatement->condition, indent + "  ");
            if (forStatement->increment) dumpConstruct(forStatement->increment, indent + "  ");
            dumpConstruct(forStatement->body, indent + "  ");
            break;
        }

        case Statement::Kind::Switch:
        {
            const SwitchStatement* switchStatement = static_cast<const SwitchStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(switchStatement->condition, indent + "  ");
            dumpConstruct(switchStatement->body, indent + "  ");
            break;
        }

        case Statement::Kind::Case:
        {
            const CaseStatement* caseStatement = static_cast<const CaseStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(caseStatement->condition, indent + "  ");
            dumpConstruct(caseStatement->body, indent + "  ");
            break;
        }

        case Statement::Kind::Default:
        {
            const DefaultStatement* defaultStatement = static_cast<const DefaultStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(defaultStatement->body, indent + "  ");
            break;
        }

        case Statement::Kind::While:
        {
            const WhileStatement* whileStatement = static_cast<const WhileStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(whileStatement->condition, indent + "  ");
            dumpConstruct(whileStatement->body, indent + "  ");
            break;
        }

        case Statement::Kind::Do:
        {
            const DoStatement* doStatement = static_cast<const DoStatement*>(statement);

            std::cout << std::endl;

            dumpConstruct(doStatement->body, indent + "  ");
            dumpConstruct(doStatement->condition, indent + "  ");
            break;
        }

        case Statement::Kind::Break:
        {
            std::cout << std::endl;
            break;
        }

        case Statement::Kind::Continue:
        {
            std::cout << std::endl;
            break;
        }

        case Statement::Kind::Return:
        {
            const ReturnStatement* returnStatement = static_cast<const ReturnStatement*>(statement);

            std::cout << std::endl;

            if (returnStatement->result)
            {
                dumpConstruct(returnStatement->result, indent + "  ");
            }
            break;
        }

        default:
            break;
    }
}

static std::string toString(Expression::Category category)
{
    switch (category)
    {
        case Expression::Category::Lvalue: return "Lvalue";
        case Expression::Category::Rvalue: return "Rvalue";
        default: return "Unknown";
    }
}

void ASTContext::dumpExpression(const Expression* expression, std::string indent) const
{
    std::cout << " " << toString(expression->getExpressionKind()) << ", category: " << toString(expression->category);

    switch (expression->getExpressionKind())
    {
        case Expression::Kind::Call:
        {
            const CallExpression* callExpression = static_cast<const CallExpression*>(expression);

            std::cout << std::endl;

            dumpConstruct(callExpression->declarationReference, indent + "  ");

            for (Expression* argument : callExpression->arguments)
                dumpConstruct(argument, indent + "  ");

            break;
        }

        case Expression::Kind::Literal:
        {
            const LiteralExpression* literalExpression = static_cast<const LiteralExpression*>(expression);

            std::cout << ", literal kind: " << toString(literalExpression->getLiteralKind()) << ", value: ";

            switch (literalExpression->getLiteralKind())
            {
                case LiteralExpression::Kind::Boolean:
                {
                    const BooleanLiteralExpression* booleanLiteralExpression = static_cast<const BooleanLiteralExpression*>(literalExpression);
                    std::cout << (booleanLiteralExpression->value ? "true" : "false");
                    break;
                }
                case LiteralExpression::Kind::Integer:
                {
                    const IntegerLiteralExpression* integerLiteralExpression = static_cast<const IntegerLiteralExpression*>(literalExpression);
                    std::cout << integerLiteralExpression->value;
                    break;
                }
                case LiteralExpression::Kind::FloatingPoint:
                {
                    const FloatingPointLiteralExpression* floatingPointLiteralExpression = static_cast<const FloatingPointLiteralExpression*>(literalExpression);
                    std::cout << floatingPointLiteralExpression->value;
                    break;
                }
                case LiteralExpression::Kind::String:
                {
                    const StringLiteralExpression* stringLiteralExpression = static_cast<const StringLiteralExpression*>(literalExpression);
                    std::cout << stringLiteralExpression->value;
                    break;
                }
            }

            std::cout << std::endl;
            break;
        }

        case Expression::Kind::DeclarationReference:
        {
            const DeclarationReferenceExpression* declarationReferenceExpression = static_cast<const DeclarationReferenceExpression*>(expression);

            std::cout << " " << declarationReferenceExpression->declaration->name << " " << declarationReferenceExpression->declaration;

            std::cout << std::endl;
            break;
        }

        case Expression::Kind::Paren:
        {
            const ParenExpression* parenExpression = static_cast<const ParenExpression*>(expression);

            std::cout << std::endl;

            dumpConstruct(parenExpression->expression, indent + "  ");
            break;
        }

        case Expression::Kind::Member:
        {
            const MemberExpression* memberExpression = static_cast<const MemberExpression*>(expression);

            std::cout << ", field: " << memberExpression->fieldDeclaration->name << std::endl;

            dumpConstruct(memberExpression->expression, indent + "  ");
            break;
        }

        case Expression::Kind::ArraySubscript:
        {
            const ArraySubscriptExpression* arraySubscriptExpression = static_cast<const ArraySubscriptExpression*>(expression);

            std::cout << std::endl;

            dumpConstruct(arraySubscriptExpression->expression, indent + "  ");
            dumpConstruct(arraySubscriptExpression->subscript, indent + "  ");
            break;
        }

        case Expression::Kind::UnaryOperator:
        {
            const UnaryOperatorExpression* unaryOperatorExpression = static_cast<const UnaryOperatorExpression*>(expression);

            std::cout <<", operator: " << toString(unaryOperatorExpression->operatorDeclaration->op) << std::endl;

            dumpConstruct(unaryOperatorExpression->expression, indent + "  ");
            break;
        }

        case Expression::Kind::BinaryOperator:
        {
            const BinaryOperatorExpression* binaryOperatorExpression = static_cast<const BinaryOperatorExpression*>(expression);

            std::cout << ", operator: " << toString(binaryOperatorExpression->operatorDeclaration->op) << std::endl;

            dumpConstruct(binaryOperatorExpression->leftExpression, indent + "  ");
            dumpConstruct(binaryOperatorExpression->rightExpression, indent + "  ");
            break;
        }

        case Expression::Kind::TernaryOperator:
        {
            const TernaryOperatorExpression* ternaryOperatorExpression = static_cast<const TernaryOperatorExpression*>(expression);

            std::cout << std::endl;

            dumpConstruct(ternaryOperatorExpression->condition, indent + "  ");
            dumpConstruct(ternaryOperatorExpression->leftExpression, indent + "  ");
            dumpConstruct(ternaryOperatorExpression->rightExpression, indent + "  ");
            break;
        }

        case Expression::Kind::TemporaryObject:
        {
            const TemporaryObjectExpression* temporaryObjectExpression = static_cast<const TemporaryObjectExpression*>(expression);

            const TypeDeclaration* typeDeclaration = static_cast<const TypeDeclaration*>(temporaryObjectExpression->constructorDeclaration->parent);

            std::cout << " " << typeDeclaration->name << std::endl;

            for (Expression* parameter : temporaryObjectExpression->parameters)
                dumpConstruct(parameter, indent + "  ");

            break;
        }

        case Expression::Kind::InitializerList:
        {
            const InitializerListExpression* initializerListExpression = static_cast<const InitializerListExpression*>(expression);

            std::cout << std::endl;

            for (Expression* subExpression : initializerListExpression->expressions)
                dumpConstruct(subExpression, indent + "  ");

            break;
        }

        case Expression::Kind::Cast:
        {
            const CastExpression* castExpression = static_cast<const CastExpression*>(expression);

            std::cout << ", cast kind: " << toString(castExpression->getCastKind()) <<
                ", type: " << castExpression->qualifiedType.typeDeclaration->name << std::endl;

            dumpConstruct(castExpression->expression, indent + "  ");

            break;
        }
        case Expression::Kind::Sizeof:
        {
            const SizeofExpression* sizeofExpression = static_cast<const SizeofExpression*>(expression);

            std::cout << std::endl;

            if (sizeofExpression->expression)
                dumpConstruct(sizeofExpression->expression, indent + "  ");
            else if (sizeofExpression->type)
                dumpConstruct(sizeofExpression->type, indent + "  ");
            break;
        }
    }
}

void ASTContext::dumpConstruct(const Construct* construct, std::string indent) const
{
    std::cout << indent << construct;
    if (construct->parent) std::cout << ", parent: " << construct->parent;
    std::cout << " " << toString(construct->getKind());

    switch (construct->getKind())
    {
        case Construct::Kind::Declaration:
        {
            const Declaration* declaration = static_cast<const Declaration*>(construct);
            dumpDeclaration(declaration, indent);
            break;
        }

        case Construct::Kind::Statement:
        {
            const Statement* statement = static_cast<const Statement*>(construct);
            dumpStatement(statement, indent);
            break;
        }

        case Construct::Kind::Expression:
        {
            const Expression* expression = static_cast<const Expression*>(construct);
            dumpExpression(expression, indent);
            break;
        }
    }
}
