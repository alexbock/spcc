#pragma once

#include <cstddef>
#include <string>
#include <map>

/* [6.4.6]/1
 punctuator: one of
  [ ] ( ) { } . ->
  ++ -- & * + - ~ !
  / % << >> < > <= >= == != ^ | && || ? : ; ...
  = *= /= %= += -= <<= >>= &= ^= |=
  , # ##
  <: :> <% %> %: %:%:
*/
enum class punctuator_kind {
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
    digraph_less_colon,
    digraph_colon_greater,
    digraph_less_percent,
    digraph_percent_greater,
    digraph_percent_colon,
    digraph_percent_colon_percent_colon
};

extern std::map<std::string, punctuator_kind> punctuator_table;
static const std::size_t punctuator_max_length = 4;
