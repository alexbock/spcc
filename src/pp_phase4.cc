#include "pp.hh"
#include "pp_token.hh"
#include "diagnostic.hh"

#include <algorithm>
#include <string>
#include <set>

using namespace lex_behavior;

class lexer {
public:
    lexer(std::vector<pp_token>& tokens) : tokens(tokens) { }
    pp_token* next(mode spaces, mode newlines) {
        std::size_t index_delta;
        auto tok = ::peek(tokens, 0, spaces, newlines,
                          false, &index_delta, index);
        if (tok) index = index_delta;
        return tok;
    }
    pp_token* peek(std::size_t offset, mode spaces, mode newlines) {
        return ::peek(tokens, offset, spaces, newlines,
                      false, nullptr, index);
    }
    std::vector<pp_token*> eat_to_end_of_line() {
        std::vector<pp_token*> results;
        while (index < tokens.size()) {
            auto tok = next(INCLUDE, INCLUDE);
            if (tok->spelling == "\n") break;
            if (tok->spelling != " ") results.push_back(tok);
        }
        return results;
    }

    bool at_end_of_line() {
        auto next = peek(0, SKIP, INCLUDE);
        return !next || next->spelling == "\n";
    }
private:
    std::vector<pp_token>& tokens;
    std::size_t index = 0;
};

bool is_specific_punctuator(pp_token* tok, punctuator_kind kind) {
    auto punc = tok->maybe_as<pp_token::punctuator>();
    return (punc && punc->kind == kind);
}

bool is_specific_identifier(pp_token* tok, const std::string& spelling) {
    return tok->kind == pp_token_kind::identifier && tok->spelling == spelling;
}

void handle_error_directive(lexer& lex) {
    auto error_token = lex.next(SKIP, STOP);
    auto tokens = lex.eat_to_end_of_line();
    std::string message;
    std::string delim;
    for (auto token : tokens) {
        message += delim;
        delim = " ";
        message += token->spelling;
    }
    auto loc = error_token->range.first;
    diagnose(diagnostic_id::pp_phase4_error_directive, loc, message);
}

void handle_pragma_directive(lexer& lex) {
    auto pragma_token = lex.next(SKIP, STOP);
    auto loc = pragma_token->range.first;
    auto next = lex.peek(0, SKIP, STOP);
    if (next && next->spelling == "STDC") {
        (void)lex.next(SKIP, STOP);
        next = lex.next(SKIP, STOP);
        static std::set<std::string> stdc_pragmas = {
            "FP_CONTRACT", "FENV_ACCESS", "CX_LIMITED_RANGE"
        };
        if (next && stdc_pragmas.find(next->spelling) != stdc_pragmas.end()) {
            diagnose(diagnostic_id::not_yet_implemented, loc, "STDC pragmas");
        } else {
            diagnose(diagnostic_id::pp_phase4_invalid_stdc_pragma, loc);
        }
    } else {
        auto name = next ? next->spelling : "<no name>";
        diagnose(diagnostic_id::pp_phase4_pragma_ignored, loc, name);
    }
    (void)lex.eat_to_end_of_line();
}

void handle_line_directive(lexer& lex) {
    auto line_token = lex.next(SKIP, STOP);
    auto loc = line_token->range.first;
    diagnose(diagnostic_id::not_yet_implemented, loc, "line directives");
    (void)lex.eat_to_end_of_line();
}

void handle_null_directive(lexer& lex) {
    (void)lex.eat_to_end_of_line();
}

void include_header(lexer& lex, pp_token::header_name* hn, location loc) {
    // TODO
    diagnose(diagnostic_id::pp_phase4_cannot_open_header, loc, hn->name);
}

void handle_include_directive(lexer& lex) {
    auto include_token = lex.next(SKIP, STOP);
    auto loc = include_token->range.first;
    auto next = lex.peek(0, SKIP, STOP);
    if (!next) {
        diagnose(diagnostic_id::pp_phase4_expected_header_name, loc);
        (void)lex.eat_to_end_of_line();
        return;
    }
    if (auto hn = next->maybe_as<pp_token::header_name>()) {
        lex.next(SKIP, STOP);
        include_header(lex, hn, loc);
        if (!lex.at_end_of_line()) {
            diagnose(diagnostic_id::pp_phase4_extra_tokens_after_header, loc);
        }
        (void)lex.eat_to_end_of_line();
    } else {
        diagnose(diagnostic_id::not_yet_implemented, loc,
                 "macro expansion for include directives");
        (void)lex.eat_to_end_of_line();
    }
}

std::vector<pp_token> perform_pp_phase4(std::vector<pp_token>& tokens) {
    /* [5.1.1.2]/1.4
    Preprocessing directives are executed, macro invocations are
    expanded, and _Pragma unary operator expressions are executed.
    If a character sequence that matches the syntax of a universal
    character name is produced by token concatenation (6.10.3.3),
    the behavior is undefined. A #include preprocessing directive
    causes the named header or source file to be processed from
    phase 1 through phase 4, recursively. All preprocessing directives
    are then deleted.
    */
    lexer lex(tokens);
    std::vector<pp_token> output;
    while (auto tok = lex.next(SKIP, SKIP)) {
        if (is_specific_punctuator(tok, punctuator_kind::hash)) {
            auto next = lex.peek(0, SKIP, STOP);
            if (next && is_specific_identifier(next, "error")) {
                handle_error_directive(lex);
            } else if (next && is_specific_identifier(next, "pragma")) {
                handle_pragma_directive(lex);
            } else if (next && is_specific_identifier(next, "line")) {
                handle_line_directive(lex);
            } else if (next && is_specific_identifier(next, "include")) {
                handle_include_directive(lex);
            } else if (!next && lex.peek(0, SKIP, INCLUDE)) {
                handle_null_directive(lex);
            } else {
                // TODO non-directive
            }
        }
    }
    return output;
}
