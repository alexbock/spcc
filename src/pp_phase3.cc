#include "pp.hh"
#include "pp_token.hh"
#include "buffer.hh"
#include "punctuator.hh"
#include "diagnostic.hh"

#include <cctype>
#include <cassert>
#include <utility>

pp_token lex_header_name(buffer& src, std::size_t index) {
    // [6.4.7]
    const auto& data = src.data;
    header_name_kind kind;
    if (data.substr(index, 1) == "<") kind = header_name_kind::angle;
    else if (data.substr(index, 1) == "\"") kind = header_name_kind::quote;
    else return pp_token{nullptr};
    auto end = data.find(kind == header_name_kind::quote ? '"' : '>', index);
    if (end == std::string::npos) return pp_token{nullptr};
    std::string filename = data.substr(index + 1, end - (index + 1));
    if (filename.find('\n') != std::string::npos) {
        return pp_token{nullptr};
    }
    // TODO need a separate validation step for undefined chars
    std::string spelling = data.substr(index, end - index + 1);
    std::pair<location, location> range{{ src, index }, { src, end + 1 }};
    pp_token::header_name hn{filename, kind};
    return pp_token{std::move(spelling), range, std::move(hn)};
}

std::string lex_ucn(buffer& src, std::size_t index) {
    // [6.4.3]/1
    int digits;
    if (src.data.substr(index, 2) == "\\u") {
        digits = 4;
    } else if (src.data.substr(index, 2) == "\\U") {
        digits = 8;
    } else return "";

    std::string hex = src.data.substr(index + 2, digits);
    if (hex.size() != digits) return "";
    for (auto c : hex) {
        if (!std::isxdigit(c)) return "";
    }
    return src.data.substr(index, digits + 2);
}

std::string lex_identifier_nondigit(buffer& src, std::size_t index) {
    auto ucn = lex_ucn(src, index);
    if (!ucn.empty()) return ucn;
    auto next = src.data.substr(index, 1);
    if (next.empty()) return "";
    char c  = next[0];
    if (std::isalpha(c) || c == '_') return next;
    else return "";
}

std::string lex_digit(buffer& src, std::size_t index) {
    auto next = src.data.substr(index, 1);
    if (next.empty()) return "";
    char c  = next[0];
    if (std::isdigit(c)) return next;
    else return "";
}

pp_token lex_identifier(buffer& src, std::size_t index) {
    // [6.4.2.1]/1
    auto head = lex_identifier_nondigit(src, index);
    if (head.empty()) return pp_token{nullptr};
    std::string name = head;
    for (;;) {
        auto digit = lex_digit(src, index + name.size());
        if (!digit.empty()) {
            name += digit;
            continue;
        }
        auto nondigit = lex_identifier_nondigit(src, index + name.size());
        if (!nondigit.empty()) {
            name += nondigit;
            continue;
        }
        break;
    }
    std::pair<location, location> range{
        { src, index }, { src, index + name.size() }
    };
    return pp_token{name, range, pp_token::identifier{}};
}

std::string lex_dot_or_digit(buffer& src, std::size_t index) {
    auto digit = lex_digit(src, index);
    if (!digit.empty()) return digit;
    if (src.data.substr(index, 1) == ".") return ".";
    return "";
}

std::string lex_pp_number_exp(buffer& src, std::size_t index) {
    auto sub = src.data.substr(index, 2);
    if (sub.size() != 2) return "";
    bool good = true;
    good &= sub[0] == 'e' || sub[0] == 'E';
    good &= sub[0] == 'p' || sub[0] == 'P';
    good &= sub[1] == '+' || sub[1] == '-';
    if (good) return sub;
    return "";
}

pp_token lex_pp_number(buffer& src, std::size_t index) {
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
    auto spelling = lex_dot_or_digit(src, index);
    if (spelling.empty()) return pp_token{nullptr};
    for (;;) {
        auto dot_digit = lex_dot_or_digit(src, index + spelling.size());
        if (!dot_digit.empty()) {
            spelling += dot_digit;
            continue;
        }
        auto exp = lex_pp_number_exp(src, index + spelling.size());
        if (!exp.empty()) {
            spelling += exp;
            continue;
        }
        auto nondigit = lex_identifier_nondigit(src, index + spelling.size());
        if (!nondigit.empty()) {
            spelling += nondigit;
            continue;
        }
        break;
    }
    std::pair<location, location> range{
        { src, index }, { src, index + spelling.size() }
    };
    return pp_token{spelling, range, pp_token::pp_number{}};
}

pp_token lex_punctuator(buffer& src, std::size_t index) {
    // [6.4.6]/1
    for (std::size_t i = punctuator_max_length; i != 0; --i) {
        auto it = punctuator_table.find(src.data.substr(index, i));
        if (it != punctuator_table.end()) {
            std::pair<location, location> range{
                { src, index }, { src, index +i }
            };
            pp_token::punctuator punc{it->second};
            return pp_token{it->first, range, punc};
        }
    }
    return pp_token{nullptr};
}

std::size_t measure_comment(buffer& src, std::size_t index) {
    // [6.4.9]/1-2
    if (src.data.substr(index, 2) == "//") {
        auto newline_pos = src.data.find('\n', index);
        assert(newline_pos != std::string::npos);
        return newline_pos - index;
    } else if (src.data.substr(index, 2) == "/*") {
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
        auto next = src.data.substr(index + size, 1);
        if (next.empty()) break;
        if (next[0] == '\n') break;
        if (!std::isspace(next[0])) break;
        ++size;
    }
    return size;
}

pp_token lex_whitespace(buffer& src, std::size_t index) {
    /* [5.1.1.2]/1.3
    Each comment is replaced by one space character. New-line characters
    are retained. Whether each nonempty sequence of white-space characters
    other than new-line is retained or replaced by one space character is
    implementation-defined.
    */
    if (src.data.substr(index, 1) == "\n") {
        std::pair<location, location> range{
            { src, index }, { src, index + 1 }
        };
        pp_token::whitespace ws{true};
        return pp_token("\n", range, ws);
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
    if (!size) return pp_token{nullptr};
    std::pair<location, location> range{
        { src, index }, { src, index + size }
    };
    pp_token::whitespace ws{false};
    return pp_token(src.data.substr(index, size), range, ws);
}

using lexer_func = pp_token (*)(buffer&, std::size_t);
static lexer_func lexer_funcs[] = {
    lex_identifier,
    lex_pp_number,
    lex_punctuator
};

static bool allow_header_name(std::vector<pp_token>& tokens) {
    /* [6.4]/4
    header name preprocessing tokens are recognized only within
    #include preprocessing directives
    */
    if (tokens.empty()) return false;
    bool found_include = false;
    bool found_hash = false;
    for (std::size_t i = tokens.size() - 1; i > 0; --i) {
        if (found_include) {
            if (tokens[i].kind == pp_token_kind::punctuator) {
                auto punc = tokens[i].as<pp_token::punctuator>();
                if (punc.kind == punctuator_kind::hash) {
                    found_hash = true;
                    break;
                }
            }
            if (tokens[i].kind != pp_token_kind::whitespace) break;
        } else {
            if (tokens[i].kind == pp_token_kind::identifier) {
                if (tokens[i].spelling == "include") {
                    found_include = true;
                    continue;
                }
            }
            if (tokens[i].kind != pp_token_kind::whitespace) break;
        }
    }
    return found_hash;
}

static bool sort_pp_tokens(const pp_token& a, const pp_token& b) {
    return a.spelling.size() < b.spelling.size();
}

std::vector<pp_token> perform_pp_phase3(buffer& src) {
    std::vector<pp_token> tokens;
    std::size_t index = 0;
    while (index < src.data.size()) {
        if (auto ws = lex_whitespace(src, index)) {
            index += ws.spelling.size();
            tokens.push_back(std::move(ws));
            continue;
        }
        std::vector<pp_token> tentative_lexes;
        if (allow_header_name(tokens)) {
            tentative_lexes.push_back(lex_header_name(src, index));
        }
        for (auto lexer : lexer_funcs) {
            tentative_lexes.push_back(lexer(src, index));
        }
        std::sort(tentative_lexes.rbegin(),
                  tentative_lexes.rend(),
                  sort_pp_tokens);
        assert(tentative_lexes.size() > 1);
        if (tentative_lexes[0].spelling.empty()) {
            // TODO other non-whitespace
            assert("!TODO");
        }
        auto& token = tentative_lexes[0];
        if (token.spelling.size() == tentative_lexes[1].spelling.size()) {
            // TODO ambiguous parse error
            assert("!TODO");
        }
        index += token.spelling.size();
        tokens.push_back(std::move(token));
    }
    return tokens;
}
