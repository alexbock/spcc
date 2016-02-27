#ifndef SPCC_PP_HH
#define SPCC_PP_HH

#include "buffer.hh"
#include "token.hh"

#include <memory>
#include <regex>

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
}

#endif
