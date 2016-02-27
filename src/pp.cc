#include "pp.hh"
#include "utf8.hh"
#include "diagnostic.hh"
#include "util.hh"
#include "optional.hh"

#include <cassert>
#include <map>

using diagnostic::diagnose;
using util::starts_with;
using util::ends_with;
using util::reverse_adaptor;

std::map<std::string, std::string> trigraphs = {
    { R"(??=)", "#" },
    { R"(??()", "[" },
    { R"(??/)", "\\" },
    { R"(??))", "]" },
    { R"(??')", "^" },
    { R"(??<)", "{" },
    { R"(??!)", "|" },
    { R"(??>)", "}" },
    { R"(??-)", "~" },
};

std::unique_ptr<buffer> pp::perform_phase_one(std::unique_ptr<buffer> in) {
    /* [5.1.1.2]/1.1
     Physical source file multibyte characters are mapped, in an
     implementation-defined manner, to the source character set
     (introducing new-line characters for end-of-line indicators)
     if necessary. Trigraph sequences are replaced by corresponding
     single-character internal representations.
    */
    auto out = std::make_unique<derived_buffer>(std::move(in));
    while (!out->done()) {
        // map physical source file multibyte characters to the
        // source character set
        if (!utf8::is_ascii(out->peek()[0])) {
            auto utf32 = utf8::code_point_to_utf32(out->peek());
            if (utf32) {
                auto ucn = utf8::utf32_to_ucn(*utf32);
                out->replace(*utf8::measure_code_point(out->peek()), ucn);
            } else {
                location loc{*out->parent(), out->parent_index()};
                diagnose(diagnostic::id::pp1_invalid_utf8, loc);
                out->replace(1, "\\u001A"); // U+001A "SUBSTITUTE"
            }
            continue;
        }
        // introduce newline characters for end-of-line indicators
        if (out->peek().substr(0, 2) == "\r\n") {
            out->replace(2, "\n");
            continue;
        }
        // replace trigraph sequences with corresponding single-character
        // internal representations
        auto trigraph = trigraphs.find(out->peek().substr(0, 3).to_string());
        if (trigraph != trigraphs.end()) {
            out->replace(3, trigraph->second);
            continue;
        }

        out->propagate(1);
    }
    return std::move(out);
}

std::unique_ptr<buffer> pp::perform_phase_two(std::unique_ptr<buffer> in) {
    /* [5.1.1.2]/1.2
     Each instance of a backslash character (\) immediately followed
     by a new-line character is deleted, splicing physical source lines
     to form logical source lines. Only the last backslash on any physical
     source line shall be eligible for being part of such a splice. A
     source file that is not empty shall end in a new-line character,
     which shall not be immediately preceded by a backslash character
     before any such splicing takes place.
    */
    auto out = std::make_unique<derived_buffer>(std::move(in));
    while (!out->done()) {
        // delete each instance of a backslash character immediately
        // followed by a newline character
        if (out->peek().substr(0, 2) == "\\\n") {
            out->erase(2);
        } else {
            out->propagate(1);
        }
    }
    // a source file that is not empty shall end in a newline character,
    // which shall not be immediately preceded by a backslash character
    // before any such splicing takes place
    if (!out->parent()->data().empty() && !ends_with(out->data(), "\n")) {
        location loc{*out, out->data().size()};
        if (ends_with(out->parent()->data(), "\\\n")) {
            // if the file ended with a splice, point at the backslash
            // instead of the empty line
            loc = { *out->parent(), out->parent()->data().size() - 2 };
        }
        diagnose(diagnostic::id::pp2_missing_newline, loc);
        out->insert("\n");
    }
    return std::move(out);
}

string_view pp::lexer::peek() const {
    return buf.data().substr(index_);
}

bool pp::lexer::done() const {
    return index_ == buf.data().size();
}

void pp::lexer::select(token tok) {
    index_ += tok.spelling.size();
    tokens.push_back(std::move(tok));
}

bool pp::lexer::allow_header_name() const {
    /* [6.4]/4 
     header name preprocessing tokens are recognized
     only within #include preprocessing directives
    */
    bool found_include = false;
    bool found_hash = false;
    for (auto tok : reverse_adaptor(tokens)) {
        if (!found_include) {
            if (tok.spelling == "include") {
                found_include = true;
                continue;
            }
        } else if (!found_hash) {
            if (tok.is(punctuator::hash)) {
                found_hash = true;
                continue;
            }
        } else {
            if (tok.kind == token::newline) return true;
        }
        if (tok.is(token::space)) continue;
        if (tok.is(token::newline)) continue;
        return false;
    }
    return found_include && found_hash;
}

optional<token> pp::lexer::try_lex(token_kind kind, const std::regex& regex) {
    if (kind == token::header_name && !allow_header_name()) return {};
    std::cmatch match;
    bool matched = std::regex_search(
         peek().begin(),
         peek().end(),
         match,
         regex,
         std::regex_constants::match_continuous
    );
    if (matched) return token(kind, match, buf);
    else return {};
}

static std::map<token_kind, const std::regex*> pp_token_patterns = {
    { token::header_name, &pp::regex::header_name },
    { token::pp_number, &pp::regex::pp_number },
    { token::identifier, &pp::regex::identifier },
    { token::string_literal, &pp::regex::string_literal },
    { token::character_constant, &pp::regex::char_constant },
    { token::punctuator, &pp::regex::punctuator },
    { token::space, &pp::regex::space },
    { token::newline, &pp::regex::newline },
};

static std::vector<std::string> header_name_undef_seqs = {
    "'", "\"", "\\", "//", "/*"
};

std::vector<token> pp::perform_phase_three(const buffer& in) {
    lexer lexer{in};
    while (!lexer.done()) {
        /* [6.4]/4
         If the input stream has been parsed into preprocessing tokens up to
         a given character, the next preprocessing token is the longest
         sequence of characters that could constitute a preprocessing token.
        */
        std::vector<token> lexes;
        for (const auto pattern : pp_token_patterns) {
            auto tok = lexer.try_lex(pattern.first, *pattern.second);
            if (tok) lexes.push_back(std::move(*tok));
        }
        // if we didn't match anything, this is an "other" token
        if (lexes.empty()) {
            location loc{in, lexer.index()};
            std::pair<location, location> range{loc, loc.next_loc()};
            token other{token::other, lexer.peek().substr(0, 1), range};
            if (other.spelling == "'" || other.spelling == "\"") {
                const auto name = other.spelling == "'" ? "single" : "double";
                diagnose(diagnostic::id::pp3_unmatched_quote, loc, name);
            }
            lexes.push_back(std::move(other));
        }
        // sort tokens in descending order by spelling size
        std::sort(lexes.rbegin(), lexes.rend(), [](auto a, auto b) {
            return a.spelling.size() < b.spelling.size();
        });
        // if there were multiple equally long matches then we have an
        // ambiguity unless it is between a header name and a string literal
        if (lexes.size() > 1) {
            if (lexes[0].spelling.size() == lexes[1].spelling.size()) {
                /* [6.4]/4
                a sequence of characters that could be either a header
                name or a string literal is recognized as the former
                */
                bool exempt = true;
                exempt &= lexes[0].is(token::header_name);
                exempt &= lexes[1].is(token::string_literal);
                if (!exempt) {
                    const auto loc = lexes[0].range.first;
                    diagnose(diagnostic::id::pp3_ambiguous_lex, loc);
                }
            }
        }

        // clean up and diagnose chosen token
        auto tok = lexes[0];
        if (tok.is(token::punctuator)) {
            auto it = punctuator_table.find(tok.spelling.to_string());
            assert(it != punctuator_table.end()); // table doesn't match regex
            tok.punc = it->second;
        } else if (tok.is(token::space)) {
            if (starts_with(tok.spelling, "/*")) {
                if (!ends_with(tok.spelling, "*/")) {
                    const auto loc = tok.range.first;
                    diagnose(diagnostic::id::pp3_incomplete_comment, loc);
                }
            }
            tok.kind = token::newline;
        } else if (tok.is(token::header_name)) {
            /* [6.4.7]/3
             If the characters ', \, ", //, or / * occur in the sequence
             between the < and > delimiters, the behavior is undefined.
             Similarly, if the characters ', \, //, or / * occur in the
             sequence between the " delimiters, the behavior is undefined.
            */
            auto range = tok.spelling.substr(1, tok.spelling.size() - 2);
            // TODO implement find for string_view to avoid this allocation
            auto haystack = range.to_string();
            for (const auto seq : header_name_undef_seqs) {
                auto pos = haystack.find(seq);
                if (pos != std::string::npos) {
                    auto quote = seq == "'" ? "\"" : "'";
                    location loc = tok.range.first.next_loc(pos + 1);
                    diagnose(diagnostic::id::pp3_undef_char_in_hdr_name,
                             loc, quote + seq + quote);
                }
            }
        }
        
        lexer.select(tok);
    }
    return std::move(lexer.tokens);
}

