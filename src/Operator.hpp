//
//  OSL
//

#pragma once

enum class Operator
{
    NONE,
    NEGATION, // !
    POSITIVE, // +
    NEGATIVE, // -
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
