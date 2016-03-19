#ifndef SPCC_PARSER_HH
#define SPCC_PARSER_HH

#include "token.hh"
#include "buffer.hh"

#include <memory>
#include <vector>
#include <exception>
#include <map>
#include <cstddef>

namespace parse {
    class parser;

    class node {
    public:
        void dump(std::size_t indent = 0) const;
        virtual loc_range range() = 0;
        virtual ~node() = 0;
    private:
        virtual std::string get_dump_info() const = 0;
        virtual std::vector<const node*> children() const = 0;
    };

    using node_ptr = std::unique_ptr<node>;

    class token_node : public node {
    public:
        token_node(token tok) : tok{tok} { }
        loc_range range() override { return tok.range; }
        const struct token& token() const { return tok; }
    private:
        std::string get_dump_info() const override {
            return "TOKEN " + tok.spelling.to_string();
        }
        std::vector<const node*> children() const override {
            return {};
        }

        struct token tok;
    };

    class unary_node : public node {
    public:
        unary_node(token tok, node_ptr operand, bool prefix) :
        tok{tok}, op{std::move(operand)}, prefix{prefix} { }

        loc_range range() override {
            return { tok.range.first, op->range().second };
        }
        const struct token& token() const { return tok; }
        const node& operand() const { return *op; }
        bool is_prefix() const { return prefix; }
    private:
        std::string get_dump_info() const override {
            auto result =  "UNARY " + tok.spelling.to_string();
            result += prefix ? " prefix" : " postfix";
            return result;
        }
        std::vector<const node*> children() const override {
            return { op.get() };
        }

        struct token tok;
        node_ptr op;
        bool prefix;
    };

    class paren_node : public node {
    public:
        paren_node(token left, node_ptr operand, token right) :
        left{left}, op{std::move(operand)}, right{right} { }

        loc_range range() override {
            return { left.range.first, right.range.second };
        }
        const node& operand() const { return *op; }
        const struct token& lparen() const { return left; }
        const struct token& rparen() const { return right; }
    private:
        std::string get_dump_info() const override {
            return "PAREN GROUPING ()";
        }
        std::vector<const node*> children() const override {
            return { op.get() };
        }

        struct token left;
        node_ptr op;
        struct token right;
    };

    class binary_node : public node {
    public:
        binary_node(token tok, node_ptr left, node_ptr right) :
        tok{tok}, left{std::move(left)}, right{std::move(right)} { }

        loc_range range() override {
            return { left->range().first, right->range().second };
        }
        const struct token& token() const { return tok; }
        const node& lhs() const { return *left; }
        const node& rhs() const { return *right; }
    private:
        std::string get_dump_info() const override {
            return "BINARY " + tok.spelling.to_string();
        }
        std::vector<const node*> children() const override {
            return { left.get(), right.get() };
        }

        struct token tok;
        node_ptr left, right;
    };

    class ternary_node : public node {
    public:
        ternary_node(token tok1, token tok2, node_ptr op1,
                     node_ptr op2, node_ptr op3) :
        tok1{tok1}, tok2{tok2}, op1{std::move(op1)},
        op2{std::move(op2)}, op3{std::move(op3)} { }

        loc_range range() override {
            return { op1->range().first, op3->range().second };
        }
        const struct token& first_token() const { return tok1; }
        const struct token& second_token() const { return tok2; }
        const node& first_operand() const { return *op1; }
        const node& second_operand() const { return *op2; }
        const node& third_operand() const { return *op3; }
    private:
        std::string get_dump_info() const override {
            auto result = "TERNARY " + tok1.spelling.to_string();
            result += " " + tok2.spelling.to_string();
            return result;
        }
        std::vector<const node*> children() const override {
            return { op1.get(), op2.get(), op3.get() };
        }

        struct token tok1, tok2;
        node_ptr op1, op2, op3;
    };

    class call_node : public node {
    public:
        call_node(node_ptr callee, token lparen, token rparen,
                  std::vector<node_ptr> args) :
        call_target{std::move(callee)}, left{lparen}, right{rparen},
        arguments{std::move(args)} { }

        loc_range range() override {
            return { call_target->range().first, right.range.second };
        }
        const struct token& lparen() const { return left; }
        const struct token& rparen() const { return right; }
        const std::vector<node_ptr>& args() const { return arguments; }
        const node& callee() const { return *call_target; }
    private:
        std::string get_dump_info() const override {
            return "CALL()";
        }
        std::vector<const node*> children() const override {
            std::vector<const node*> result;
            result.push_back(call_target.get());
            for (const auto& arg : arguments) {
                result.push_back(arg.get());
            }
            return result;
        }

        node_ptr call_target;
        struct token left;
        struct token right;
        std::vector<node_ptr> arguments;
    };

    class prefix_rule {
    public:
        virtual node_ptr parse(parser&, token) const = 0;
        virtual ~prefix_rule() = default;
    };

    class token_rule : public prefix_rule {
    public:
        node_ptr parse(parser&, token) const override;
    };

    class unary_prefix_rule : public prefix_rule {
    public:
        unary_prefix_rule(int prec) : prec{prec} { }
        node_ptr parse(parser&, token) const override;
    private:
        int prec;
    };

    class paren_rule : public prefix_rule {
    public:
        node_ptr parse(parser&, token) const override;
    };

    class infix_rule {
    public:
        virtual node_ptr parse(parser&, node_ptr, token) const = 0;
        virtual int precedence() const = 0;
        virtual ~infix_rule() = default;
    };

    class binary_rule : public infix_rule {
    public:
        binary_rule(int prec, bool right_assoc) :
        prec{prec}, right_assoc{right_assoc} { }

        node_ptr parse(parser&, node_ptr, token) const override;
        int precedence() const override { return prec; }
    private:
        int prec;
        bool right_assoc;
    };

    class unary_postfix_rule : public infix_rule {
    public:
        unary_postfix_rule(int prec) : prec{prec} { }
        node_ptr parse(parser&, node_ptr, token) const override;
        int precedence() const override { return prec; }
    private:
        int prec;
    };

    class call_rule : public infix_rule {
    public:
        node_ptr parse(parser&, node_ptr, token) const override;
        int precedence() const override { return INT_MAX; }
    };

    class parse_error : public std::exception {
    public:
        parse_error(std::string msg) : msg{std::move(msg)} { }
        const char* what() const noexcept override {
            return msg.c_str();
        }
    private:
        std::string msg;
    };

    using token_check = bool(*)(const token&);
    template<typename T>
    using rule_map = std::map<token_check, T*>;
    struct ruleset {
        rule_map<prefix_rule> prefix_rules;
        rule_map<infix_rule> infix_rules;
    };

    extern ruleset expr_ruleset;
    extern ruleset declarator_ruleset;

    class parser {
    public:
        parser(const std::vector<token>& tokens) : tokens{tokens} { }
        node_ptr parse(int precedence);
        token next() noexcept(false);
        const token& peek() const noexcept(false);
        void rewind();
        bool has_next_token() const;
        const ruleset& rules() const;
    private:
        template<typename T>
        const T* find_rule(const token& tok, const rule_map<T>& rules) const {
            T* match = nullptr;
            for (const auto& pair : rules) {
                if (pair.first(tok)) {
                    if (match) {
                        throw parse_error("multiple rules matched token");
                    }
                    match = pair.second;
                }
            }
            return match;
        }
        int precedence_peek() const noexcept(false);

        const std::vector<token>& tokens;
        std::size_t next_token = 0;
    };
}

#endif
