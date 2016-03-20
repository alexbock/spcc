#include "declarator.hh"
#include "util.hh"
#include "diagnostic.hh"

using diagnostic::diagnose;

enum declarator_precedence {
    dp_pointer = 1000,
    dp_pre_qual = 1000,
    dp_type_spec = 1000,
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

static bool is_simple_type_spec(keyword kw) {
    switch (kw) {
        case kw_void:
        case kw_short:
        case kw_int:
        case kw_long:
        case kw_float:
        case kw_double:
        case kw_signed:
        case kw_unsigned:
        case kw_Bool:
        case kw_Complex:
            return true;
        default:
            return false;
    }
}

static bool is_storage_class_spec(keyword kw) {
    switch (kw) {
        case kw_typedef:
        case kw_extern:
        case kw_static:
        case kw_Thread_local:
        case kw_auto:
        case kw_register:
            return true;
        default:
            return false;
    }
}

static bool is_function_specifier(keyword kw) {
    switch (kw) {
        case kw_inline:
        case kw_Noreturn:
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

    node_ptr tag_rule::parse(parser& p, token tok) const {
        optional<token> ident;
        if (p.peek().is(token::identifier)) {
            ident = p.next();
        }
        std::vector<node_ptr> body;
        optional<token> rbrace;
        if (p.peek().is(punctuator::curly_left)) {
            p.next();
            while (!p.peek().is(punctuator::curly_right)) {
                auto child = p.parse(0);
                if (p.peek().is(punctuator::semicolon)) p.next();
                else {
                    diagnose(diagnostic::id::pp7_expected_semicolon,
                             p.peek().range.first);
                }
                body.push_back(std::move(child));
            }
            assert(p.peek().is(punctuator::curly_right));
            rbrace = p.next();
        } else if (!ident) {
            diagnose(diagnostic::id::pp7_expected_ident_or_body,
                     p.peek().range.first);
        }
        return std::make_unique<tag_node>(tok, ident, rbrace,
                                          std::move(body), p.parse(0));
    }
}

parse::ruleset parse::declarator_ruleset = {
    {
        {
            +[](const token& tok, parser& p) -> bool {
                if (tok.is(token::identifier)) {
                    if (p.is_typedef_name(tok.spelling)) return true;
                }
                if (!tok.is(token::keyword)) return false;
                return is_simple_type_spec(tok.kw) ||
                       is_storage_class_spec(tok.kw) ||
                       is_type_qualifier(tok.kw) ||
                       is_function_specifier(tok.kw);
            },
            new parse::unary_prefix_rule(dp_type_spec)
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
        {
            +[](const token& tok, parser& p) -> bool {
                return tok.is(kw_struct) ||
                       tok.is(kw_union) ||
                       tok.is(kw_enum);
            },
            new parse::tag_rule
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
