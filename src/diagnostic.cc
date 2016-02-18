#include "diagnostic.hh"
#include "buffer.hh"
#include "location.hh"
#include "color.hh"
#include "utf8.hh"
#include "options.hh"

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
    },
    {
        diagnostic_id::opt_unrecognized,
        {
            "unrecognized option '%%'",
            {},
            diagnostic_category::error
        }
    },
    {
        diagnostic_id::no_input_files,
        {
            "no input files",
            {},
            diagnostic_category::error
        }
    },
    {
        diagnostic_id::pp_phase3_undef_char_in_hdr_name_quote,
        {
            "if the characters ', \\, //, or /* occur in the"
            " sequence between the \" delimiters, the behavior is undefined",
            "[6.4.7]/3",
            diagnostic_category::undefined
        }
    },
    {
        diagnostic_id::pp_phase3_undef_char_in_hdr_name_angle,
        {
            "if the characters ', \\, \", //, or /* occur in the"
            " sequence between the < and > delimiters, the behavior"
            " is undefined",
            "[6.4.7]/3",
            diagnostic_category::undefined
        }
    },
    {
        diagnostic_id::pp_phase3_partial_block_comment,
        {
            "a source file shall not end in a partial comment",
            "[5.1.1.2]/1.3",
            diagnostic_category::error
        }
    },
    {
        diagnostic_id::pp_phase3_ambiguous_parse,
        {
            "ambiguous parse: '%%'",
            {},
            diagnostic_category::error
        }
    },
    {
        diagnostic_id::cannot_open_file,
        {
            "cannot open file '%%'",
            {},
            diagnostic_category::error
        }
    },
    {
        diagnostic_id::pp_phase3_undef_stray_quote,
        {
            "if a ' or a \" character does not lexically"
            " match a preprocessing token,"
            " the behavior is undefined",
            "[6.4]/3",
            diagnostic_category::undefined
        }
    },
    {
        diagnostic_id::source_file_extension_not_c,
        {
            "source file '%%' does not have a '.c' extension",
            {},
            diagnostic_category::warning
        }
    },
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
        case diagnostic_category::undefined: return "undefined-behavior";
    }
}

color get_category_color(diagnostic_category cat) {
    switch (cat) {
        case diagnostic_category::error: return color::red;
        case diagnostic_category::warning: return color::yellow;
        case diagnostic_category::undefined: return color::magenta;
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
    if (program_options.run_internal_tests) return;
    if (loc) loc = loc.spelling();
    if (loc) emit_file_line_col(loc);
    emit_category_message(diag.category, msg);
    if (!diag.citation.empty()) std::cout << " " << diag.citation;
    std::cout << "\n";
    if (loc) emit_snippet_caret(loc);
}
