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
            std::vector<const node*> result = { lhs.get() };
            if (sz) result.push_back(sz.get());
            return result;
        }

        node_ptr lhs;
        std::vector<token> mods;
        node_ptr sz;
        token left, right;
    };

    class tag_node : public node {
    public:
        tag_node(token tag, optional<token> ident, optional<token> rbrace,
                 std::vector<node_ptr> body, node_ptr op) :
        tag_{tag}, ident_{ident}, rbrace_{rbrace},
        body_{std::move(body)}, op_{std::move(op)} { }

        loc_range range() override {
            auto begin = tag().range.first;
            auto end = tag().range.second;
            if (ident()) end = ident()->range.second;
            if (rbrace_) end = rbrace_->range.second;
            return { begin, end };
        }

        const token& tag() const { return tag_; }
        const optional<token>& ident() const { return ident_; }
        const std::vector<node_ptr>& body() const { return body_; }
        const node& operand() const { return *op_; }
    private:
        std::string get_dump_info() const override {
            auto result = "TAG " + tag().spelling.to_string();
            if (ident()) {
                result += " " + ident()->spelling.to_string();
            }
            return result;
        }
        std::vector<const node*> children() const override {
            std::vector<const node*> result;
            for (const auto& child : body()) {
                result.push_back(child.get());
            }
            result.push_back(&operand());
            return result;
        }

        token tag_;
        optional<token> ident_;
        optional<token> rbrace_;
        std::vector<node_ptr> body_;
        node_ptr op_;
    };

    class declarator_array_rule : public infix_rule {
    public:
        node_ptr parse(parser&, node_ptr, token) const override;
        int precedence() const override { return INT_MAX; }
    };

    class tag_rule : public prefix_rule {
    public:
        node_ptr parse(parser&, token) const override;
    };
}

#endif
