#ifndef SPCC_PP_HH
#define SPCC_PP_HH

#include "buffer.hh"
#include "token.hh"

#include <memory>
#include <regex>
#include <utility>
#include <map>
#include <optional>

namespace pp {
    using buffer_ptrs = std::vector<std::unique_ptr<buffer>>;

    std::unique_ptr<buffer> perform_phase_one(std::unique_ptr<buffer> in);
    std::unique_ptr<buffer> perform_phase_two(std::unique_ptr<buffer> in);
    std::vector<token> perform_phase_three(const buffer& in);
    std::vector<token> perform_phase_six(std::vector<token> tokens,
                                         buffer_ptrs& extra);
    std::vector<token> perform_phase_seven(const std::vector<token>& tokens);
    std::optional<token> convert_pp_token_to_token(token tok);
    void remove_whitespace(std::vector<token>& tokens);

    struct string_literal_info {
        enum encoding {
            plain,
            utf8,
            wchar,
            char16,
            char32,
        };
        std::string_view body;
        enum encoding encoding;
    };
    using string_literal_encoding = enum string_literal_info::encoding;
    string_literal_info analyze_string_literal(const token& tok);
    std::string to_string(string_literal_encoding enc);

    namespace regex {
        extern const std::regex header_name;
        extern const std::regex pp_number;
        extern const std::regex identifier;
        extern const std::regex string_literal;
        extern const std::regex char_constant;
        extern const std::regex punctuator;
        extern const std::regex space;
        extern const std::regex newline;
        extern const std::regex integer_constant;
    }

    class lexer {
    public:
        lexer(const buffer& buf) : buf(buf) { }
        std::optional<token> try_lex(token_kind kind, const std::regex& regex);
        bool done() const;
        std::string_view peek() const;
        std::size_t index() const { return index_; }
        void select(token tok);

        std::vector<token> tokens;
    private:
        bool allow_header_name() const;

        const buffer& buf;
        std::size_t index_ = 0;
    };

    struct macro {
        std::string_view name;
        location loc;
        std::vector<token> body;
        bool predefined = false;
        bool being_replaced = false;

        std::vector<std::string_view> param_names;
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
            add_predefined_macros();
        }

        std::vector<token> process(bool in_arg = false);
    private:
        enum ws_mode {
            SKIP,
            STOP,
            TAKE,
        };

        std::optional<std::size_t> find(ws_mode space, ws_mode newline);
        std::optional<token> peek(ws_mode space, ws_mode newline);
        std::optional<token> get(ws_mode space, ws_mode newline);
        std::vector<token> finish_line();
        void finish_directive_line(token name);

        void handle_null_directive();
        void handle_error_directive();
        void handle_pragma_directive();
        void handle_line_directive();
        void handle_define_directive();
        void handle_undef_directive();
        void handle_include_directive();
        void handle_ifdef_directive();
        void handle_ifndef_directive();
        void handle_else_directive();
        void handle_endif_directive();
        void handle_non_directive();

        void maybe_diagnose_macro_redefinition(const macro& def) const;
        std::optional<std::vector<token>> maybe_expand_macro();
        std::vector<token> handle_concatenation(std::vector<token> in);
        void remove_placemarkers(std::vector<token>& v);
        token make_placemarker();
        void hijack();
        void unhijack();
        void add_predefined_macros();
        void make_predefined_macro(std::string name, std::string body);
        token make_file_token(token at);
        token make_line_token(token at);
        bool in_disabled_region() const;
        void handle_ifdef_ifndef(token tok, bool is_ifndef);

        std::unique_ptr<buffer> buf;
        std::vector<token> tokens;
        std::vector<token> out;
        std::map<std::string_view, macro> macros;
        std::vector<std::unique_ptr<buffer>> extra_buffers;
        std::unique_ptr<raw_buffer> placemarker_buffer;
        std::size_t index = 0;
        std::vector<bool> cond_states;
        std::size_t include_level = 0;

        struct saved_state {
            std::vector<token> tokens;
            std::vector<token> out;
            std::size_t index;
        };
        std::vector<saved_state> saved_states;
    };
}

#endif
