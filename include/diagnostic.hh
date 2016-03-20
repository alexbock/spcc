#ifndef SPCC_DIAGNOSTIC_HH
#define SPCC_DIAGNOSTIC_HH

#include "buffer.hh"
#include "string_view.hh"
#include "optional.hh"

#include <string>
#include <vector>
#include <utility>

namespace diagnostic {
    enum class category {
        error,
        warning,
        auxiliary,
        undefined,
    };

    enum class id {
        no_input_files,
        cannot_open_file,
        input_file_not_dot_c,
        invalid_option,
        invalid_size,
        not_yet_implemented,
        pp1_invalid_utf8,
        pp2_missing_newline,
        pp3_unmatched_quote,
        pp3_ambiguous_lex,
        pp3_incomplete_comment,
        pp3_undef_char_in_hdr_name,
        pp4_error_directive,
        pp4_unknown_pragma,
        pp4_expected_macro_name,
        pp4_extra_after_directive,
        pp4_macro_redef,
        pp4_duplicate_macro_param,
        pp4_unexpected_macro_param,
        pp4_missing_macro_space,
        pp4_missing_macro_right_paren,
        pp4_missing_macro_args_end,
        pp4_wrong_arity_macro_args,
        pp4_cannot_use_hash_hash_here,
        pp4_stringize_invalid_token,
        pp4_stringize_no_parameter,
        pp4_concatenate_invalid_token,
        pp4_cannot_use_predef_macro_here,
        pp4_predef_expand_failure,
        pp4_mismatched_cond_directive,
        pp4_cannot_use_va_args_here,
        pp4_non_directive_ignored,
        pp4_too_many_nested_includes,
        pp6_cannot_concatenate_wide_utf8,
        pp6_cannot_concatenate_diff_wide,
        pp7_expected_end_of_list,
        pp7_incomplete_list,
        pp7_expected_end_of_array_declarator,
        pp7_expected_ident_or_body,
        pp7_expected_semicolon,
        pp_token_is_not_a_valid_token,
        translation_limit_exceeded,
        aux_previous_def,
        aux_previous_use,
        aux_expanded_here,
        aux_included_here,
        aux_macro_defined_here,
    };

    struct info {
        std::string pattern;
        std::string citation;
        category category;
    };

    const info& find(id diag);
    std::string format_diagnostic_message(const std::string& pattern,
                                          std::vector<std::string> args);

    inline std::string to_string(const char* c) {
        return { c };
    }

    inline std::string to_string(const std::string& s) {
        return s;
    }

    inline std::string to_string(string_view v) {
        return v.to_string();
    }

    template<typename... T>
    std::string format_diagnostic_message(const std::string& pattern, T... t) {
        using diagnostic::to_string;
        using std::to_string;
        std::vector<std::string> args = { to_string(t)... };
        return format_diagnostic_message(pattern, std::move(args));
    }

    void emit_diagnostic(const info&, optional<location>,
                         const std::string& msg);

    std::pair<std::size_t, std::size_t> compute_line_col(location loc);

    template<typename... T>
    void diagnose(id diag, optional<location> loc, T... t) {
        auto& info = find(diag);
        auto msg = format_diagnostic_message(info.pattern, t...);
        emit_diagnostic(info, loc, msg);
    }
}

#endif
