#include "decl_spec.hh"
#include "parser.hh"
#include "options.hh"
#include "diagnostic.hh"

#include <algorithm>
#include <cassert>

using diagnostic::diagnose;

#define SIMPLE_TYPE_SPECIFIER_CASE(x) \
case kw_##x: \
ds.simple_type_specifiers.push_back(sts_##x); \
p.next(); \
continue;
#define STORAGE_CLASS_CASE(x) \
case kw_##x: \
ds.storage_classes.push_back(sc_##x); \
p.next(); \
continue;
#define TYPE_QUALIFIER_CASE(x) \
case kw_##x: \
ds.type_qualifiers.push_back(tq_##x); \
p.next(); \
continue;
#define FUNCTION_SPECIFIER_CASE(x) \
case kw_##x: \
ds.function_specifiers.push_back(fs_##x); \
p.next(); \
continue;

namespace parse {
    decl_spec parse_decl_spec(parser& p) {
        decl_spec ds;
        ds.p = &p;
        for (;;) {
            auto tok = p.peek();
            ds.loc_start = tok.range.first;
            if (tok.is(token::keyword)) {
                switch (tok.kw) {
                    SIMPLE_TYPE_SPECIFIER_CASE(void)
                    SIMPLE_TYPE_SPECIFIER_CASE(char)
                    SIMPLE_TYPE_SPECIFIER_CASE(short)
                    SIMPLE_TYPE_SPECIFIER_CASE(int)
                    SIMPLE_TYPE_SPECIFIER_CASE(long)
                    SIMPLE_TYPE_SPECIFIER_CASE(float)
                    SIMPLE_TYPE_SPECIFIER_CASE(double)
                    SIMPLE_TYPE_SPECIFIER_CASE(signed)
                    SIMPLE_TYPE_SPECIFIER_CASE(unsigned)
                    SIMPLE_TYPE_SPECIFIER_CASE(Bool)
                    SIMPLE_TYPE_SPECIFIER_CASE(Complex)
                    STORAGE_CLASS_CASE(typedef)
                    STORAGE_CLASS_CASE(extern)
                    STORAGE_CLASS_CASE(static)
                    STORAGE_CLASS_CASE(Thread_local)
                    STORAGE_CLASS_CASE(auto)
                    STORAGE_CLASS_CASE(register)
                    TYPE_QUALIFIER_CASE(const)
                    TYPE_QUALIFIER_CASE(restrict)
                    TYPE_QUALIFIER_CASE(volatile)
                    TYPE_QUALIFIER_CASE(Atomic)
                    FUNCTION_SPECIFIER_CASE(inline)
                    FUNCTION_SPECIFIER_CASE(Noreturn)

                    case kw_Alignas: {
                        p.next();
                        auto lparen = p.next();
                        if (!lparen.is(punctuator::paren_left)) {
                            diagnose(diagnostic::id::pp7_expected_token,
                                     lparen.range.first, "(");
                            p.rewind(); // this might help recovery
                        }
                        alignment_specifier as;
                        if (p.could_be_expr_ahead()) {
                            p.push_ruleset(false);
                            as.expr = p.parse(0);
                            p.pop_ruleset();
                        } else {
                            assert(false); // TODO parse type name
                        }
                        auto rparen = p.next();
                        if (!rparen.is(punctuator::paren_right)) {
                            diagnose(diagnostic::id::pp7_expected_token,
                                     rparen.range.first, ")");
                            p.rewind();
                        }
                        ds.alignment_specifiers.push_back(std::move(as));
                        continue;
                    }
                    case kw_struct:
                    case kw_union:
                    case kw_enum:
                        assert(false); // TODO
                        break;
                    default:
                        break;
                }
            } else if (tok.is(token::identifier)) {
                if (p.is_typedef_name(tok.spelling)) {
                    auto ty = p.get_typedef_type(tok.spelling);
                    ds.direct_type_specifiers.push_back(ty);
                    p.next();
                    continue;
                }
            }
            break;
        }
        return ds;
    }

    template<typename T>
    static bool match(const std::set<T>& a,
                      const std::set<T>& b) {
        return std::equal(a.begin(), a.end(), b.begin());
    }

    const type* decl_spec::build_unqualified_type() {
        if (direct_type_specifiers.empty()) {
            auto long_kind = ik_long;
            int delta = 0;
            if (std::count(simple_type_specifiers.begin(),
            simple_type_specifiers.end(), sts_long) == 2) {
                long_kind = ik_long_long;
                delta = 1;
            }
            std::set<simple_type_specifier> specs;
            specs.insert(simple_type_specifiers.begin(),
                         simple_type_specifiers.end());
            if (specs.size() - delta < simple_type_specifiers.size()) {
                diagnose(diagnostic::id::pp7_invalid_decl_spec_type,
                         loc_start);
            }
            if (match(specs, { sts_void })) {
                return p->tm.get_void_type();
            } else if (match(specs, { sts_char })) {
                bool char_signed = options::state.is_char_signed;
                return p->tm.get_integer_type(ik_char, char_signed);
            } else if (match(specs, { sts_signed, sts_char })) {
                return p->tm.get_integer_type(ik_char, true);
            } else if (match(specs, { sts_unsigned, sts_char })) {
                return p->tm.get_integer_type(ik_char, false);
            } else if (match(specs, { sts_int })) {
                return p->tm.get_integer_type(ik_int, true);
            } else if (match(specs, { sts_int, sts_signed })) {
                return p->tm.get_integer_type(ik_int, true);
            } else if (match(specs, { sts_int, sts_unsigned })) {
                return p->tm.get_integer_type(ik_int, false);
            } else if (match(specs, { sts_signed })) {
                return p->tm.get_integer_type(ik_int, true);
            } else if (match(specs, { sts_unsigned })) {
                return p->tm.get_integer_type(ik_int, false);
            } else if (match(specs, { sts_short })) {
                return p->tm.get_integer_type(ik_short, true);
            } else if (match(specs, { sts_short, sts_signed })) {
                return p->tm.get_integer_type(ik_short, true);
            } else if (match(specs, { sts_short, sts_unsigned })) {
                return p->tm.get_integer_type(ik_short, false);
            } else if (match(specs, { sts_short, sts_int })) {
                return p->tm.get_integer_type(ik_short, true);
            } else if (match(specs, { sts_short, sts_int, sts_signed })) {
                return p->tm.get_integer_type(ik_short, true);
            } else if (match(specs, { sts_short, sts_int, sts_unsigned })) {
                return p->tm.get_integer_type(ik_short, false);
            } else if (match(specs, { sts_long })) {
                return p->tm.get_integer_type(long_kind, true);
            } else if (match(specs, { sts_long, sts_signed })) {
                return p->tm.get_integer_type(long_kind, true);
            } else if (match(specs, { sts_long, sts_unsigned })) {
                return p->tm.get_integer_type(long_kind, false);
            } else if (match(specs, { sts_long, sts_int })) {
                return p->tm.get_integer_type(long_kind, true);
            } else if (match(specs, { sts_long, sts_int, sts_signed })) {
                return p->tm.get_integer_type(long_kind, true);
            } else if (match(specs, { sts_long, sts_int, sts_unsigned })) {
                return p->tm.get_integer_type(long_kind, false);
            } else if (match(specs, { sts_float })) {
                return p->tm.get_real_floating_type(fk_float);
            } else if (match(specs, { sts_double })) {
                return p->tm.get_real_floating_type(fk_double);
            } else if (match(specs, { sts_float, sts_Complex })) {
                return p->tm.get_complex_floating_type(fk_float);
            } else if (match(specs, { sts_double, sts_Complex })) {
                return p->tm.get_complex_floating_type(fk_double);
            } else if (match(specs, { sts_double, sts_long })) {
                diagnose(diagnostic::id::pp7_invalid_decl_spec_type,
                         loc_start);
                return p->tm.get_real_floating_type(fk_long_double);
            } else if (match(specs, { sts_double, sts_long, sts_Complex })) {
                diagnose(diagnostic::id::pp7_invalid_decl_spec_type,
                         loc_start);
                return p->tm.get_complex_floating_type(fk_long_double);
            } else if (match(specs, { sts_Bool })) {
                return p->tm.get_integer_type(ik_bool, false);
            } else {
                diagnose(diagnostic::id::pp7_invalid_decl_spec_type,
                         loc_start);
                return p->tm.get_integer_type(ik_int, true); // recover
            }
        } else {
            if (direct_type_specifiers.size() != 1 ||
            !simple_type_specifiers.empty()) {
                diagnose(diagnostic::id::pp7_invalid_decl_spec_type,
                         loc_start);
            }
            return direct_type_specifiers[0];
        }
    }
}
