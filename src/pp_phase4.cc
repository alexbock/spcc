#include "pp.hh"
#include "pp_token.hh"
#include "diagnostic.hh"
#include "buffer.hh"

#include <algorithm>
#include <string>
#include <set>
#include <fstream>
#include <sstream>
#include <map>
#include <stack>

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
    std::vector<pp_token*> eat_to_end_of_line(bool include_spaces = false) {
        std::vector<pp_token*> results;
        while (index < tokens.size()) {
            auto tok = next(INCLUDE, INCLUDE);
            if (tok->spelling == "\n") break;
            if (tok->spelling != " " || include_spaces) results.push_back(tok);
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

void include_header(lexer& lex, pp_token::header_name* hn, location loc,
                    std::vector<buffer_ptr>& storage,
                    std::vector<pp_token>& output) {
    std::ifstream file{hn->name};
    if (!file.good()) {
        diagnose(diagnostic_id::pp_phase4_cannot_open_header, loc, hn->name);
        return;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    buffer_ptr buf = std::make_unique<buffer>(hn->name, ss.str());
    buf->included_at = loc;
    ss.str({});
    auto p1 = perform_pp_phase1(*buf);
    auto p2 = perform_pp_phase2(*p1);
    auto p3 = perform_pp_phase3(*p2);
    auto p4 = perform_pp_phase4(p3);
    storage.push_back(std::move(buf));
    storage.push_back(std::move(p1));
    storage.push_back(std::move(p2));
    output.insert(output.end(),
                  std::make_move_iterator(p4.begin()),
                  std::make_move_iterator(p4.end()));
}

void handle_include_directive(lexer& lex, std::vector<buffer_ptr>& storage,
                              std::vector<pp_token>& output) {
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
        include_header(lex, hn, loc, storage, output);
        if (!lex.at_end_of_line()) {
            diagnose(diagnostic_id::pp_phase4_extra_after_directive, loc,
                     "include");
        }
        (void)lex.eat_to_end_of_line();
    } else {
        diagnose(diagnostic_id::not_yet_implemented, loc,
                 "macro expansion for include directives");
        (void)lex.eat_to_end_of_line();
    }
}

struct macro {
    std::string name;
    std::vector<pp_token*> body;
    std::vector<std::string> parameter_names;
    bool is_function_like = false;
    bool is_variadic = false;
};

void handle_define_directive(lexer& lex,
                             std::map<std::string, macro>& macros) {
    auto define_token = lex.next(SKIP, STOP);
    auto loc = define_token->range.first;
    auto macro_name = lex.next(SKIP, STOP);
    if (!macro_name || macro_name->kind != pp_token_kind::identifier) {
        diagnose(diagnostic_id::pp_phase4_missing_macro_name, loc);
        (void)lex.eat_to_end_of_line();
        return;
    }
    macro m;
    m.name = macro_name->spelling;
    // handle the parameter list if this is a function-like macro
    auto next = lex.peek(0, STOP, STOP);
    if (next && is_specific_punctuator(next, punctuator_kind::paren_left)) {
        m.is_function_like = true;
        lex.next(STOP, STOP);
        bool found_rparen = false;
        bool another_parameter = false;
        bool first_parameter = true;
        while ((next = lex.next(SKIP, STOP))) {
            if (is_specific_punctuator(next, punctuator_kind::paren_right)) {
                if (another_parameter) {
                    diagnose(diagnostic_id::pp_phase4_missing_flm_parameter,
                             next->range.first);
                    (void)lex.eat_to_end_of_line();
                    return;
                }
                found_rparen = true;
                break;
            }
            if (!another_parameter && !first_parameter) {
                diagnose(diagnostic_id::pp_phase4_flm_expected_comma_or_paren,
                         next->range.first);
                (void)lex.eat_to_end_of_line();
                return;
            }
            first_parameter = false;
            another_parameter = false;

            if (is_specific_punctuator(next, punctuator_kind::ellipsis)) {
                m.is_variadic = true;
            } else if (next->kind == pp_token_kind::identifier) {
                m.parameter_names.push_back(next->spelling);
            } else {
                diagnose(diagnostic_id::pp_phase4_invalid_flm_param,
                         next->range.first);
                (void)lex.eat_to_end_of_line();
                return;
            }

            next = lex.peek(0, SKIP, STOP);
            if (next && is_specific_punctuator(next, punctuator_kind::comma)) {
                if (m.is_variadic) {
                    diagnose(diagnostic_id::pp_phase4_flm_comma_after_ellipsis,
                             next->range.first);
                    (void)lex.eat_to_end_of_line();
                    return;
                }
                another_parameter = true;
                lex.next(SKIP, STOP);
            }
        }
        if (!found_rparen) {
            diagnose(diagnostic_id::pp_phase4_missing_end_of_macro_param_list,
                     loc);
            (void)lex.eat_to_end_of_line();
            return;
        }
    }
    // capture the replacement list without leading or trailing whitespace
    for (;;) {
        auto next = lex.peek(0, INCLUDE, STOP);
        if (next && next->spelling == " ") lex.next(INCLUDE, STOP);
        else break;
    }
    m.body = lex.eat_to_end_of_line(true);
    while (!m.body.empty() && m.body.back()->spelling == " ") {
        m.body.pop_back();
    }
    // check if this is an invalid redefinition
    auto it = macros.find(m.name);
    if (it != macros.end()) {
        /* [6.10.3]/2
        An identifier currently defined as an object-like macro shall not
        be redefined by another #define preprocessing directive unless the
        second definition is an object-like macro definition and the two
        replacement lists are identical. Likewise, an identifier currently
        defined as a function-like macro shall not be redefined by another
        #define preprocessing directive unless the second definition is a
        function-like macro definition that has the same number and spelling
        of parameters, and the two replacement lists are identical.
        */
        const auto& old = it->second;
        bool invalid = false;
        // both macros must be of the same type
        invalid |= m.is_function_like != old.is_function_like;
        // and have identical replacement lists
        invalid |= m.body.size() != old.body.size();
        if (!invalid) {
            for (std::size_t i = 0; i < m.body.size(); ++i) {
                invalid |= m.body[i]->spelling != old.body[i]->spelling;
            }
        }
        // function-like macros must have identical parameter lists
        invalid |= m.parameter_names.size() != old.parameter_names.size();
        if (!invalid && m.is_function_like) {
            for (std::size_t i = 0; i < m.parameter_names.size(); ++i) {
                invalid |= m.parameter_names[i] != old.parameter_names[i];
            }
        }

        if (invalid) {
            diagnose(diagnostic_id::pp_phase4_invalid_macro_redef, loc);
            return;
        }
    }

    macros[m.name] = std::move(m);
}

void handle_undef_directive(lexer& lex, std::map<std::string, macro>& macros) {
    auto undef_token = lex.next(SKIP, STOP);
    auto loc = undef_token->range.first;
    auto next = lex.next(SKIP, STOP);
    if (!next || next->kind != pp_token_kind::identifier) {
        diagnose(diagnostic_id::pp_phase4_undef_missing_ident, loc);
        (void)lex.eat_to_end_of_line();
        return;
    }
    macros.erase(next->spelling);
    if (!lex.at_end_of_line()) {
        diagnose(diagnostic_id::pp_phase4_extra_after_directive, loc, "undef");
        (void)lex.eat_to_end_of_line();
        return;
    }
    (void)lex.eat_to_end_of_line();
}

bool is_active(const std::vector<bool>& activation) {
    for (bool b : activation) if (!b) return false;
    return true;
}

void handle_ifdef_directive(lexer& lex, std::map<std::string, macro>& macros,
                            std::vector<bool>& activation) {
    auto ifdef_token = lex.next(SKIP, STOP);
    auto loc = ifdef_token->range.first;
    auto next = lex.next(SKIP, STOP);
    if (!next || next->kind != pp_token_kind::identifier) {
        diagnose(diagnostic_id::pp_phase4_ifdef_missing_arg, loc);
        (void)lex.eat_to_end_of_line();
        return;
    }

    if (is_active(activation)) {
        activation.push_back(macros.find(next->spelling) != macros.end());
    } else {
        activation.push_back(false);
    }

    if (!lex.at_end_of_line()) {
        diagnose(diagnostic_id::pp_phase4_extra_after_directive, loc, "ifdef");
        (void)lex.eat_to_end_of_line();
        return;
    }
    (void)lex.eat_to_end_of_line();
}

void handle_ifndef_directive(lexer& lex, std::map<std::string, macro>& macros,
                             std::vector<bool>& activation) {
    auto ifndef_token = lex.next(SKIP, STOP);
    auto loc = ifndef_token->range.first;
    diagnose(diagnostic_id::not_yet_implemented, loc, "ifndef directives");
    (void)lex.eat_to_end_of_line();
    // TODO
}

void handle_else_directive(lexer& lex, std::vector<bool>& activation) {
    auto else_token = lex.next(SKIP, STOP);
    auto loc = else_token->range.first;
    if (activation.empty()) {
        diagnose(diagnostic_id::pp_phase4_invalid_non_directive, loc, "else");
        (void)lex.eat_to_end_of_line();
        return;
    }
    bool old = activation.back();
    activation.pop_back();
    if (is_active(activation)) {
        activation.push_back(!old);
    } else {
        activation.push_back(false);
    }
    if (!lex.at_end_of_line()) {
        diagnose(diagnostic_id::pp_phase4_extra_after_directive, loc, "else");
        (void)lex.eat_to_end_of_line();
        return;
    }
}

void handle_endif_directive(lexer& lex, std::vector<bool>& activation) {
    auto endif_token = lex.next(SKIP, STOP);
    auto loc = endif_token->range.first;
    if (activation.empty()) {
        diagnose(diagnostic_id::pp_phase4_invalid_non_directive, loc, "endif");
        (void)lex.eat_to_end_of_line();
        return;
    }
    activation.pop_back();
    if (!lex.at_end_of_line()) {
        diagnose(diagnostic_id::pp_phase4_extra_after_directive, loc, "endif");
        (void)lex.eat_to_end_of_line();
        return;
    }
}

void handle_non_directive(lexer& lex) {
    auto token = lex.peek(0, SKIP, STOP);
    diagnose(diagnostic_id::pp_phase4_non_directive, token->range.first);
    (void)lex.eat_to_end_of_line();
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
    std::vector<buffer_ptr> storage;
    std::map<std::string, macro> macros;
    std::vector<bool> activation;
    while (auto tok = lex.next(SKIP, SKIP)) {
        if (is_specific_punctuator(tok, punctuator_kind::hash)) {
            auto next = lex.peek(0, SKIP, STOP);

            if (!is_active(activation)) {
                bool pass = false;
                if (next) {
                    pass |= is_specific_identifier(next, "if"); // TODO
                    pass |= is_specific_identifier(next, "ifdef");
                    pass |= is_specific_identifier(next, "ifndef");
                    pass |= is_specific_identifier(next, "else");
                    pass |= is_specific_identifier(next, "elif"); // TODO
                    pass |= is_specific_identifier(next, "endif");
                }
                if (!pass) {
                    (void)lex.eat_to_end_of_line();
                    continue;
                }
            }

            if (next && is_specific_identifier(next, "error")) {
                handle_error_directive(lex);
            } else if (next && is_specific_identifier(next, "pragma")) {
                handle_pragma_directive(lex);
            } else if (next && is_specific_identifier(next, "line")) {
                handle_line_directive(lex);
            } else if (next && is_specific_identifier(next, "include")) {
                handle_include_directive(lex, storage, output);
            } else if (next && is_specific_identifier(next, "define")) {
                handle_define_directive(lex, macros);
            } else if (next && is_specific_identifier(next, "undef")) {
                handle_undef_directive(lex, macros);
            } else if (next && is_specific_identifier(next, "ifdef")) {
                handle_ifdef_directive(lex, macros, activation);
            } else if (next && is_specific_identifier(next, "ifndef")) {
                handle_ifndef_directive(lex, macros, activation);
            } else if (next && is_specific_identifier(next, "else")) {
                handle_else_directive(lex, activation);
            } else if (next && is_specific_identifier(next, "endif")) {
                handle_endif_directive(lex, activation);
            } else if (next) {
                handle_non_directive(lex);
            } else if (!next && lex.peek(0, SKIP, INCLUDE)) {
                handle_null_directive(lex);
            }
        }
    }
    return output;
}
