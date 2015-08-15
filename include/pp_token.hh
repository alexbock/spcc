#pragma once

#include "location.hh"
#include "punctuator.hh"

#include <new>
#include <map>
#include <string>
#include <utility>
#include <cstddef>
#include <type_traits>
#include <cassert>

/* [6.4]/1
preprocessing-token:
  header-name
  identifier
  pp-number
  character-constant
  string-literal
  punctuator
  each non-white-space character that cannot be one of the above
*/
/* [6.4]*69
An additional category, placemarkers, is used internally in
translation phase 4; it cannot occur in source files.
*/
/* [6.4]/3
Preprocessing tokens can be separated by white space
*/
enum class pp_token_kind {
    header_name,
    identifier,
    pp_number,
    character_constant,
    string_literal,
    punctuator,
    other_non_whitespace,
    placemarker,
    whitespace
};

// [6.4.7]/1
enum class header_name_kind {
    quote,
    angle
};

// [6.4.4.4]/1
enum class character_constant_prefix {
    none,
    L,
    u,
    U
};

// [6.4.5]/1
enum class string_literal_prefix {
    none,
    u8,
    u,
    U,
    L
};

struct pp_token {
    pp_token_kind kind;
    std::string spelling;
    std::pair<location, location> range;
    bool valid = false;

    struct variant_base {
        virtual ~variant_base() = 0;
    };

    struct header_name : variant_base {
        static const auto token_kind = pp_token_kind::header_name;
        header_name(std::string name, header_name_kind style) :
        name{std::move(name)}, style{style} { }
        std::string name;
        header_name_kind style;
    };

    struct identifier : variant_base {
        static const auto token_kind = pp_token_kind::identifier;
    };

    struct pp_number : variant_base {
        static const auto token_kind = pp_token_kind::pp_number;
    };

    struct character_constant : variant_base {
        static const auto token_kind = pp_token_kind::character_constant;
        character_constant(character_constant_prefix prefix,
                       std::string body) :
        prefix{prefix}, body{std::move(body)} { }
        character_constant_prefix prefix;
        std::string body;
    };

    struct string_literal : variant_base {
        static const auto token_kind = pp_token_kind::string_literal;
        string_literal(string_literal_prefix prefix,
                       std::string body) :
        prefix{prefix}, body{std::move(body)} { }
        string_literal_prefix prefix;
        std::string body;
    };

    struct punctuator : variant_base {
        static const auto token_kind = pp_token_kind::punctuator;
        punctuator(punctuator_kind kind) : kind{kind} { }
        punctuator_kind kind;
    };

    struct other_non_whitespace : variant_base {
        static const auto token_kind = pp_token_kind::other_non_whitespace;
    };

    struct placemarker : variant_base {
        static const auto token_kind = pp_token_kind::placemarker;
    };

    struct whitespace : variant_base {
        static const auto token_kind = pp_token_kind::whitespace;
        whitespace(bool newline) : newline{newline} { }
        bool newline;
    };

    template<typename T>
    pp_token(std::string spelling,
             std::pair<location, location> range,
             T&& obj);
    explicit pp_token(std::nullptr_t) : valid{false} { }
    pp_token(const pp_token&) = delete;
    pp_token(pp_token&&);
    pp_token& operator=(pp_token&&);
    template<typename T>
    T& as();
    ~pp_token();
    operator bool() const { return valid; }
private:
    std::aligned_union_t<0,
        header_name, identifier, pp_number, character_constant,
        string_literal, punctuator, other_non_whitespace,
        placemarker, whitespace
    > variant;
};

template<typename T>
pp_token::pp_token(std::string spelling,
                   std::pair<location, location> range,
                   T&& obj) :
spelling{spelling},
range{range} {
    new (&variant) std::remove_reference_t<T>(std::forward<T>(obj));
    kind = std::remove_reference_t<T>::token_kind;
    valid = true;
}

template<typename T>
T& pp_token::as() {
    assert(kind == T::token_kind);
    return reinterpret_cast<T&>(variant);
}
