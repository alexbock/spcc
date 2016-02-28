#ifndef SPCC_PP_HH
#define SPCC_PP_HH

#include "buffer.hh"
#include "token.hh"
#include "optional.hh"

#include <memory>
#include <regex>
#include <utility>
#include <map>

namespace pp {
    std::unique_ptr<buffer> perform_phase_one(std::unique_ptr<buffer> in);
    std::unique_ptr<buffer> perform_phase_two(std::unique_ptr<buffer> in);
    std::vector<token> perform_phase_three(const buffer& in);

    namespace regex {
        extern const std::regex header_name;
        extern const std::regex pp_number;
        extern const std::regex identifier;
        extern const std::regex string_literal;
        extern const std::regex char_constant;
        extern const std::regex punctuator;
        extern const std::regex space;
        extern const std::regex newline;
    }

    class lexer {
    public:
        lexer(const buffer& buf) : buf(buf) { }
        optional<token> try_lex(token_kind kind, const std::regex& regex);
        bool done() const;
        string_view peek() const;
        std::size_t index() const { return index_; }
        void select(token tok);

        std::vector<token> tokens;
    private:
        bool allow_header_name() const;

        const buffer& buf;
        std::size_t index_ = 0;
    };

    struct macro {
        string_view name;
        location loc;
        std::vector<token> body;
        bool being_replaced = false;

        std::vector<string_view> param_names;
        bool function_like = false;
        bool variadic = false;
    };

    class phase_four_manager {
    public:
        phase_four_manager(std::unique_ptr<buffer> buf,
                           std::vector<token> tokens) :
        buf(std::move(buf)), tokens(std::move(tokens)) {
            placemarker_buffer = std::make_unique<raw_buffer>("<placemarker>",
                                                              "$\n");
        }

        std::vector<token> process(bool in_arg = false);
    private:
        enum ws_mode {
            SKIP,
            STOP,
            TAKE,
        };

        optional<std::size_t> find(ws_mode space, ws_mode newline);
        optional<token> peek(ws_mode space, ws_mode newline);
        optional<token> get(ws_mode space, ws_mode newline);
        std::vector<token> finish_line();
        void finish_directive_line(token name);

        void handle_null_directive();
        void handle_error_directive();
        void handle_pragma_directive();
        void handle_line_directive();
        void handle_define_directive();
        void handle_undef_directive();

        void maybe_diagnose_macro_redefinition(const macro& def) const;
        optional<std::vector<token>> maybe_expand_macro();
        std::vector<token> handle_concatenation(std::vector<token> in);
        void remove_placemarkers(std::vector<token>& v);
        token make_placemarker();
        void hijack();
        void unhijack();

        std::unique_ptr<buffer> buf;
        std::vector<token> tokens;
        std::vector<token> out;
        std::map<string_view, macro> macros;
        std::vector<std::unique_ptr<buffer>> extra_buffers;
        std::unique_ptr<raw_buffer> placemarker_buffer;
        std::size_t index = 0;

        struct saved_state {
            std::vector<token> tokens;
            std::vector<token> out;
            std::size_t index;
        };
        std::vector<saved_state> saved_states;
    };
}

#endif
