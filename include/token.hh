#ifndef SPCC_TOKEN_HH
#define SPCC_TOKEN_HH

#include "buffer.hh"
#include "punctuator.hh"
#include "keyword.hh"

#include <optional>
#include <string_view>
#include <utility>
#include <regex>

inline auto range_from_match(const std::cmatch& match, const buffer& buf) {
    return std::pair<location, location>{
        location(buf, match[0].first - buf.data().begin()),
        location(buf, match[0].second - buf.data().begin())
    };
}

struct token {
    enum token_kind {
        header_name,
        identifier,
        pp_number,
        character_constant,
        string_literal,
        punctuator,
        other,
        space,
        newline,
        placemarker,
        keyword,
        floating_constant,
        integer_constant,
    };

    token(token_kind kind, std::string_view spelling,
          std::pair<location, location> range) :
    kind(kind), spelling(spelling), range(range) { }

    token(token_kind kind, const std::cmatch& match, const buffer& buf) :
    kind(kind), spelling(match[0].first, match[0].length()),
    range(range_from_match(match, buf)) {
    }

    bool is(enum punctuator punc) const {
        if (!is(punctuator)) return false;
        return this->punc == punc;
    }

    bool is(enum keyword kw) const {
        if (!is(keyword)) return false;
        return this->kw == kw;
    }

    bool is(token_kind kind) const {
        return this->kind == kind;
    }

    token_kind kind;
    std::string_view spelling;
    std::pair<location, location> range;
    union {
        enum punctuator punc;
        enum keyword kw;
    };
    bool blue = false; // ineligible for further macro replacement
};

using token_kind = token::token_kind;

#endif
