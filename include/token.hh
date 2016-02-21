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
#include <vector>
#include <experimental/string_view>

using std::experimental::string_view;

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
enum class token_kind {
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

struct token {
    string_view spelling;
    token_kind kind;
    std::pair<location, location> range;

    struct header_name {
        string_view name;
        header_name_kind style;
    };

    struct character_constant {
        character_constant_prefix prefix;
        string_view body;
    };

    struct string_literal {
        string_literal_prefix prefix;
        string_view body;
    };

    union {
        header_name hn;
        character_constant cc;
        string_literal sl;
        punctuator_kind pk;
    };

    token(string_view spelling, std::pair<location, location> range,
          token_kind kind) :
    spelling{spelling}, kind{kind}, range{range} { }

    token(string_view spelling, std::pair<location, location> range,
          header_name hn) :
    spelling{spelling}, kind{token_kind::header_name},
    range{range}, hn{hn} { }

    token(string_view spelling, std::pair<location, location> range,
          character_constant cc) :
    spelling{spelling}, kind{token_kind::character_constant},
    range{range}, cc{cc} { }

    token(string_view spelling, std::pair<location, location> range,
          string_literal sl) :
    spelling{spelling}, kind{token_kind::string_literal},
    range{range}, sl{sl} { }

    token(string_view spelling, std::pair<location, location> range,
          punctuator_kind pk) :
    spelling{spelling}, kind{token_kind::punctuator},
    range{range}, pk{pk} { }
};

namespace lex_behavior {
    enum mode {
        SKIP,
        INCLUDE,
        STOP
    };
}

token* peek(std::vector<token>& tokens,
            std::size_t offset,
            lex_behavior::mode spaces,
            lex_behavior::mode newlines,
            bool reverse = true,
            std::size_t* out_index = nullptr,
            std::size_t initial_skip = 0);
