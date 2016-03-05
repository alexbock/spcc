#ifndef SPCC_KEYWORD_HH
#define SPCC_KEYWORD_HH

#include <string>
#include <map>

enum keyword {
    kw_auto,
    kw_break,
    kw_case,
    kw_char,
    kw_const,
    kw_continue,
    kw_default,
    kw_do,
    kw_double,
    kw_else,
    kw_enum,
    kw_extern,
    kw_float,
    kw_for,
    kw_goto,
    kw_if,
    kw_inline,
    kw_int,
    kw_long,
    kw_register,
    kw_restrict,
    kw_return,
    kw_short,
    kw_signed,
    kw_sizeof,
    kw_static,
    kw_struct,
    kw_switch,
    kw_typedef,
    kw_union,
    kw_unsigned,
    kw_void,
    kw_volatile,
    kw_while,
    kw_Alignas,
    kw_Alignof,
    kw_Atomic,
    kw_Bool,
    kw_Complex,
    kw_Generic,
    kw_Imaginary,
    kw_Noreturn,
    kw_Static_assert,
    kw_Thread_local,
};

extern std::map<std::string, keyword> keyword_table;

#endif
