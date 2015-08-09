#include "diagnostic.hh"
#include "buffer.hh"
#include "location.hh"
#include "color.hh"
#include "utf8.hh"

#include <iostream>
#include <cassert>
#include <map>

static std::map<diagnostic_id, diagnostic> diags = {
    {
        diagnostic_id::pp_phase1_invalid_utf8,
        {
            "invalid UTF-8 sequence: %%",
            {},
            diagnostic_category::error
        }
    },
    {
        diagnostic_id::pp_phase2_missing_newline,
        {
            "a source file that is not empty shall end in a"
            " newline character, which shall not be immediately"
            " preceded by a backslash character before any such"
            " splicing takes place",
            "[5.1.1.2]/1.2",
            diagnostic_category::error
        }
    }
};

const diagnostic& find_diagnostic(diagnostic_id diag_id) {
    auto it = diags.find(diag_id);
    assert(it != diags.end());
    return it->second;
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

void emit_file_line_col(location loc) {
    std::cout << loc.buf->name << ":";
    auto line_col = loc.get_line_col();
    std::cout << line_col.first + 1 << ":";
    std::cout << line_col.second + 1 << ": ";
}

std::string to_string(diagnostic_category category) {
    switch (category) {
        case diagnostic_category::error: return "error";
        case diagnostic_category::warning: return "warning";
    }
}

color get_category_color(diagnostic_category cat) {
    switch (cat) {
        case diagnostic_category::error: return color::red;
        case diagnostic_category::warning: return color::yellow;
    }
}

void emit_category_message(diagnostic_category cat, const std::string& msg) {
    set_color(get_category_color(cat));
    std::cout << to_string(cat) << ": ";
    set_color(color::standard);
    std::cout << msg;
}

std::string generate_caret_indent(std::size_t col, std::string line) {
    line = line.substr(0, col);
    std::string result;
    for (char c : line) {
        if (c == '\t') result += '\t';
        else if (!utf8::is_continuation(c)) result += ' ';
    }
    return result;
}

void emit_snippet_caret(location loc) {
    auto line_col = loc.get_line_col();
    std::string line = loc.buf->get_line(line_col.first);
    std::cout << line << "\n";
    set_color(color::green);
    std::cout << generate_caret_indent(line_col.second, line) << "^";
    set_color(color::standard);
    std::cout << "\n";
}

void emit_diagnostic(const diagnostic& diag,
                     location loc,
                     const std::string& msg) {
    if (loc) loc = loc.spelling();
    if (loc) emit_file_line_col(loc);
    emit_category_message(diag.category, msg);
    if (!diag.citation.empty()) std::cout << " " << diag.citation;
    std::cout << "\n";
    if (loc) emit_snippet_caret(loc);
}
