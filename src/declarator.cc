#include "declarator.hh"
#include "util.hh"
#include "diagnostic.hh"
#include "parse_expr.hh"

using diagnostic::diagnose;

enum declarator_precedence {
    dp_pointer = 1000,
    dp_qual = 1000,
};

static bool is_type_qualifier(keyword kw) {
    switch (kw) {
        case kw_Atomic:
        case kw_const:
        case kw_volatile:
        case kw_restrict:
            return true;
        default:
            return false;
    }
}

namespace parse {
    node_ptr abstract_placeholder_rule::parse(parser& p, token tok) const {
        p.rewind(); // we don't want to consume this token
        return std::make_unique<abstract_placeholder_node>(tok);
    }

    node_ptr declarator_array_rule::parse(parser& p, node_ptr lhs,
                                          token tok) const {
        std::vector<token> mods;
        while (p.peek().is(token::keyword)) {
            if (is_type_qualifier(p.peek().kw) || p.peek().is(kw_static)) {
                mods.push_back(p.next());
            } else break;
        }
        bool found_star = false;
        if (p.peek().is(punctuator::star)) {
            mods.push_back(p.next());
            found_star = true;
        }
        if (found_star && !p.peek().is(punctuator::square_right)) {
            // the grammar requires the star modifier to be last,
            // so we have to back this out to allow a VLA expression
            // that happens to start with unary *
            mods.pop_back();
            p.rewind();
        }
        node_ptr size = nullptr;
        if (!p.peek().is(punctuator::square_right)) {
            p.push_ruleset(false);
            size = p.parse(0);
            p.pop_ruleset();
        }
        auto end = p.next();
        if (!end.is(punctuator::square_right)) {
            diagnose(diagnostic::id::pp7_expected_end_of_array_declarator,
                     end.range.first);
        }
        return std::make_unique<declarator_array_node>(std::move(lhs),
                                                       std::move(mods),
                                                       std::move(size),
                                                       tok, end);
    }

    std::vector<init_declarator> parse_init_declarator_list(parser& p) {
        std::vector<init_declarator> declarators;
        bool first = true;
        while (!p.peek().is(punctuator::semicolon)) {
            if (!first) {
                if (p.peek().is(punctuator::comma)) {
                    p.next();
                } else {
                    diagnose(diagnostic::id::pp7_expected_token,
                             p.peek().range.first, ",");
                }
            }
            first = false;
            init_declarator id;
            p.push_ruleset(true);
            id.declarator = p.parse(0);
            p.pop_ruleset();
            if (p.peek().is(punctuator::equal)) {
                p.next();
                p.push_ruleset(false);
                id.init = p.parse(ep_comma);
                p.pop_ruleset();
            }
            declarators.push_back(std::move(id));
        }
        return declarators;
    }
}

parse::ruleset parse::declarator_ruleset = {
    {
        {
            +[](const token& tok, parser& p) -> bool {
                if (!tok.is(token::keyword)) return false;
                return is_type_qualifier(tok.kw);
            },
            new parse::unary_prefix_rule(dp_qual)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(token::identifier);
            },
            new parse::token_rule
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::star);
            },
            new parse::unary_prefix_rule(dp_pointer)
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::paren_left);
            },
            new parse::paren_rule
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::paren_right) ||
                       tok.is(punctuator::comma) ||
                       tok.is(punctuator::square_right);
            },
            new parse::abstract_placeholder_rule
        },
    },
    {
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::paren_left);
            },
            new parse::call_rule
        },
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(punctuator::square_left);
            },
            new parse::declarator_array_rule
        },
    }
};
