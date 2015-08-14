#include "punctuator.hh"

/* [6.4.6]/1
punctuator: one of
  [ ] ( ) { } . ->
  ++ -- & * + - ~ !
  / % << >> < > <= >= == != ^ | && || ? : ; ...
  = *= /= %= += -= <<= >>= &= ^= |=
  , # ##
  <: :> <% %> %: %:%:
*/
std::map<std::string, punctuator_kind> punctuator_table = {
    { "[", punctuator_kind::square_left },
    { "]", punctuator_kind::square_right },
    { "(", punctuator_kind::paren_left },
    { ")", punctuator_kind::paren_right },
    { "{", punctuator_kind::curly_left },
    { "}", punctuator_kind::curly_right },
    { ".", punctuator_kind::dot },
    { "->", punctuator_kind::arrow },
    { "++", punctuator_kind::plus_plus },
    { "--", punctuator_kind::minus_minus },
    { "&", punctuator_kind::ampersand },
    { "*", punctuator_kind::star },
    { "+", punctuator_kind::plus },
    { "-", punctuator_kind::minus },
    { "~", punctuator_kind::tilde },
    { "!", punctuator_kind::bang },
    { "/", punctuator_kind::slash_forward },
    { "%", punctuator_kind::percent },
    { "<<", punctuator_kind::less_less },
    { ">>", punctuator_kind::greater_greater },
    { "<", punctuator_kind::less },
    { ">", punctuator_kind::greater },
    { "<=", punctuator_kind::less_equal },
    { ">=", punctuator_kind::greater_equal },
    { "==", punctuator_kind::equal_equal },
    { "!=", punctuator_kind::bang_equal },
    { "^", punctuator_kind::caret },
    { "|", punctuator_kind::pipe },
    { "&&", punctuator_kind::ampersand_ampersand },
    { "||", punctuator_kind::pipe_pipe },
    { "?", punctuator_kind::question },
    { ":", punctuator_kind::colon },
    { ";", punctuator_kind::semicolon },
    { "...", punctuator_kind::ellipsis },
    { "=", punctuator_kind::equal },
    { "*=", punctuator_kind::star_equal },
    { "/=", punctuator_kind::slash_forward_equal },
    { "%=", punctuator_kind::percent_equal },
    { "+=", punctuator_kind::plus_equal },
    { "-=", punctuator_kind::minus_equal },
    { "<<=", punctuator_kind::less_less_equal },
    { ">>=", punctuator_kind::greater_greater_equal },
    { "&=", punctuator_kind::ampersand_equal },
    { "^=", punctuator_kind::caret_equal },
    { "|=", punctuator_kind::pipe_equal },
    { ",", punctuator_kind::comma },
    { "#", punctuator_kind::hash },
    { "##", punctuator_kind::hash_hash },
    { "<:", punctuator_kind::digraph_less_colon },
    { ":>", punctuator_kind::digraph_colon_greater },
    { "<%", punctuator_kind::digraph_less_percent },
    { "%>", punctuator_kind::digraph_percent_greater },
    { "%:", punctuator_kind::digraph_percent_colon },
    { "%:%:", punctuator_kind::digraph_percent_colon_percent_colon }
};
