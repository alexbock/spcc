#ifndef SPCC_PARSE_EXPR_HH
#define SPCC_PARSE_EXPR_HH

#include "parser.hh"

namespace parse {
    extern ruleset expr_ruleset;

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

    class cast_node : public node {
    public:
        cast_node(token left, token right, node_ptr declarator,
                  node_ptr op) :
        left{left}, right{right}, declarator{std::move(declarator)},
        op{std::move(op)} { }

        loc_range range() override {
            return { left.range.first, right.range.second };
        }
    private:
        std::string get_dump_info() const override {
            return "CAST";
        }
        std::vector<const node*> children() const override {
            return { declarator.get(), op.get() };
        }

        token left, right;
        node_ptr declarator;
        node_ptr op;
    };
}

#endif
