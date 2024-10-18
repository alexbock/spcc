#include "expr.hh"
#include "pp.hh"
#include "diagnostic.hh"

#include <iostream>
#include <iomanip>
#include <cstdint>

namespace sem {
    expr::~expr() = default;

    void expr::dump(std::size_t indent) const {
        std::print(stderr, "{}{} {} {}",
                   std::string(indent * 4, ' '),
                   get_dump_name(),
                   (std::uintptr_t)this,
                   get_dump_info());
        if (indent == 0) {
            const auto& loc = range().first;
            const auto line_col = diagnostic::compute_line_col(loc);
            std::print(stderr, " @ {}: {}: {}: ",
                       loc.buffer().name(), line_col.first + 1, line_col.second + 1);
        }
        std::println(stderr, "");

        const auto children = this->children();
        for (auto child : children) {
            child->dump(indent + 1);
        }
    }

    std::pair<location, location> string_literal_expr::range() const {
        return str_tok.range;
    }

    std::string_view string_literal_expr::body() const {
        return pp::analyze_string_literal(str_tok).body;
    }

    std::string string_literal_expr::get_dump_name() const {
        return "string literal";
    }

    std::string string_literal_expr::get_dump_info() const {
        return std::string(str_tok.spelling);
    }

    std::vector<const expr*> string_literal_expr::children() const {
        return {};
    }
}
