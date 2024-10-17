#include "pp.hh"

#include <cassert>
#include <map>
#include <string>

namespace pp { namespace regex {
    std::map<std::string, std::string> table = {
        { "header-name", R"(<.*?>|".*?")" },
        { "pp-number", R"(\.?[0-9]([0-9]|[epEP][\+-]|@id-nondigit@|\.)*)" },
        { "identifier", "@id-nondigit@(@id-nondigit@|[0-9])*" },
        { "string-literal", R"((u8|u|U|L)?"@sc-char@*?")" },
        { "char-constant", R"((u|U|L)?'@sc-char@*?')" },
        { "punc-4", R"(%:%:)" },
        { "punc-3", R"(\.\.\.|<<=|>>=)" },
        { "punc-2-1", R"(\+\+|--|<<|>>|<=|>=|==|!=|&&|\|\|)" },
        { "punc-2-2", R"(\*=|\\=|%=|\+=|-=|&=|\^=|\|=|##|->)" },
        { "punc-2-3", R"(<:|:>|<%|%>|%:)" },
        { "punc-2", "@punc-2-1@|@punc-2-2@|@punc-2-3@" },
        { "punc-1-1", R"(\[|\]|\(|\)|\{|\}|\.)" },
        { "punc-1-2", R"(&|\*|\+|-|~|!)" },
        { "punc-1-3", R"(/|%|<|>|\^|\|)" },
        { "punc-1-4", R"(\?|:|;|=|,|#)" },
        { "punc-1", "@punc-1-1@|@punc-1-2@|@punc-1-3@|@punc-1-4@" },
        { "punctuator", "@punc-4@|@punc-3@|@punc-2@|@punc-1@" },
        { "line-comment", R"(//.*\n)" },
        { "multiline-comment", R"(\/\*[\s\S]*?(\*\/|$))" },
        { "comment", "@line-comment@|@multiline-comment@" },
        { "space", R"((@comment@| +))" },
        { "newline", R"(\n)" },
        // TODO floating constants
        { "integer-constant-any-base", "@decimal-con@|@octal-con@|@hex-con@" },
        { "integer-constant", "@integer-constant-any-base@@int-suffix@?" },
        { "int-suffix","(u|U)(l|L)?|(u|U)(ll|LL)|(ll|LL)(u|U)?|(ll|LL)(u|U)?"},
        { "decimal-con", "[1-9][0-9]*" },
        { "octal-con", "0[0-7]*" },
        { "hex-con", "(0x|0X)@hex-digit@+" },
        { "hex-digit", "[0-9a-fA-F]" },
        { "ucn", R"(\\u@hex-digit@{4}|\\U@hex-digit@{8})" },
        { "id-nondigit", "[a-zA-Z_]|@ucn@" },
        { "escape-seq", "@simple-escape@|@octal-escape@|@hex-escape@|@ucn@" },
        { "simple-escape", R"(\\('|"|\?|\\|a|b|f|n|r|t|v))" },
        { "octal-escape", R"(\\[0-7]{1,3})" },
        { "hex-escape", R"(\\x@hex-digit@+)" },
        { "sc-char", R"(@escape-seq@|[^\\\n])" },
    };

    std::string resolve_references(std::string pattern) {
        auto it = pattern.find('@');
        if (it == std::string::npos) return pattern;
        auto end = pattern.find('@', it + 1);
        assert(end != std::string::npos);
        auto prefix = pattern.substr(0, it);
        auto repl = table[pattern.substr(it + 1, end - (it + 1))];
        auto suffix = pattern.substr(end + 1);
        return resolve_references(prefix + "(?:" + repl + ")" + suffix);
    }

    std::regex build(std::string name) {
        auto it = table.find(name);
        assert(it != table.end());
        return std::regex{
            resolve_references(it->second),
            std::regex::ECMAScript | std::regex::optimize
        };
    }

    const std::regex header_name = build("header-name");
    const std::regex pp_number = build("pp-number");
    const std::regex identifier = build("identifier");
    const std::regex string_literal = build("string-literal");
    const std::regex char_constant = build("char-constant");
    const std::regex punctuator = build("punctuator");
    const std::regex space = build("space");
    const std::regex newline = build("newline");
    const std::regex integer_constant = build("integer-constant");
} }
