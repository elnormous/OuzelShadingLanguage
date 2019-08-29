//
//  OSL
//

#ifndef OPERATOR_HPP
#define OPERATOR_HPP

enum class Operator
{
    Negation, // !
    Positive, // +
    Negative, // -
    PrefixIncrement, // ++
    PrefixDecrement, // --
    PostfixIncrement, // ++
    PostfixDecrement, // --
    Addition, // +
    Subtraction, // -
    Multiplication, // *
    Division, // /
    AdditionAssignment, // +=
    SubtractAssignment, // -=
    MultiplicationAssignment, // *=
    DivisionAssignment, // /=
    LessThan, // <
    LessThanEqual, // <=
    GreaterThan, // >
    GraterThanEqual, // >=
    Equality, // ==
    Inequality, // !=
    Assignment, // =
    Or, // ||
    And, // &&
    Comma, // ,
    Conditional // ?:
};

#endif // OPERATOR_HPP
