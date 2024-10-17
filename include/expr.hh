#ifndef SPCC_EXPR_HH
#define SPCC_EXPR_HH

#include "buffer.hh"
#include "type.hh"
#include "token.hh"

#include <utility>
#include <cstddef>
#include <vector>

namespace sem {
    class expr {
    public:
        virtual std::pair<location, location> range() const = 0;
        void dump(std::size_t indent = 0) const;
        virtual ~expr() = 0;
    private:
        virtual std::string get_dump_name() const = 0;
        virtual std::string get_dump_info() const = 0;
        virtual std::vector<const expr*> children() const = 0;
    };

    class string_literal_expr : public expr {
    public:
        std::pair<location, location> range() const override;
        std::string_view body() const;
    private:
        std::string get_dump_name() const override;
        std::string get_dump_info() const override;
        std::vector<const expr*> children() const override;

        token str_tok;
    };

    class unary_expr : public expr {
    public:
    private:
    };

    class binary_expr : public expr {
    public:
    private:
    };
}

#endif
