#include "declarator.hh"
#include "util.hh"

enum declarator_precedence {
    dp_pointer = 100,
    dp_pre_qual = 100,
    dp_type_spec = 100,
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

namespace parse {
    node_ptr abstract_placeholder_rule::parse(parser& p, token tok) const {
        p.rewind(); // we don't want to consume this token
        return std::make_unique<abstract_placeholder_node>(tok);
    }

    std::string pronounce_declarator(const node& root) {
        if (dynamic_cast<const abstract_placeholder_node*>(&root)) {
            return "";
        } else if (auto unary = dynamic_cast<const unary_node*>(&root)) {
            if (unary->token().is(punctuator::star)) {
                std::string result = pronounce_declarator(unary->operand());
                result += "pointer to ";
                return result;
            } else if (unary->token().is(token::keyword) &&
                       is_simple_type_spec(unary->token().kw)) {
                std::string result = pronounce_declarator(unary->operand());
                result += unary->token().spelling.to_string() + " ";
                return result;
            } else {
                std::string result = pronounce_declarator(unary->operand());
                result += unary->token().spelling.to_string() + " ";
                return result;
            }
        } else if (auto call = dynamic_cast<const call_node*>(&root)) {
            std::string result = pronounce_declarator(call->callee());
            result += "function ";
            if (!call->args().empty()) {
                result += "(";
                std::string comma = "";
                for (const auto& arg : call->args()) {
                    result += comma;
                    comma = ", ";
                    result += util::rtrim(pronounce_declarator(*arg));
                }
                result += ") ";
            }
            result += "returning ";
            return result;
        } else if (auto tn = dynamic_cast<const token_node*>(&root)) {
            auto result = tn->token().spelling.to_string();
            if (result != "...") result += " is ";
            return result;
        } else if (auto p = dynamic_cast<const paren_node*>(&root)) {
            return pronounce_declarator(p->operand());
        } else {
            return " ??? ";
        }
    }
}

parse::ruleset parse::declarator_ruleset = {
    {
        {
            +[](const token& tok) -> bool {
                if (!tok.is(token::keyword)) return false;
                return is_type_qualifier(tok.kw);
            },
            new parse::unary_prefix_rule(dp_pre_qual)
        },
        {
            +[](const token& tok) -> bool {
                if (!tok.is(token::keyword)) return false;
                return is_simple_type_spec(tok.kw);
            },
            new parse::unary_prefix_rule(dp_type_spec)
        },
        {
            +[](const token& tok) -> bool {
                return tok.is(token::identifier);
            },
            new parse::token_rule
        },
        {
            +[](const token& tok) -> bool {
                return tok.is(punctuator::star);
            },
            new parse::unary_prefix_rule(dp_pointer)
        },
        {
            +[](const token& tok) -> bool {
                return tok.is(punctuator::paren_left);
            },
            new parse::paren_rule
        },
        {
            +[](const token& tok) -> bool {
                return tok.is(punctuator::paren_right) ||
                       tok.is(punctuator::comma);
            },
            new parse::abstract_placeholder_rule
        },
    },
    {
        {
            +[](const token& tok) -> bool {
                return tok.is(punctuator::paren_left);
            },
            new parse::call_rule
        },
    }
};
