#include "pp_token.hh"
#include "util.hh"

#include <cstring>

pp_token::variant_base::~variant_base() = default;

pp_token::pp_token(pp_token&& other) {
    *this = std::move(other);
}

pp_token& pp_token::operator=(pp_token&& other) {
    kind = other.kind;
    spelling = other.spelling;
    range = other.range;
    std::memcpy(&variant, &other.variant, sizeof(variant));
    valid = other.valid;
    other.valid = false;
    return *this;
}

pp_token::~pp_token() {
    if (!valid) return;
    reinterpret_cast<variant_base&>(variant).~variant_base();
}

pp_token* peek(std::vector<pp_token>& tokens,
               std::size_t offset,
               lex_behavior::mode spaces,
               lex_behavior::mode newlines,
               bool reverse,
               std::size_t* out_index,
               std::size_t initial_skip) {
    using namespace lex_behavior;
    std::size_t non_ignored_tokens_skipped = 0;
    for (std::size_t i = initial_skip; i < tokens.size(); ++i) {
        const auto j = maybe_reverse_index(i, tokens.size(), reverse);
        bool ignore = false;
        ignore |= (spaces == SKIP && tokens[j].spelling == " ");
        ignore |= (newlines == SKIP && tokens[j].spelling == "\n");
        bool stop = false;
        stop |= (spaces == STOP && tokens[j].spelling == " ");
        stop |= (newlines == STOP && tokens[j].spelling == "\n");
        if (stop) return nullptr;
        if (!ignore && non_ignored_tokens_skipped++ == offset) {
            if (out_index) *out_index = j + 1;
            return &tokens[j];
        }
    }
    return nullptr;
}
