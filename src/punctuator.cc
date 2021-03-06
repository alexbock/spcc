#include "punctuator.hh"

std::map<std::string, punctuator> punctuator_table = {
    { "[", punctuator::square_left },
    { "]", punctuator::square_right },
    { "(", punctuator::paren_left },
    { ")", punctuator::paren_right },
    { "{", punctuator::curly_left },
    { "}", punctuator::curly_right },
    { ".", punctuator::dot },
    { "->", punctuator::arrow },
    { "++", punctuator::plus_plus },
    { "--", punctuator::minus_minus },
    { "&", punctuator::ampersand },
    { "*", punctuator::star },
    { "+", punctuator::plus },
    { "-", punctuator::minus },
    { "~", punctuator::tilde },
    { "!", punctuator::bang },
    { "/", punctuator::slash_forward },
    { "%", punctuator::percent },
    { "<<", punctuator::less_less },
    { ">>", punctuator::greater_greater },
    { "<", punctuator::less },
    { ">", punctuator::greater },
    { "<=", punctuator::less_equal },
    { ">=", punctuator::greater_equal },
    { "==", punctuator::equal_equal },
    { "!=", punctuator::bang_equal },
    { "^", punctuator::caret },
    { "|", punctuator::pipe },
    { "&&", punctuator::ampersand_ampersand },
    { "||", punctuator::pipe_pipe },
    { "?", punctuator::question },
    { ":", punctuator::colon },
    { ";", punctuator::semicolon },
    { "...", punctuator::ellipsis },
    { "=", punctuator::equal },
    { "*=", punctuator::star_equal },
    { "/=", punctuator::slash_forward_equal },
    { "%=", punctuator::percent_equal },
    { "+=", punctuator::plus_equal },
    { "-=", punctuator::minus_equal },
    { "<<=", punctuator::less_less_equal },
    { ">>=", punctuator::greater_greater_equal },
    { "&=", punctuator::ampersand_equal },
    { "^=", punctuator::caret_equal },
    { "|=", punctuator::pipe_equal },
    { ",", punctuator::comma },
    { "#", punctuator::hash },
    { "##", punctuator::hash_hash },
    { "<:", punctuator::square_left },
    { ":>", punctuator::square_right },
    { "<%", punctuator::curly_left },
    { "%>", punctuator::curly_right },
    { "%:", punctuator::hash },
    { "%:%:", punctuator::hash_hash },
};

/* [6.4.6]/3
 In all aspects of the language, the six tokens
 <:  :>  <%  %>  %:  %:%:
 behave, respectively, the same as the six tokens
 [ ] { } # ##
 except for their spelling.
*/