spcc is a simple C11 compiler written in C++14 that aims to be strictly
compliant with the ISO C standard.

Preprocessor Progress:
- [x] Expressive diagnostic system
- [x] Convert UTF-8 multibyte characters to universal character names
- [x] Process trigraphs
- [x] Process line splices
- [x] Lex source file into preprocessing tokens
- [x] `#ifdef`/`#ifndef`/`#else`/`#endif`
- [x] `#define`/`#undef`
- [x] `#include`
- [x] `#error`
- [x] Macro expansion with support for `#`, `##`, and the rescanning rules
- [x] `__VA_ARGS__`
- [ ] `#line`
- [ ] `#pragma STDC`
- [ ] `#if`/`#elif`
- [ ] `_Pragma`
- [x] Predefined macros
- [ ] Convert string literals and character constants to the execution
character set  
- [X] Concatenate adjacent string literals
- [x] Convert preprocessing tokens to tokens

Semantic Analysis Progress:
- [ ] Type system
- [x] Expression parser
- [x] Declarator parser
- [ ] Expression analysis
- [ ] Constant expression evaluation

Visual Studio Note:
Visual Studio 2015 is not capable of compiling this project due to a
number of compiler bugs that are not yet resolved as of VS2015 Update 2:
- C++14 aggregate list initialization with non-static data member initializers
  don't seem to be supported
- Conversion of a non-capturing lambda to a function pointer is incorrectly
  reported as being ambiguous due to the non-standard calling convention
  overloads VS2015 internally generates for the conversion function.
- The use of an elaborated type specifier like `enum X` to refer to an
  enum defined using `enum class X` is incorrectly reported as an error.
  The use of plain `enum` here is explicit in the grammar for an elaborated
  type specifier.
- The bodies of functions and initializers for namespace members defined at
  global scope using a nested name specifier are not processed in the
  correct scope.
