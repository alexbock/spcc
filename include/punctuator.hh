#ifndef SPCC_PUNCTUATOR_HH
#define SPCC_PUNCTUATOR_HH

#include <map>
#include <string>

/* [6.4.6]/1
 punctuator: one of
 [ ] ( ) { } . ->
 ++ -- & * + - ~ !
 / % << >> < > <= >= == != ^ | && || ? : ; ...
 = *= /= %= += -= <<= >>= &= ^= |=
 , # ##
 <: :> <% %> %: %:%:
 */
enum class punctuator {
    square_left,
    square_right,
    paren_left,
    paren_right,
    curly_left,
    curly_right,
    dot,
    arrow,
    plus_plus,
    minus_minus,
    ampersand,
    star,
    plus,
    minus,
    tilde,
    bang,
    slash_forward,
    percent,
    less_less,
    greater_greater,
    less,
    greater,
    less_equal,
    greater_equal,
    equal_equal,
    bang_equal,
    caret,
    pipe,
    ampersand_ampersand,
    pipe_pipe,
    question,
    colon,
    semicolon,
    ellipsis,
    equal,
    star_equal,
    slash_forward_equal,
    percent_equal,
    plus_equal,
    minus_equal,
    less_less_equal,
    greater_greater_equal,
    ampersand_equal,
    caret_equal,
    pipe_equal,
    comma,
    hash,
    hash_hash,
};

extern std::map<std::string, punctuator> punctuator_table;

#endif
