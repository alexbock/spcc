#include "pp.hh"
#include "utf8.hh"
#include "diagnostic.hh"
#include "util.hh"
#include "optional.hh"

#include <algorithm>
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
                    tok.kind = token::newline;
                }
            }
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

using p4m = pp::phase_four_manager;

optional<std::size_t> p4m::find(ws_mode space, ws_mode newline) {
    for (std::size_t offset = index; offset < tokens.size(); ++offset) {
        auto tok = tokens[offset];
        if (tok.is(token::space) || tok.is(token::newline)) {
            auto mode = tok.is(token::space) ? space : newline;
            switch (mode) {
                case SKIP: continue;
                case STOP: return {};
                case TAKE: return offset;
            }
        } else return offset;
    }
    return {};
}

optional<token> p4m::peek(ws_mode space, ws_mode newline) {
    auto offset = find(space, newline);
    if (offset) return tokens[*offset];
    else return {};
}

optional<token> p4m::get(ws_mode space, ws_mode newline) {
    auto offset = find(space, newline);
    if (offset) {
        index = *offset + 1;
        return tokens[*offset];
    } else return {};
}

std::vector<token> p4m::finish_line() {
    std::vector<token> tokens;
    for (auto tok = get(SKIP, STOP); tok; tok = get(SKIP, STOP)) {
        tokens.push_back(*tok);
    }
    assert(get(SKIP, TAKE)->is(token::newline));
    return tokens;
}

void p4m::finish_directive_line(token name) {
    if (!finish_line().empty()) {
        diagnose(diagnostic::id::pp4_extra_after_directive,
                 name.range.first, name.spelling);
    }
}

void p4m::maybe_diagnose_macro_redefinition(const macro& def) const {
    auto old = macros.find(def.name);
    if (old == macros.end()) return;
    bool bad = false;
    if (def.function_like != old->second.function_like) bad = true;
    if (def.variadic != old->second.variadic) bad = true;
    if (def.body.size() != old->second.body.size()) bad = true;
    for (std::size_t i = 0; i < def.body.size(); ++i) {
        if (def.body[i].spelling != old->second.body[i].spelling) {
            bad = true;
            break;
        }
    }
    if (def.param_names.size() != old->second.param_names.size()) bad = true;
    for (std::size_t i = 0; i < def.param_names.size(); ++i) {
        if (def.param_names[i] != old->second.param_names[i]) {
            bad = true;
            break;
        }
    }
    if (bad) {
        diagnose(diagnostic::id::pp4_macro_redef, def.loc, def.name);
        diagnose(diagnostic::id::aux_previous_def, old->second.loc);
    }
}

optional<std::vector<token>> p4m::maybe_expand_macro() {
    auto next = peek(SKIP, SKIP);
    if (!next || !next->is(token::identifier) || next->blue) return {};
    auto loc = next->range.first;
    auto it = macros.find(next->spelling);
    if (it == macros.end()) return {};
    auto& mac = it->second;
    if (mac.being_replaced) {
        tokens[*find(SKIP, SKIP)].blue = true;
        return {};
    }
    next = get(SKIP, SKIP);
    if (mac.function_like) {
        auto lparen = peek(SKIP, SKIP);
        if (!lparen || !lparen->is(punctuator::paren_left)) {
            return std::vector<token>{*next};
        }
        get(SKIP, SKIP);
        // collect arguments
        std::vector<std::vector<token>> args;
        std::vector<token> arg;
        std::size_t inner_parens = 0;
        bool done = false;
        for (auto tok = get(TAKE, TAKE); tok; tok = get(TAKE, TAKE)) {
            if (tok->is(punctuator::paren_right) && inner_parens) {
                --inner_parens;
            } else if (tok->is(punctuator::paren_left)) {
                ++inner_parens;
            } else if (tok->is(punctuator::paren_right)) {
                done = true;
                args.push_back(std::move(arg));
                arg.clear();
                break;
            } else if (tok->is(punctuator::comma) && !inner_parens) {
                args.push_back(std::move(arg));
                arg.clear();
            } else if (tok->is(token::newline)) {
                /* [6.10.3]/10
                 Within the sequence of preprocessing tokens making up an
                 invocation of a function-like macro, new-line is considered
                 a normal white-space character.
                */
                tok->kind = token::space;
                arg.push_back(*tok);
            } else {
                arg.push_back(*tok);
            }
        }
        if (!done) {
            diagnose(diagnostic::id::pp4_missing_macro_args_end, loc);
            return std::vector<token>{};
        } else if (args.size() < mac.param_names.size()) {
            const auto diff = mac.param_names.size() - args.size();
            std::string req = std::to_string(mac.param_names.size());
            if (mac.variadic) req = " at least " + req;
            diagnose(diagnostic::id::pp4_wrong_arity_macro_args, loc,
                     mac.name, req, std::to_string(args.size()));
            // recover
            for (std::size_t i = 0; i < diff; ++i) {
                args.emplace_back();
            }
        } else if (args.size() > mac.param_names.size() && !mac.variadic) {
            const auto diff = args.size() - mac.param_names.size();
            std::string req = std::to_string(mac.param_names.size());
            if (mac.variadic) req = " at least " + req;
            diagnose(diagnostic::id::pp4_wrong_arity_macro_args, loc,
                     mac.name, req, std::to_string(args.size()));
            // recover
            for (std::size_t i = 0; i < diff; ++i) {
                args.pop_back();
            }
        }
        // TODO diagnose UB for apparent directives in macro args [6.10.3]/11

        // expand each argument
        for (auto& arg : args) {
            hijack();
            tokens = std::move(arg);
            std::vector<token> expansion = macro_expand_hijacked_tokens();
            unhijack();
            arg = std::move(expansion);
        }
        // replace parameter names and handle #
        auto get_arg = [&](string_view name) -> optional<std::size_t> {
            for (std::size_t i = 0; i < mac.param_names.size(); ++i) {
                if (mac.param_names[i] == name) return i;
            }
            return {};
        };
        std::vector<token> expansion;
        for (std::size_t i = 0; i < mac.body.size(); ++i) {
            auto tok = mac.body[i];
            if (tok.is(token::identifier)) {
                if (auto index = get_arg(tok.spelling)) {
                    auto& arg = args[*index];
                    expansion.insert(expansion.end(),
                                     arg.begin(), arg.end());
                    if (arg.empty()) {
                        // TODO is it visible to the user if we do this
                        // even if this parameter isn't an argument to ##?
                        expansion.push_back(make_placemarker());
                    }
                    continue;
                } else if (tok.spelling == "__VA_ARGS__") {
                    // TODO
                    diagnose(diagnostic::id::not_yet_implemented,
                             tok.range.first, "__VA_ARGS__");
                }
            } else if (tok.is(punctuator::hash)) {
                optional<std::size_t> index;
                if (i + 1 < mac.body.size()) {
                    auto next = mac.body[i + 1];
                    if (next.is(token::identifier)) {
                        index = get_arg(next.spelling);
                    }
                }
                if (!index) {
                    diagnose(diagnostic::id::pp4_stringize_no_parameter,
                             tok.range.first);
                    continue;
                }
                auto& arg = args[*index];
                std::string data;
                for (auto tok : arg) {
                    bool needs_escape = false;
                    needs_escape |= tok.is(token::string_literal);
                    needs_escape |= tok.is(token::character_constant);
                    if (tok.is(token::space)) data += " ";
                    else if (!needs_escape) data += tok.spelling.to_string();
                    else {
                        for (char c : tok.spelling) {
                            if (c == '"') data += "\\\"";
                            else if (c == '\\') data += "\\\\";
                            else data += c;
                        }
                    }
                }
                data = util::ltrim(util::rtrim(data));
                data = "\"" + data + "\"";
                auto buf = std::make_unique<raw_buffer>("<stringized>", data);
                auto tokens = perform_phase_three(*buf);
                extra_buffers.push_back(std::move(buf));
                if (tokens.size() != 1) {
                    diagnose(diagnostic::id::pp4_stringize_invalid_token,
                             tok.range.first);
                } else {
                    expansion.push_back(tokens[0]);
                    ++i;
                    continue;
                }
            }
            expansion.push_back(tok);
        }
        // rescan
        mac.being_replaced = true;
        hijack();
        tokens = handle_concatenation(std::move(expansion));
        remove_placemarkers(tokens);
        expansion = macro_expand_hijacked_tokens();
        unhijack();
        mac.being_replaced = false;

        return expansion;
    } else {
        mac.being_replaced = true;
        hijack();
        tokens = handle_concatenation(mac.body);
        remove_placemarkers(tokens);
        std::vector<token> expansion = macro_expand_hijacked_tokens();
        unhijack();
        mac.being_replaced = false;
        return expansion;
    }
}

std::vector<token> p4m::macro_expand_hijacked_tokens() {
    std::vector<token> expansion;
    while (peek(TAKE, TAKE)) {
        auto result = maybe_expand_macro();
        if (result) {
            expansion.insert(expansion.end(),
                             result->begin(), result->end());
        } else {
            expansion.push_back(*get(TAKE, TAKE));
        }
    }
    return expansion;
}

std::vector<token> p4m::handle_concatenation(std::vector<token> in) {
    std::vector<token> result;
    hijack();
    tokens = in;
    while (peek(TAKE, TAKE)) {
        auto tok = *get(TAKE, TAKE);
        if (tok.is(punctuator::hash_hash)) {
            diagnose(diagnostic::id::pp4_cannot_use_hash_hash_here,
                     tok.range.first);
            continue;
        } else if (tok.is(token::space) || tok.is(token::newline)) {
            result.push_back(tok);
        } else {
            auto next = peek(SKIP, SKIP);
            if (next && next->is(punctuator::hash_hash)) {
                auto op = *get(SKIP, SKIP);
                next = get(SKIP, SKIP);
                if (!next) {
                    diagnose(diagnostic::id::pp4_cannot_use_hash_hash_here,
                             tok.range.first);
                    continue;
                }
                auto lhs = tok, rhs = *next;
                /* [6.10.3.3]/3
                 concatenation of two placemarkers results in a single
                 placemarker preprocessing token, and concatenation of
                 a placemarker with a non-placemarker preprocessing token
                 results in the non-placemarker preprocessing token
                */
                if (lhs.is(token::placemarker) && rhs.is(token::placemarker)) {
                    result.push_back(lhs);
                    continue;
                } else if (lhs.is(token::placemarker)) {
                    result.push_back(rhs);
                    continue;
                } else if (rhs.is(token::placemarker)) {
                    result.push_back(lhs);
                    continue;
                }
                /* [6.10.3.3]/3
                 the preceding preprocessing token is concatenated
                 with the following preprocessing token
                */
                std::string data;
                data += lhs.spelling.to_string();
                data += rhs.spelling.to_string();
                auto buf = std::make_unique<raw_buffer>("<concatenated>",
                                                        data);
                auto tokens = perform_phase_three(*buf);
                extra_buffers.push_back(std::move(buf));
                if (tokens.size() != 1) {
                    diagnose(diagnostic::id::pp4_concatenate_invalid_token,
                             op.range.first);
                    continue;
                }
                result.push_back(tokens[0]);
            } else {
                result.push_back(tok);
            }
        }
    }
    unhijack();
    return result;
}

void p4m::remove_placemarkers(std::vector<token>& v) {
    v.erase(std::remove_if(v.begin(), v.end(), [](token tok) {
        return tok.kind == token::placemarker;
    }), v.end());
}

token p4m::make_placemarker() {
    auto spelling = placemarker_buffer->data().substr(0, 1);
    location loc{*placemarker_buffer, 0};
    std::pair<location, location> range{loc, loc.next_loc()};
    return token(token::placemarker, spelling, range);
}

void p4m::hijack() {
    saved_states.push_back({ std::move(tokens), index });
    index = 0;
    tokens.clear();
}

void p4m::unhijack() {
    assert(!saved_states.empty());
    tokens = std::move(saved_states.back().tokens);
    index = saved_states.back().index;
    saved_states.pop_back();
}

void p4m::handle_null_directive() {
    assert(get(SKIP, TAKE)->is(token::newline));
}


void p4m::handle_error_directive() {
    auto error_tok = *get(SKIP, STOP);
    auto tokens = finish_line();
    std::string msg;
    std::string delim;
    for (auto tok : tokens) {
        msg += delim;
        delim = " ";
        msg += tok.spelling.to_string();
    }
    const auto loc = error_tok.range.first;
    diagnose(diagnostic::id::pp4_error_directive, loc, msg);
}

void p4m::handle_pragma_directive() {
    auto pragma_tok = *get(SKIP, STOP);
    const auto loc = pragma_tok.range.first;
    auto next = get(SKIP, STOP);
    if (next && next->spelling == "STDC") {
        diagnose(diagnostic::id::not_yet_implemented, loc, "#pragma STDC");
    } else {
        diagnostic::diagnose(diagnostic::id::pp4_unknown_pragma, loc);
    }
    (void)finish_line();
}

void p4m::handle_line_directive() {
    auto line_tok = *get(SKIP, STOP);
    const auto loc = line_tok.range.first;
    diagnose(diagnostic::id::not_yet_implemented, loc, "#line directive");
    (void)finish_line();
}

void p4m::handle_define_directive() {
    auto define_tok = *get(SKIP, STOP);
    const auto loc = define_tok.range.first;
    auto name = get(SKIP, STOP);
    if (!name || !name->is(token::identifier)) {
        diagnose(diagnostic::id::pp4_expected_macro_name, loc);
        (void)finish_line();
        return;
    }
    macro mac{name->spelling, name->range.first};
    if (peek(STOP, STOP) && peek(STOP, STOP)->is(punctuator::paren_left)) {
        (void)get(STOP, STOP);
        mac.function_like = true;
        // parse the parameter list
        bool allow_comma = false; // after a parameter other than ellipsis
        bool allow_param = true; // at the beginning or after a comma
        bool require_param = false; // after a comma
        bool done = false; // exited due to a right paren
        for (auto tok = get(SKIP, STOP); tok; tok = get(SKIP, STOP)) {
            if (tok->is(token::identifier) && allow_param) {
                mac.param_names.push_back(tok->spelling);
                allow_comma = true;
                allow_param = false;
                require_param = false;
            } else if (tok->is(punctuator::ellipsis) && allow_param) {
                mac.variadic = true;
                allow_comma = false;
                allow_param = false;
                require_param = false;
            } else if (tok->is(punctuator::comma) && allow_comma) {
                allow_comma = false;
                allow_param = true;
                require_param = true;
            } else if (tok->is(punctuator::paren_right) && !require_param) {
                done = true;
                break;
            } else {
                diagnose(diagnostic::id::pp4_unexpected_macro_param,
                         tok->range.first);
                (void)finish_line();
                return;
            }
        }
        if (!done) {
            diagnose(diagnostic::id::pp4_missing_macro_right_paren, loc);
            (void)finish_line();
            return;
        }
        // check for duplicate parameter names
        auto params = mac.param_names;
        std::stable_sort(params.begin(), params.end());
        auto it = std::adjacent_find(params.begin(), params.end());
        if (it != params.end()) {
            auto it2 = it + 1;
            location loc2(*buf, it2->begin() - buf->data().begin());
            diagnose(diagnostic::id::pp4_duplicate_macro_param, loc2, *it2);
            location loc1(*buf, it->begin() - buf->data().begin());
            diagnose(diagnostic::id::aux_previous_use, loc1);
            (void)finish_line();
            return;
        }
    }
    if (!mac.function_like) {
        if (auto tok = peek(STOP, STOP)) {
            const auto loc = tok->range.first;
            diagnose(diagnostic::id::pp4_missing_macro_space, loc);
        }
    }
    mac.body = finish_line();
    maybe_diagnose_macro_redefinition(mac);
    macros.insert({ mac.name, std::move(mac) });
}

std::vector<token> p4m::process() {
    bool allow_directive = true;
    while (index < tokens.size()) {
        auto next = *peek(SKIP, TAKE);
        if (next.is(token::newline)) {
            (void)get(SKIP, TAKE);
            allow_directive = true;
            continue;
        } else if (next.is(punctuator::hash) && allow_directive) {
            (void)get(SKIP, TAKE);
            auto id = peek(SKIP, STOP);
            if (!id) {
                handle_null_directive();
            } else if (id->spelling == "error") {
                handle_error_directive();
            } else if (id->spelling == "pragma") {
                handle_pragma_directive();
            } else if (id->spelling == "line") {
                handle_line_directive();
            } else if (id->spelling == "define") {
                handle_define_directive();
            }
        } else {
            allow_directive = false;
            auto old_index = index;
            auto invocation_start = tokens.begin() + index;
            if (auto exp = maybe_expand_macro()) {
                // TODO is it visible to the user if we only do
                // inline expansion at the top level?
                auto invocation_end = tokens.begin() + index;
                tokens.erase(invocation_start, invocation_end);
                tokens.insert(invocation_start, exp->begin(), exp->end());
                index = old_index;
            } else {
                out.push_back(*get(SKIP, TAKE));
            }
        }
    }
    return std::move(out);
}
