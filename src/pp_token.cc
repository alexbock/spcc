#include "pp_token.hh"

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
