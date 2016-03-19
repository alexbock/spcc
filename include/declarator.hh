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

    std::string pronounce_declarator(const node& root);
}

#endif
