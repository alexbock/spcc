#include "parser.hh"
#include "diagnostic.hh"

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
        auto body = p.parse(INT_MAX);
        auto rparen = p.next();
        assert(rparen.is(punctuator::paren_right));
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
            if (p.peek().is(punctuator::ellipsis)) {
                allow_arg = false;
                require_arg = false;
                args.push_back(std::make_unique<token_node>(p.next()));
                continue;
            } else {
                require_arg = false;
                args.push_back(p.parse(0));
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
        return declarator_ruleset; // TODO
    }

    bool parser::is_typedef_name(string_view name) const {
        return typedef_names.count(name);
    }

    void parser::push_ruleset(bool declarator) {
        use_declarator_ruleset.push(declarator);
    }

    void parser::pop_ruleset() {
        assert(!use_declarator_ruleset.empty());
        use_declarator_ruleset.pop();
    }

    int parser::precedence_peek() {
        if (!has_next_token()) return 0;
        auto in_rule = find_rule(peek(), rules().infix_rules);
        return in_rule ? in_rule->precedence() : 0;
    }
}