# 1. Introduction
Ouzel Shading Language (OSL) is a high-level language, designed for writing shaders. OSL can be transpiled to High Level Shading Language (HLSL), OpenGL Shading Language (GLSL), and Metal Shading Language (MSL). OSL is a subset of C++11.

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