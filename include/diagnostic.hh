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
        aux_previous_def,
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

    void emit_diagnostic(const info&, optional<location>, const std::string& msg);

    template<typename... T>
    void diagnose(id diag, optional<location> loc, T... t) {
        auto& info = find(diag);
        auto msg = format_diagnostic_message(info.pattern, t...);
        emit_diagnostic(info, loc, msg);
    }
}

#endif
