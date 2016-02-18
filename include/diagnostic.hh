#pragma once

#include "location.hh"

#include <string>
#include <vector>
#include <utility>

enum class diagnostic_id {
    pp_phase1_invalid_utf8,
    pp_phase2_missing_newline,
    opt_unrecognized,
    no_input_files,
    pp_phase3_undef_char_in_hdr_name_quote,
    pp_phase3_undef_char_in_hdr_name_angle,
    pp_phase3_partial_block_comment,
    pp_phase3_ambiguous_parse,
    cannot_open_file,
    pp_phase3_undef_stray_quote,
    source_file_extension_not_c,
};

enum class diagnostic_category {
    error,
    warning,
    undefined
};

struct diagnostic {
    std::string pattern;
    std::string citation;
    diagnostic_category category;
};

const diagnostic& find_diagnostic(diagnostic_id diag_id);
std::string format_diagnostic_message(const std::string& pattern,
                                      std::vector<std::string> args);

inline std::string to_string(const char* c) {
    return { c };
}

inline std::string to_string(const std::string& s) {
    return s;
}

template<typename... T>
std::string format_diagnostic_message(const std::string& pattern, T... t) {
    using ::to_string;
    using std::to_string;
    std::vector<std::string> args = { to_string(t)... };
    return format_diagnostic_message(pattern, std::move(args));
}

void emit_diagnostic(const diagnostic&, location, const std::string& msg);

template<typename... T>
void diagnose(diagnostic_id diag_id, location loc, T... t) {
    auto& diag = find_diagnostic(diag_id);
    auto msg = format_diagnostic_message(diag.pattern, t...);
    emit_diagnostic(diag, loc, msg);
}
