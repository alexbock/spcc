#ifndef SPCC_DECLARATOR_HH
#define SPCC_DECLARATOR_HH

#include "parser.hh"

namespace parse {
    extern ruleset declarator_ruleset;

    class abstract_placeholder_node : public node {
    public:
        abstract_placeholder_node(token rparen) : rparen{rparen} { }
        loc_range range() override { return rparen.range; }
    private:
        std::string get_dump_info() const override {
            return "ABSTRACT PLACEHOLDER";
        }
        std::vector<const node*> children() const override {
            return {};
        }

        token rparen;
    };

    class abstract_placeholder_rule : public prefix_rule {
    public:
        node_ptr parse(parser&, token) const override;
    };

    class declarator_array_node : public node {
    public:
        declarator_array_node(node_ptr base, std::vector<token> mods,
                              node_ptr sz, token left, token right) :
        lhs{std::move(base)}, mods{std::move(mods)}, sz{std::move(sz)},
        left{left}, right{right} { }

        loc_range range() override {
            return { left.range.first, right.range.second };
        }

        const std::vector<token>& modifiers() const { return mods; }
        const node* size() const { return sz.get(); }
        const node& base() const { return *lhs; }
    private:
        std::string get_dump_info() const override {
            return "ARRAY";
        }
        std::vector<const node*> children() const override {
            return { lhs.get() };
        }

        node_ptr lhs;
        std::vector<token> mods;
        node_ptr sz;
        token left, right;
    };

    class declarator_array_rule : public infix_rule {
    public:
        node_ptr parse(parser&, node_ptr, token) const override;
        int precedence() const override { return INT_MAX; }
    };

    std::string pronounce_declarator(const node& root);
}

#endif
