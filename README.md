# Ouzel Shading Language

* [1 Introduction](#1)
* [2 Lexical Structure](#2)
    * [2.1 Identifiers](#2.1)
    * [2.2 Keywords](#2.2)
    * [2.3 Operators](#2.3)
    * [2.4 Other tokens](#2.4)
    * [2.5 Literals](#2.5)
    * [2.6 Comments](#2.6)
* [3 Values and Data Types](#3)

# <a name="1"></a>1. Introduction
Ouzel Shading Language (OSL) is a high-level language, designed for writing cross-platform shaders. OSL can be transpiled to High Level Shading Language (HLSL), OpenGL Shading Language (GLSL), and Metal Shading Language (MSL). OSL is a subset of C++11.

# <a name="2"></a>2. Lexical Structure

## <a name="2.1"></a>2.1 Identifiers

Identifiers start with a alphabetic character or underscore (_) followed by any number of alphabetic characters, underscore or digits (0-9). Identifiers are case sensitive language.

## <a name="2.2"></a>2.2 Keywords

The following are the language's keywords and can only be used as described in this specification, or a compile-time error results:

* and
* bool
* break
* case
* const
* continue
* default
* do
* else
* false
* float
* for
* if
* inline
* int
* not
* not_eq
* or
* return
* static
* struct
* switch
* true
* void
* volatile
* while

The following are the keywords reserved for future use. Using them will result in a compile-time error:

* and_eq
* auto
* bitand
* bitor
* catch
* char
* class
* compl
* delete
* double
* enum
* explicit
* export
* extern
* friend
* goto
* long
* mutable
* namespace
* new
* noexcept
* nullptr
* operator
* or_eq
* private
* protected
* public
* short
* signed
* sizeof
* template
* this
* throw
* try
* typedef
* typeid
* typename
* union
* unsigned
* using
* virtual
* xor
* xor_eq

## <a name="2.3"></a>2.3 Operators

OSL has the following operators (in the order of precedence):

| Operator | Description |
|----------|-------------|
| () | parenthetical grouping |
| [] | subscript |
| . | member access |
| \* / | multiplicative |
| \+ \- | additive |
| < <= | less than relational |
| > >= | greater than relational |
| == != | equality relational |
| && | logical and |
| \|\| | logical or |
| ?: | selection |
| = | assignment |
| += -= | additive assignment |
| += -= | multiplicative assignment |
| , | sequence |

## <a name="2.4"></a>2.4 Other tokens

{ } [ ] . : \' \"

## <a name="2.5"></a>2.5 Literals

## <a name="2.6"></a>2.6 Comments

# <a name="3"></a>3 Values and Data Types