#include "parser.hh"
#include "diagnostic.hh"
#include "parse_expr.hh"

#include <cassert>
#include <climits>
#include <iostream>

using diagnostic::diagnose;

namespace parse {
    void node::dump(std::size_t indent) const {
        std::cerr << std::string(indent * 4, ' ');
        std::cerr << get_dump_info() << "\n";
        const auto children = this->children();
        for (auto child : children) {
            child->dump(indent + 1);
        }
    }

    node::~node() = default;

    node_ptr token_rule::parse(parser& p, token tok) const {
        return std::make_unique<token_node>(std::move(tok));
    }

    node_ptr unary_prefix_rule::parse(parser& p, token tok) const {
        return std::make_unique<unary_node>(std::move(tok),
                                            p.parse(prec),
                                            true);
    }

    node_ptr paren_rule::parse(parser& p, token tok) const {
        assert(tok.is(punctuator::paren_left));
        if (!p.is_parsing_declarator()) {
            if (!p.could_be_expr_ahead()) {
                p.push_ruleset(true);
                auto declarator = p.parse(0);
                p.pop_ruleset();
                auto rparen = p.next();
                assert(rparen.is(punctuator::paren_right)); // TODO error
                auto operand = p.parse(ep_prefix);
                return std::make_unique<cast_node>(tok, rparen,
                                                   std::move(declarator),
                                                   std::move(operand));
            }
        }
        auto body = p.parse(0);
        auto rparen = p.next();
        assert(rparen.is(punctuator::paren_right)); // TODO error
        return std::make_unique<paren_node>(tok, std::move(body), rparen);
    }

    node_ptr binary_rule::parse(parser& p, node_ptr left, token tok) const {
        auto rhs_prec = prec;
        if (right_assoc) --rhs_prec;
        auto right = p.parse(rhs_prec);
        return std::make_unique<binary_node>(tok,
                                             std::move(left),
                                             std::move(right));
    }

    node_ptr unary_postfix_rule::parse(parser& p, node_ptr lhs,
                                       token tok) const {
        return std::make_unique<unary_node>(tok, std::move(lhs), false);
    }

    node_ptr call_rule::parse(parser& p, node_ptr lhs, token tok) const {
        assert(tok.is(punctuator::paren_left));
        std::vector<node_ptr> args;
        bool allow_arg = true;
        bool require_arg = false;
        while (!p.peek().is(punctuator::paren_right)) {
            if (!allow_arg) {
                diagnose(diagnostic::id::pp7_expected_end_of_list,
                         p.peek().range.first);
                allow_arg = true;
            }
            if (p.is_parsing_declarator()) {
                if (p.peek().is(punctuator::ellipsis)) {
                    allow_arg = false;
                    require_arg = false;
                    args.push_back(std::make_unique<token_node>(p.next()));
                    continue;
                } else {
                    // TODO decl spec + declarator
                    require_arg = false;
                    args.push_back(p.parse(0));
                    if (p.peek().is(punctuator::comma)) {
                        require_arg = true;
                        p.next();
                    }
                }
            } else {
                require_arg = false;
                args.push_back(p.parse(ep_comma));
                if (p.peek().is(punctuator::comma)) {
                    require_arg = true;
                    p.next();
                }
            }
        }
        if (require_arg) {
            diagnose(diagnostic::id::pp7_incomplete_list,
                     p.peek().range.first);
        }
        auto rparen = p.next();
        return std::make_unique<call_node>(std::move(lhs),
                                           tok, rparen,
                                           std::move(args));
    }

    node_ptr parser::parse(int precedence) {
        auto tok = next();
        auto pre_rule = find_rule(tok, rules().prefix_rules);
        if (!pre_rule) throw parse_error("no rules matched token");
        auto node = pre_rule->parse(*this, std::move(tok));
        while (precedence < precedence_peek()) {
            tok = next();
            auto in_rule = find_rule(tok, rules().infix_rules);
            node = in_rule->parse(*this, std::move(node), tok);
        }
        return node;
    }

    token parser::next() {
        auto tok = peek();
        ++next_token;
        return tok;
    }

    const token& parser::peek() const {
        if (has_next_token()) return tokens[next_token];
        else throw parse_error("no more tokens");
    }

    void parser::rewind() {
        assert(next_token);
        --next_token;
    }

    bool parser::has_next_token() const {
        return next_token < tokens.size();
    }

    const ruleset& parser::rules() const {
        return is_parsing_declarator() ? declarator_ruleset : expr_ruleset;
    }

    bool parser::is_typedef_name(string_view name) const {
        return typedef_names.count(name);
    }

    const sem::type* parser::get_typedef_type(string_view name) const {
        assert(is_typedef_name(name));
        return typedef_names.find(name)->second;
    }

    void parser::push_ruleset(bool declarator) {
        use_declarator_ruleset.push(declarator);
    }

    void parser::pop_ruleset() {
        assert(!use_declarator_ruleset.empty());
        use_declarator_ruleset.pop();
    }

    bool parser::could_be_expr_ahead() const {
        auto tok = peek();
        if (tok.is(token::identifier)) return !is_typedef_name(tok.spelling);
        if (tok.is(token::keyword)) return tok.is(kw_sizeof);
        return true;
    }

    int parser::precedence_peek() {
        if (!has_next_token()) return 0;
        auto in_rule = find_rule(peek(), rules().infix_rules);
        return in_rule ? in_rule->precedence() : 0;
    }

    bool parser::is_parsing_declarator() const {
        assert(!use_declarator_ruleset.empty());
        return use_declarator_ruleset.top();
    }
}
