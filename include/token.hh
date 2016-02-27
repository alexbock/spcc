#ifndef SPCC_TOKEN_HH
#define SPCC_TOKEN_HH

#include "buffer.hh"
#include "string_view.hh"
#include "punctuator.hh"
#include "optional.hh"

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
    };

    token(token_kind kind, string_view spelling,
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

    bool is(token_kind kind) const {
        return this->kind == kind;
    }

    token_kind kind;
    string_view spelling;
    std::pair<location, location> range;
    enum punctuator punc;
    bool blue = false; // ineligible for further macro replacement
};

using token_kind = token::token_kind;

#endif
