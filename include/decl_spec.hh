#ifndef SPCC_DECL_SPEC_HH
#define SPCC_DECL_SPEC_HH

#include "token.hh"
#include "type.hh"
#include "buffer.hh"
#include "optional.hh"

#include <memory>
#include <vector>

namespace parse {
    using namespace sem;

    class node;
    class parser;

    enum storage_class {
        sc_typedef,
        sc_extern,
        sc_static,
        sc_Thread_local,
        sc_auto,
        sc_register,
    };

    enum type_qualifier {
        tq_const,
        tq_restrict,
        tq_volatile,
        tq_Atomic,
    };

    enum function_specifier {
        fs_inline,
        fs_Noreturn,
    };

    struct alignment_specifier {
        const type* ty = nullptr;
        std::unique_ptr<node> expr = nullptr;
    };

    enum simple_type_specifier {
        sts_void,
        sts_char,
        sts_short,
        sts_int,
        sts_long,
        sts_float,
        sts_double,
        sts_signed,
        sts_unsigned,
        sts_Bool,
        sts_Complex,
    };

    struct decl_spec {
        optional<location> loc_start;
        parser* p = nullptr;
        std::vector<storage_class> storage_classes;
        std::vector<type_qualifier> type_qualifiers;
        std::vector<function_specifier> function_specifiers;
        std::vector<alignment_specifier> alignment_specifiers;
        std::vector<simple_type_specifier> simple_type_specifiers;
        std::vector<const type*> direct_type_specifiers;

        const type* build_unqualified_type();
    };

    decl_spec parse_decl_spec(parser&);
}

#endif
