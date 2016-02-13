#include "pp.hh"
#include "buffer.hh"
#include "translator.hh"
#include "utf8.hh"
#include "diagnostic.hh"

#include <map>

std::map<std::string, std::string> trigraphs = {
    { R"(??=)", "#" },
    { R"(??()", "[" },
    { R"(??/)", "\\" },
    { R"(??))", "]" },
    { R"(??')", "^" },
    { R"(??<)", "{" },
    { R"(??!)", "|" },
    { R"(??>)", "}" },
    { R"(??-)", "~" }
};

buffer perform_pp_phase1(buffer& src) {
    buffer dst{src.name + "#1", ""};
    dst.src = std::make_unique<translator>(src, dst);
    auto& t = *dst.src;
    /* [5.1.1.2]/1.1
    Physical source file multibyte characters are mapped, in an
    implementation-defined manner, to the source character set
    (introducing new-line characters for end-of-line indicators)
    if necessary. Trigraph sequences are replaced by corresponding
    single-character internal representations.
    */
    while (!t.done()) {
        // map physical source file multibyte characters to the
        // source character set
        if (!utf8::is_ascii(t.peek())) try {
            std::string cp = t.peek(4);
            std::uint32_t utf32 = utf8::code_point_to_utf32(cp);
            std::string ucn = utf8::utf32_to_ucn(utf32);
            auto cp_length = utf8::measure_code_point(cp);
            t.replace(cp_length, ucn);
            continue;
        } catch (utf8::invalid_utf8_error error) {
            location loc{src, t.src_index};
            diagnose(diagnostic_id::pp_phase1_invalid_utf8,
                     loc,
                     error.what());
            t.replace(1, "\\u001A"); // U+001A "SUBSTITUTE"
        }
        // introduce new-line characters for end-of-line indicators
        if (t.peek(2) == "\r\n") {
            t.replace(2, "\n");
            continue;
        }
        // replace trigraph sequences with corresponding single-character
        // internal representations
        auto trigraph = trigraphs.find(t.peek(3));
        if (trigraph != trigraphs.end()) {
            t.replace(3, trigraph->second);
            continue;
        }

        t.propagate(1);
    }
    return dst;
}
