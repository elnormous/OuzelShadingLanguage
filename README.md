# 1. Introduction
Ouzel Shading Language (OSL) is a high-level language, designed for writing cross-platform shaders. OSL can be transpiled to High Level Shading Language (HLSL), OpenGL Shading Language (GLSL), and Metal Shading Language (MSL). OSL is a subset of C++11.

# 2. Lexical Structure

## 2.1 Identifiers

Identifiers start with a alphabetic character or underscore (_) followed by any number of alphabetic characters, underscore or digits (0-9). Identifiers are case sensitive language.

## 2.2 Keywords

The following are the language's keywords and can only be used as described in this specification, or a compile-time error results:

* if
* else
* return
* for
* while
* do
* break
* continue
* true
* false
* switch
* case
* default
* static
* const
* struct
* bool
* int
* float
* void

The following are the keywords reserved for future use. Using them will result in a compile-time error:

* inline
* goto
* extern
* mutable
* auto
* typedef
* union
* enum
* template
* type
* class
* public
* protected
* private
* new
* delete
* this
* sizeof
* namespace
* using
* try
* catch
* throw
* noexcept
* operator
* signed
* unsigned
* char
* short
* long
* double
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