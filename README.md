# 1. Introduction
Ouzel Shading Language (OSL) is a high-level language, designed for writing cross-platform shaders. OSL can be transpiled to High Level Shading Language (HLSL), OpenGL Shading Language (GLSL), and Metal Shading Language (MSL). OSL is a subset of C++11.

# 2. Lexical Structure

## 2.1 Identifiers

Identifiers start with a alphabetic character or underscore (_) followed by any number of alphabetic characters, underscore or digits (0-9). Identifiers are case sensitive language.

## 2.2 Keywords

The following are the keywords of the language and cannot be used as identifiers:

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
* inline
* goto
* switch
* case
* default
* static
* const
* extern
* mutable
* auto
* typedef
* union
* enum
* template
* type
* struct
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
* bool
* char
* short
* int
* long
* float
* double
* virtual
* void

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