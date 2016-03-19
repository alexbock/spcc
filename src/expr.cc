#include "expr.hh"
#include "pp.hh"
#include "diagnostic.hh"

#include <iostream>
#include <iomanip>
#include <cstdint>

namespace sem {
    expr::~expr() = default;

    void expr::dump(std::size_t indent) const {
        std::cerr << std::string(indent * 4, ' ');
        std::cerr << get_dump_name();
        std::cerr << " ";
        std::cerr << "0x" << std::hex << (std::uintptr_t)this;
        std::cerr << " ";
        std::cerr << get_dump_info();
        if (indent == 0) {
            std::cerr << " @ ";
            const auto& loc = range().first;
            std::cerr << loc.buffer().name() << ":";
            const auto line_col = diagnostic::compute_line_col(loc);
            std::cerr << line_col.first + 1 << ":";
            std::cerr << line_col.second + 1 << ": ";
        }
        std::cerr << "\n";

        const auto children = this->children();
        for (auto child : children) {
            child->dump(indent + 1);
        }
    }

    std::pair<location, location> string_literal_expr::range() const {
        return str_tok.range;
    }

    string_view string_literal_expr::body() const {
        return pp::analyze_string_literal(str_tok).body;
    }

    std::string string_literal_expr::get_dump_name() const {
        return "string literal";
    }

    std::string string_literal_expr::get_dump_info() const {
        return str_tok.spelling.to_string();
    }

    std::vector<const expr*> string_literal_expr::children() const {
        return {};
    }
}
