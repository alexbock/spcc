#include "diagnostic.hh"
#include "platform.hh"
#include "options.hh"
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
        {
            id::cannot_open_file,
            {
                "cannot open file '%%'",
                {},
                category::error
            }
        },
        {
            id::input_file_not_dot_c,
            {
                "input file '%%' does not have a '.c' extension",
                {},
                category::warning
            }
        },
        {
            id::not_yet_implemented,
            {
                "not yet implemented: %%",
                {},
                category::error
            }
        },
        {
            id::pp1_invalid_utf8,
            {
                "invalid UTF-8",
                "[5.1.1.2]/1.1",
                category::error
            }
        },
        {
            id::pp2_missing_newline,
            {
                "missing newline at end of file",
                "[5.1.1.2]/1.2",
                category::error
            }
        },
        {
            id::pp3_unmatched_quote,
            {
                "%% quote did match any preprocessing token",
                "[6.4]/3",
                category::undefined
            }
        },
        {
            id::pp3_ambiguous_lex,
            {
                "interpretation of character sequence as "
                "a preprocessing token is ambiguous",
                "[6.4]",
                category::error
            }
        },
        {
            id::pp3_incomplete_comment,
            {
                "incomplete multiline comment",
                "[5.1.1.2]/1.3",
                category::error
            }
        },
        {
            id::pp3_undef_char_in_hdr_name,
            {
                "use of %% in a header name",
                "[6.4.7]/3",
                category::undefined
            }
        },
        {
            id::pp4_error_directive,
            {
                "#error directive: %%",
                "[6.10.5]",
                category::error
            }
        },
        {
            id::pp4_unknown_pragma,
            {
                "unrecognized #pragma directive",
                "[6.10.6]",
                category::warning
            }
        },
        {
            id::pp4_expected_macro_name,
            {
                "expected macro name",
                "[6.10]",
                category::error
            }
        },
        {
            id::pp4_extra_after_directive,
            {
                "extra tokens after #%% directive",
                "[6.10]",
                category::error
            }
        },
        {
            id::pp4_macro_redef,
            {
                "macro '%%' redefined differently",
                "[6.10.3]/2",
                category::error
            }
        },
        {
            id::pp4_duplicate_macro_param,
            {
                "duplicate macro parameter name '%%'",
                "[6.10.3]/6",
                category::error
            }
        },
        {
            id::pp4_unexpected_macro_param,
            {
                "unexpected token in function-like macro parameter list",
                "[6.10.3]",
                category::error
            }
        },
        {
            id::pp4_missing_macro_space,
            {
                "missing whitespace before replacement list",
                "[6.10.3]/3",
                category::error
            }
        },
        {
            id::pp4_missing_macro_right_paren,
            {
                "expected right parenthesis to terminate function-like "
                "macro parameter list",
                "[6.10.3]",
                category::error
            }
        },
        {
            id::pp4_missing_macro_args_end,
            {
                "expected right parenthesis to terminate function-like "
                "macro invocation",
                "[6.10.3]",
                category::error
            }
        },
        {
            id::pp4_wrong_arity_macro_args,
            {
                "function-like macro '%%' requires %% argument%%, but "
                "%% %% provided",
                "[6.10.3]",
                category::error
            }
        },
        {
            id::pp4_cannot_use_hash_hash_here,
            {
                "## cannot be used here",
                "[6.10.3.3]",
                category::error
            }
        },
        {
            id::pp4_stringize_invalid_token,
            {
                "use of # operator did not produce a valid "
                "character string literal",
                "[6.10.3.2]/2",
                category::undefined
            }
        },
        {
            id::pp4_stringize_no_parameter,
            {
                "# must be followed a parameter name",
                "[6.10.3.2]/2",
                category::error
            }
        },
        {
            id::pp4_concatenate_invalid_token,
            {
                "use of ## operator did not produce a valid "
                "preprocessing token",
                "[6.10.3.3]/3",
                category::undefined
            }
        },
        {
            id::pp4_cannot_use_predef_macro_here,
            {
                "cannot use predefined macro name '%%' here",
                "[6.10.8]/2",
                category::error
            }
        },
        {
            id::pp4_predef_expand_failure,
            {
                "failed to expand dynamic predefined macro '%%'",
                {},
                category::error
            }
        },
        {
            id::pp4_mismatched_cond_directive,
            {
                "mismatched #%% directive",
                "[6.10.1]",
                category::error
            }
        },
        {
            id::pp4_cannot_use_va_args_here,
            {
                "cannot use __VA_ARGS__ here",
                "[6.10.3]/5",
                category::error
            }
        },
        {
            id::pp4_non_directive_ignored,
            {
                "non-directive ignored",
                "[6.10]",
                category::warning
            }
        },
        {
            id::pp4_too_many_nested_includes,
            {
                "too many nested #include directives",
                {},
                category::error
            }
        },
        {
            id::pp6_cannot_concatenate_wide_utf8,
            {
                "cannot concatenate UTF-8 and wide string literals",
                "[6.4.5]/2",
                category::error
            }
        },
        {
            id::pp6_cannot_concatenate_diff_wide,
            {
                "cannot concatenate wide string literals of different "
                "character sizes",
                "[6.4.5]/5",
                category::error
            }
        },
        {
            id::translation_limit_exceeded,
            {
                "minimum translation limit exceeded: %% %%",
                "[5.2.4.1]",
                category::warning
            }
        },
        {
            id::pp_token_is_not_a_valid_token,
            {
                "preprocessing token could not be converted into a token",
                "[6.4]/2",
                category::error
            }
        },
        {
            id::aux_previous_def,
            {
                "previous definition is here",
                {},
                category::auxiliary
            }
        },
        {
            id::aux_previous_use,
            {
                "previous use is here",
                {},
                category::auxiliary
            }
        },
        {
            id::aux_expanded_here,
            {
                "expanded from here",
                {},
                category::auxiliary
            }
        },
        {
            id::aux_included_here,
            {
                "in file included here",
                {},
                category::auxiliary
            }
        },
        {
            id::aux_macro_defined_here,
            {
                "macro '%%' defined here",
                {},
                category::auxiliary
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
        set_color(stdout, color::white);
        std::cout << loc.buffer().name() << ":";
        auto line_col = compute_line_col(loc);
        std::cout << line_col.first + 1 << ":";
        std::cout << line_col.second + 1 << ": ";
        reset_attributes(stdout);
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
        std::size_t lno = 0;
        std::size_t line_start = 0;
        std::size_t i;
        for (i = 0; lno < line; ++i) {
            if (data[i] == '\n') {
                ++lno;
                line_start = i + 1;
            }
        }
        std::size_t j = i;
        while (data[j] != '\n' && j < data.size()) ++j;
        return data.substr(line_start, j - line_start);
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

    void update_exit_code(category cat) {
        switch (cat) {
            case category::error:
                options::state.exit_code = 1;
            default:
                break;
        }
    }

    void emit_diagnostic(const info& info,
                         optional<location> loc,
                         const std::string& msg) {
        auto original_loc = loc;
        update_exit_code(info.category);
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

        if (loc && loc->buffer().included_at()) {
            diagnose(id::aux_included_here, *loc->buffer().included_at());
        }
        if (original_loc && original_loc->expanded_from) {
            diagnose(id::aux_expanded_here, *original_loc->expanded_from);
        }
    }
}