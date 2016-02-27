#include "pp.hh"
#include "utf8.hh"
#include "diagnostic.hh"
#include "util.hh"

#include <map>

using diagnostic::diagnose;
using util::ends_with;

static std::map<std::string, std::string> trigraphs = {
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
                diagnose(diagnostic::id::invalid_utf8, loc);
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
