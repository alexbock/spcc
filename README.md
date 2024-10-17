spcc is a simple C11 compiler written in C++20 that aims to be strictly
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
