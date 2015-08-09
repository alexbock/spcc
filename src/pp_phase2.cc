#include "pp.hh"
#include "buffer.hh"
#include "translator.hh"
#include "diagnostic.hh"
#include "util.hh"

buffer perform_pp_phase2(buffer& src) {
    buffer dst{src.name + "#2", ""};
    dst.src = std::make_unique<translator>(src, dst);
    auto& t = *dst.src;
    /* [5.1.1.2]/1.2
    Each instance of a backslash character (\) immediately followed
    by a new-line character is deleted, splicing physical source lines
    to form logical source lines. Only the last backslash on any physical
    source line shall be eligible for being part of such a splice. A
    source file that is not empty shall end in a new-line character,
    which shall not be immediately preceded by a backslash character
    before any such splicing takes place.
    */
    while (!t.done()) {
        // delete each instance of a backslash character (\)
        // immediately followed by a new-line character
        if (t.peek(2) == "\\\n") {
            t.erase(2);
        } else {
            t.propagate(1);
        }
    }
    // a source file that is not empty shall end in a new-line character,
    // which shall not be immediately preceded by a backslash character
    // before any such splicing takes place
    if (!src.data.empty() && !ends_with(dst.data, "\n")) {
        location loc{dst, dst.data.size()};
        if (ends_with(src.data, "\\\n")) {
            // if the file ended with a splice, point at the backslash
            // instead of the empty line
            loc = { src, src.data.size() - 2 };
        }
        diagnose(diagnostic_id::pp_phase2_missing_newline, loc);
        t.insert("\n");
    }
    return dst;
}
