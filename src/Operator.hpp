//
//  OSL
//

#ifndef OPERATOR_HPP
#define OPERATOR_HPP

enum class Operator
{
    NONE,
    NEGATION, // !
    POSITIVE, // +
    NEGATIVE, // -
    PREFIX_INCREMENT, // ++
    PREFIX_DECREMENT, // --
    POSTFIX_INCREMENT, // ++
    POSTFIX_DECREMENT, // --
    ADDITION, // +
    SUBTRACTION, // -
    MULTIPLICATION, // *
    DIVISION, // /
    ADDITION_ASSIGNMENT, // +=
    SUBTRACTION_ASSIGNMENT, // -=
    MULTIPLICATION_ASSIGNMENT, // *=
    DIVISION_ASSIGNMENT, // /=
    LESS_THAN, // <
    LESS_THAN_EQUAL, // <=
    GREATER_THAN, // >
    GREATER_THAN_EQUAL, // >=
    EQUALITY, // ==
    INEQUALITY, // !=
    ASSIGNMENT, // =
    OR, // ||
    AND, // &&
    COMMA, // ,
    CONDITIONAL // ?:
};

#endif // OPERATOR_HPP
