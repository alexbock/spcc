#include "keyword.hh"

std::map<std::string, keyword> keyword_table = {
    { "auto", kw_auto },
    { "break", kw_break },
    { "case", kw_case },
    { "char", kw_char },
    { "const", kw_const },
    { "continue", kw_continue },
    { "default", kw_default },
    { "do", kw_do },
    { "double", kw_double },
    { "else", kw_else },
    { "enum", kw_enum },
    { "extern", kw_extern },
    { "float", kw_float },
    { "for", kw_for },
    { "goto", kw_goto },
    { "if", kw_if },
    { "inline", kw_inline },
    { "int", kw_int },
    { "long", kw_long },
    { "register", kw_register },
    { "restrict", kw_restrict },
    { "return", kw_return },
    { "short", kw_short },
    { "sizeof", kw_sizeof },
    { "static", kw_static },
    { "struct", kw_struct },
    { "switch", kw_switch },
    { "typedef", kw_typedef },
    { "union", kw_union },
    { "unsigned", kw_unsigned },
    { "void", kw_void },
    { "volatile", kw_volatile },
    { "while", kw_while },
    { "_Alignas", kw_Alignas },
    { "_Alignof", kw_Alignof },
    { "_Atomic", kw_Atomic },
    { "_Bool", kw_Bool },
    { "_Complex", kw_Complex },
    { "_Generic", kw_Generic },
    { "_Imaginary", kw_Imaginary },
    { "_Noreturn", kw_Noreturn },
    { "_Static_assert", kw_Static_assert },
    { "_Thread_local", kw_Thread_local },
};
