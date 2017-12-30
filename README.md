# 1. Introduction
Ouzel Shading Language (OSL) is a high-level language, designed for writing cross-platform shaders. OSL can be transpiled to High Level Shading Language (HLSL), OpenGL Shading Language (GLSL), and Metal Shading Language (MSL). OSL is a subset of C++11.

# 2. Lexical Structure

## 2.1 Identifiers

Identifiers start with a alphabetic character or underscore (_) followed by any number of alphabetic characters, underscore or digits (0-9). Identifiers are case sensitive language.

## 2.2 Keywords

The following are the language's keywords and can only be used as described in this specification, or a compile-time error results:

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
* return
* static
* struct
* switch
* true
* void
* while

The following are the keywords reserved for future use. Using them will result in a compile-time error:

* auto
* catch
* char
* class
* delete
* double
* enum
* extern
* goto
* long
* mutable
* namespace
* new
* noexcept
* operator
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
* type
* typedef
* union
* unsigned
* using
* virtual

## 2.3 Operators

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

## 2.4 Other tokens

{ } [ ] . : \' \"

## 2.5 Literals

## 2.6 Comments

# 3 Values and Data Types