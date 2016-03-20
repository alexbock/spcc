#include "parse_expr.hh"

enum expr_precedence {
    ep_postfix = 1000,
    ep_prefix = 900,
    ep_multiplicative = 800,
    ep_additive = 750,
    ep_shift = 700,
    ep_relational = 650,
    ep_equality = 600,
    ep_bit_and = 575,
    ep_bit_xor = 550,
    ep_bit_or = 525,
    ep_logical_and = 500,
    ep_logical_or = 475,
    ep_conditonal = 400,
    ep_assignment = 300,
    ep_comma = 1,
};

parse::ruleset parse::expr_ruleset = {
    {
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(token::identifier) ||
                       tok.is(token::integer_constant);
            },
            new parse::token_rule
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::paren_left);
            },
            new parse::paren_rule
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::plus_plus) ||
                       tok.is(punctuator::minus_minus) ||
                       tok.is(punctuator::plus) ||
                       tok.is(punctuator::minus) ||
                       tok.is(punctuator::bang) ||
                       tok.is(punctuator::tilde) ||
                       tok.is(punctuator::star) ||
                       tok.is(punctuator::ampersand) ||
                       tok.is(kw_sizeof) ||
                       tok.is(kw_Alignof);
            },
            new parse::unary_prefix_rule(ep_prefix)
        },
    },
    {
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::plus_plus) ||
                       tok.is(punctuator::minus_minus);
            },
            new parse::unary_postfix_rule(ep_postfix)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::paren_left);
            },
            new parse::call_rule
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::dot) ||
                       tok.is(punctuator::arrow);
            },
            new parse::binary_rule(ep_postfix, false)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::star) ||
                       tok.is(punctuator::slash_forward) ||
                       tok.is(punctuator::percent);
            },
            new parse::binary_rule(ep_multiplicative, false)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::plus) ||
                       tok.is(punctuator::minus);
            },
            new parse::binary_rule(ep_additive, false)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::less_less) ||
                       tok.is(punctuator::greater_greater);
            },
            new parse::binary_rule(ep_shift, false)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::less) ||
                       tok.is(punctuator::greater) ||
                       tok.is(punctuator::less_equal) ||
                       tok.is(punctuator::greater_equal);
            },
            new parse::binary_rule(ep_relational, false)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::equal) ||
                       tok.is(punctuator::bang_equal);
            },
            new parse::binary_rule(ep_equality, false)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::ampersand);
            },
            new parse::binary_rule(ep_bit_and, false)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::caret);
            },
            new parse::binary_rule(ep_bit_xor, false)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::pipe);
            },
            new parse::binary_rule(ep_bit_or, false)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::ampersand_ampersand);
            },
            new parse::binary_rule(ep_logical_and, false)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::pipe_pipe);
            },
            new parse::binary_rule(ep_logical_or, false)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::comma);
            },
            new parse::binary_rule(ep_comma, false)
        },
    }
};
