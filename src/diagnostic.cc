#include "diagnostic.hh"
#include "platform.hh"
#include "utf8.hh"

#include <map>
#include <iostream>

using namespace platform::stream;

namespace diagnostic {
    static std::map<id, info> diags = {
        {
            id::no_input_files,
            {
                "no input files",
                {},
                category::error
            }
        },
    };

    const info& find(id diag) {
        return diags[diag];
    }

    std::string format_diagnostic_message(const std::string& pattern,
                                          std::vector<std::string> args) {
        std::string result;
        std::size_t i = 0, arg = 0;
        while (i < pattern.size()) {
            std::size_t next = pattern.find("%%", i);
            if (next == std::string::npos) {
                result += pattern.substr(i);
                break;
            }
            result += pattern.substr(i, next - i);
            i = next + 2;
            assert(arg < args.size());
            result += args[arg++];
        }
        assert(arg == args.size());
        return result;
    }

    std::pair<std::size_t, std::size_t> compute_line_col(location loc) {
        std::size_t line = 0, col = 0;
        for (std::size_t i = 0; i < loc.offset(); ++i) {
            ++col;
            if (loc.buffer().data()[i] == '\n') {
                col = 0;
                ++line;
            }
        }
        return { line, col };
    }

    void emit_file_line_col(location loc) {
        std::cout << loc.buffer().name() << ":";
        auto line_col = compute_line_col(loc);
        std::cout << line_col.first + 1 << ":";
        std::cout << line_col.second + 1 << ": ";
    }

    std::string to_string(category cat) {
        switch (cat) {
            case category::error: return "error";
            case category::warning: return "warning";
            case category::auxiliary: return "auxiliary";
            case category::undefined: return "undefined-behavior";
        }
    }

    color get_category_color(category cat) {
        switch (cat) {
            case category::error: return color::red;
            case category::warning: return color::yellow;
            case category::auxiliary: return color::blue;
            case category::undefined: return color::magenta;
        }
    }

    void emit_category_message(category cat, const std::string& msg) {
        set_color(stdout, get_category_color(cat));
        set_style(stdout, style::bold);
        std::cout << to_string(cat) << ": ";
        set_color(stdout, color::white);
        std::cout << msg;
        reset_attributes(stdout);
    }

    std::string generate_caret_indent(std::size_t col, string_view line) {
        line = line.substr(0, col);
        std::string result;
        for (char c : line) {
            if (c == '\t') result += '\t';
            else if (!utf8::is_continuation(c)) result += ' ';
        }
        return result;
    }

    string_view compute_nth_line(string_view data, std::size_t line) {
        std::size_t lno = 1;
        std::size_t i;
        for (i = 0; lno < line; ++i) {
            if (data[i] == '\n') ++lno;
        }
        std::size_t j = i;
        while (data[j] != '\n' && j < data.size()) ++j;
        return data.substr(i, j - i);
    }

    void emit_snippet_caret(location loc) {
        auto line_col = compute_line_col(loc);
        auto line = compute_nth_line(loc.buffer().data(), line_col.first);
        std::cout << line << "\n";
        set_color(stdout, color::green);
        std::cout << generate_caret_indent(line_col.second, line) << "^";
        reset_attributes(stdout);
        std::cout << "\n";
    }

    void emit_diagnostic(const info& info,
                         optional<location> loc,
                         const std::string& msg) {
        if (loc) loc = loc->find_spelling_loc();
        if (loc) emit_file_line_col(*loc);
        emit_category_message(info.category, msg);
        if (!info.citation.empty()) {
            set_color(stdout, color::white);
            std::cout << " " << info.citation;
            reset_attributes(stdout);
        }
        std::cout << "\n";
        if (loc) emit_snippet_caret(*loc);
    }
}