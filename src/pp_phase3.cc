#include "pp.hh"
#include "token.hh"
#include "buffer.hh"
#include "punctuator.hh"
#include "diagnostic.hh"
#include "util.hh"

#include <set>
#include <cctype>
#include <cassert>
#include <utility>
#include <experimental/optional>

using std::experimental::optional;
using std::experimental::nullopt;

optional<token> lex_header_name(buffer& src, std::size_t index) {
    // [6.4.7]
    auto data = src.view();
    header_name_kind kind;
    if (data.substr(index, 1) == "<") kind = header_name_kind::angle;
    else if (data.substr(index, 1) == "\"") kind = header_name_kind::quote;
    else return nullopt;
    auto end = data.find(kind == header_name_kind::quote ? '"' : '>', index + 1);
    if (end == std::string::npos) return nullopt;
    string_view filename = data.substr(index + 1, end - (index + 1));
    if (filename.find('\n') != std::string::npos) {
        return nullopt;
    }
    // TODO need a separate validation step for undefined chars
    // (we also need to report on invalid UCNs in identifiers so
    // a general mechanism to emit diagnostics from these tentative
    // lexing functions would be good)
    string_view spelling = data.substr(index, end - index + 1);
    std::pair<location, location> range{{ src, index }, { src, end + 1 }};
    token::header_name hn{filename, kind};
    return token{spelling, range, hn};
}

string_view lex_ucn(buffer& src, std::size_t index) {
    // [6.4.3]/1
    int digits;
    if (src.view().substr(index, 2) == "\\u") {
        digits = 4;
    } else if (src.view().substr(index, 2) == "\\U") {
        digits = 8;
    } else return "";

    string_view hex = src.view().substr(index + 2, digits);
    if (hex.size() != digits) return "";
    for (auto c : hex) {
        if (!std::isxdigit(c)) return "";
    }
    return src.view().substr(index, digits + 2);
}

string_view lex_identifier_nondigit(buffer& src, std::size_t index) {
    auto ucn = lex_ucn(src, index);
    if (!ucn.empty()) return ucn;
    auto next = src.view().substr(index, 1);
    if (next.empty()) return "";
    char c  = next[0];
    if (std::isalpha(c) || c == '_') return next;
    else return "";
}

string_view lex_digit(buffer& src, std::size_t index) {
    auto next = src.view().substr(index, 1);
    if (next.empty()) return "";
    char c  = next[0];
    if (std::isdigit(c)) return next;
    else return "";
}

optional<token> lex_identifier(buffer& src, std::size_t index) {
    // [6.4.2.1]/1
    auto head = lex_identifier_nondigit(src, index);
    if (head.empty()) return nullopt;
    std::size_t size = head.size();
    for (;;) {
        auto digit = lex_digit(src, index + size);
        if (!digit.empty()) {
            size += digit.size();
            continue;
        }
        auto nondigit = lex_identifier_nondigit(src, index + size);
        if (!nondigit.empty()) {
            size += nondigit.size();
            continue;
        }
        break;
    }
    auto name = src.view().substr(index, size);
    std::pair<location, location> range{
        { src, index }, { src, index + name.size() }
    };
    return token{name, range, token_kind::identifier};
}

string_view lex_pp_number_exp(buffer& src, std::size_t index) {
    auto sub = src.view().substr(index, 2);
    if (sub.size() != 2) return "";
    bool good = true;
    good &= sub[0] == 'e' || sub[0] == 'E' || sub[0] == 'p' || sub[0] == 'P';
    good &= sub[1] == '+' || sub[1] == '-';
    if (good) return sub;
    return "";
}

optional<token> lex_pp_number(buffer& src, std::size_t index) {
    /* [6.4.8]/1
     pp-number:
       digit
       . digit
       pp-number digit
       pp-number identifier-nondigit
       pp-number e sign
       pp-number E sign
       pp-number p sign
       pp-number P sign
       pp-number .
    */
    std::size_t size = 0;
    if (src.view().substr(index, 1) == ".") {
        ++size;
    }
    auto digit = lex_digit(src, index + size);
    if (digit.empty()) return nullopt;
    for (;;) {
        auto exp = lex_pp_number_exp(src, index + size);
        if (!exp.empty()) {
            size += exp.size();
            continue;
        }
        auto nondigit = lex_identifier_nondigit(src, index + size);
        if (!nondigit.empty()) {
            size += nondigit.size();
            continue;
        }
        if (src.view().substr(index + size, 1) == ".") {
            ++size;
            continue;
        }
        auto digit = lex_digit(src, index + size);
        if (!digit.empty()) {
            size += digit.size();
            continue;
        }
        break;
    }
    string_view spelling = src.view().substr(index, size);
    std::pair<location, location> range{
        { src, index }, { src, index + spelling.size() }
    };
    return token{spelling, range, token_kind::pp_number};
}

optional<token> lex_punctuator(buffer& src, std::size_t index) {
    // [6.4.6]/1
    for (std::size_t i = punctuator_max_length; i != 0; --i) {
        auto it = punctuator_table.find(src.data.substr(index, i));
        if (it != punctuator_table.end()) {
            std::pair<location, location> range{
                { src, index }, { src, index + i }
            };
            auto spelling = src.view().substr(index, i);
            return token{spelling, range, it->second};
        }
    }
    return nullopt;
}

static std::set<std::string> simple_escape_sequences = {
    "\\'", "\\\"", "\\?", "\\\\",
    "\\a", "\\b", "\\f", "\\n", "\\r", "\\t", "\\v"
};

string_view lex_simple_escape(buffer& src, std::size_t index) {
    auto it = simple_escape_sequences.find(src.data.substr(index, 2));
    if (it != simple_escape_sequences.end()) {
        return src.view().substr(index, 2);
    }
    else return "";
}

static bool is_octal(char c) {
    return c >= '0' && c <= '7';
}

string_view lex_octal_escape(buffer& src, std::size_t index) {
    auto text = src.view().substr(index, 4);
    if (text.size() < 2 || text[0] != '\\') return "";
    if (!is_octal(text[1])) return "";
    std::string result = "\\" + std::string{text[1]};
    for (std::size_t i = 2; i < 4; ++i) {
        if (!is_octal(text[i])) break;
        result += text[i];
    }
    return src.view().substr(index, result.size());
}

string_view lex_hex_escape(buffer& src, std::size_t index) {
    if (src.view().substr(index, 2) != "\\x") return "";
    std::size_t i = 2;
    while (i < src.view().size() && std::isxdigit(src.data[i])) {
        ++i;
    }
    if (i == 2) return "";
    return src.view().substr(index, i);
}

string_view lex_escape(buffer& src, std::size_t index) {
    auto simple = lex_simple_escape(src, index);
    if (!simple.empty()) return simple;
    auto octal = lex_octal_escape(src, index);
    if (!octal.empty()) return octal;
    auto hex = lex_hex_escape(src, index);
    if (!hex.empty()) return hex;
    auto ucn = lex_ucn(src, index);
    if (!ucn.empty()) return ucn;
    return "";
}

optional<token> lex_character_constant(buffer& src, std::size_t index) {
    // [6.4.4.4]/1
    std::size_t prefix_size = 0;
    character_constant_prefix prefix = character_constant_prefix::none;
    auto head = src.view().substr(index, 1);
    if (head == "L") prefix = character_constant_prefix::L;
    else if (head == "u") prefix = character_constant_prefix::u;
    else if (head == "U") prefix = character_constant_prefix::U;
    if (prefix != character_constant_prefix::none) {
        prefix_size = 1;
    }
    if (src.view().substr(index + prefix_size, 1) != "'") {
        return nullopt;
    }

    std::size_t body_start = index + prefix_size + 1;
    std::size_t i = 0;
    for (;;) {
        auto esc = lex_escape(src, body_start + i);
        if (!esc.empty()) {
            i += esc.size();
            continue;
        }
        auto next = src.view().substr(body_start + i, 1);
        if (next != "'" && next != "\\" && next != "\n") {
            ++i;
            continue;
        }
        break;
    }
    if (src.view().substr(body_start + i, 1) != "'") {
        return nullopt;
    }
    auto body = src.view().substr(body_start, i);
    std::pair<location, location> range{
        { src, index }, { src, body_start + i + 1 }
    };
    token::character_constant cc{prefix, body};
    auto spelling = src.view().substr(index, body_start + i + 1 - index);
    return token(spelling, range, cc);
}

optional<token> lex_string_literal(buffer& src, std::size_t index) {
    // [6.4.5]/1
    string_literal_prefix prefix = string_literal_prefix::none;
    auto head = src.view().substr(index, 2);
    if (starts_with(head, "u8")) prefix = string_literal_prefix::u8;
    else if (starts_with(head, "u")) prefix = string_literal_prefix::u;
    else if (starts_with(head, "U")) prefix = string_literal_prefix::U;
    else if (starts_with(head, "L")) prefix = string_literal_prefix::L;
    std::size_t prefix_size = 0;
    if (prefix == string_literal_prefix::u8) prefix_size = 2;
    else if (prefix != string_literal_prefix::none) prefix_size = 1;
    if (src.view().substr(index + prefix_size, 1) != "\"") {
        return nullopt;
    }
    std::size_t body_start = index + prefix_size + 1;
    std::size_t i = 0;
    for (;;) {
        auto esc = lex_escape(src, body_start + i);
        if (!esc.empty()) {
            i += esc.size();
            continue;
        }
        auto next = src.view().substr(body_start + i, 1);
        if (next != "\"" && next != "\\" && next != "\n") {
            ++i;
            continue;
        }
        break;
    }
    if (src.view().substr(body_start + i, 1) != "\"") {
        return nullopt;
    }
    string_view body = src.view().substr(body_start, i);
    std::pair<location, location> range{
        { src, index }, { src, body_start + i + 1 }
    };
    token::string_literal str{prefix, body};
    auto spelling = src.view().substr(index, body_start + i + 1 - index);
    return token(spelling, range, str);
}

std::size_t measure_comment(buffer& src, std::size_t index) {
    // [6.4.9]/1-2
    if (src.view().substr(index, 2) == "//") {
        auto newline_pos = src.data.find('\n', index);
        assert(newline_pos != std::string::npos);
        return newline_pos - index;
    } else if (src.view().substr(index, 2) == "/*") {
        auto end_pos = src.data.find("*/", index);
        if (end_pos == std::string::npos) {
            location loc{src, index};
            diagnose(diagnostic_id::pp_phase3_partial_block_comment, loc);
            return src.data.size() - index;
        }
        return end_pos + 2 - index;
    } else return 0;
}

std::size_t measure_non_newline_whitespace(buffer& src, std::size_t index) {
    std::size_t size = 0;
    for (;;) {
        auto next = src.view().substr(index + size, 1);
        if (next.empty()) break;
        if (next[0] == '\n') break;
        if (!std::isspace(next[0])) break;
        ++size;
    }
    return size;
}

optional<token> lex_whitespace(buffer& src, std::size_t index) {
    /* [5.1.1.2]/1.3
    Each comment is replaced by one space character. New-line characters
    are retained. Whether each nonempty sequence of white-space characters
    other than new-line is retained or replaced by one space character is
    implementation-defined.
    */
    if (src.view().substr(index, 1) == "\n") {
        std::pair<location, location> range{
            { src, index }, { src, index + 1 }
        };
        return token(src.view().substr(index, 1),
                     range, token_kind::whitespace);
    }
    std::size_t size = 0;
    for (;;) {
        std::size_t ws = measure_non_newline_whitespace(src, index + size);
        size += ws;
        if (ws) continue;
        std::size_t comment = measure_comment(src, index + size);
        size += comment;
        if (comment) continue;
        break;
    }
    if (!size) return nullopt;
    std::pair<location, location> range{
        { src, index }, { src, index + size }
    };
    return token(src.view().substr(index, size), range, token_kind::whitespace);
}

using lexer_func = optional<token> (*)(buffer&, std::size_t);
static lexer_func lexer_funcs[] = {
    lex_identifier,
    lex_pp_number,
    lex_punctuator,
    lex_character_constant,
    lex_string_literal,
};

static bool allow_header_name(std::vector<token>& tokens) {
    /* [6.4]/4
    header name preprocessing tokens are recognized only within
    #include preprocessing directives
    */
    using namespace lex_behavior;
    bool found_include = false;
    if (auto prev = peek(tokens, 0, SKIP, STOP, true)) {
        if (prev->kind == token_kind::identifier) {
            if (prev->spelling == "include") {
                found_include = true;
            }
        }
    }
    if (!found_include) return false;
    bool found_hash = false;
    if (auto prev = peek(tokens, 1, SKIP, STOP, true)) {
        if (prev->kind == token_kind::punctuator) {
            if (prev->pk == punctuator_kind::hash) {
                found_hash = true;
            }
        }
    }
    if (!found_hash) return false;
    auto newline = peek(tokens, 2, SKIP, INCLUDE, true);
    return (!newline || newline->spelling == "\n");
}

static bool sort_tokens(const optional<token>& a, const optional<token>& b) {
    auto a_size = a ? a->spelling.size() : 0;
    auto b_size = b ? b->spelling.size() : 0;
    return a_size < b_size;
}

std::vector<token> perform_pp_phase3(buffer& src) {
    /* [5.1.1.2]/1.3
    The source file is decomposed into preprocessing tokens and
    sequences of white-space characters (including comments).
    */
    std::vector<token> tokens;
    std::size_t index = 0;
    while (index < src.data.size()) {
        if (auto ws = lex_whitespace(src, index)) {
            index += ws->spelling.size();
            if (ws->spelling != "\n") ws->spelling = " ";
            tokens.push_back(*ws);
            continue;
        }
        std::vector<optional<token>> tentative_lexes;
        if (allow_header_name(tokens)) {
            tentative_lexes.push_back(lex_header_name(src, index));
        }
        for (auto lexer : lexer_funcs) {
            tentative_lexes.push_back(lexer(src, index));
        }
        /* [6.4]/4
         If the input stream has been parsed into preprocessing tokens
         up to a given character, the next preprocessing token is the
         longest sequence of characters that could constitute a
         preprocessing token.
         */
        std::stable_sort(tentative_lexes.rbegin(),
                         tentative_lexes.rend(),
                         sort_tokens);
        assert(tentative_lexes.size() > 1);
        if (!tentative_lexes[0]) {
            std::pair<location, location> range{
                { src, index }, { src, index + 1 }
            };
            token tok{
                src.view().substr(index, 1),
                range,
                token_kind::other_non_whitespace
            };
            if (tok.spelling == "'" || tok.spelling == "\"") {
                auto loc = range.first;
                diagnose(diagnostic_id::pp_phase3_undef_stray_quote, loc);
            }
            tokens.push_back(std::move(tok));
            ++index;
            continue;
        }
        auto& first = tentative_lexes[0];
        auto& second = tentative_lexes[1];
        auto both_valid = true;
        both_valid &= (first && second);
        both_valid &= (first->spelling.size() == second->spelling.size());
        if (first && second && first->spelling.size() == second->spelling.size()) {
            /* [6.4]/4
            There is one exception to this rule: header name preprocessing
            tokens are recognized only within #include preprocessing
            directives and in implementation-defined locations within
            #pragma directives. In such contexts, a sequence of characters
            that could be either a header name or a string literal is
            recognized as the former.
            */
            bool header_name_won = first->kind == token_kind::header_name;
            bool string_second = second->kind == token_kind::string_literal;
            if (!(header_name_won && string_second)) {
                diagnose(diagnostic_id::pp_phase3_ambiguous_parse,
                         { src, index },
                         first->spelling);
            }
        }
        index += first->spelling.size();
        tokens.push_back(std::move(*first));
    }
    return tokens;
}
