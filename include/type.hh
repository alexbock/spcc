#ifndef SPCC_TYPE_HH
#define SPCC_TYPE_HH

#include "util.hh"

#include <unordered_map>
#include <memory>

namespace sem {
    enum qualifier {
        qual_const    = 1 << 0,
        qual_volatile = 1 << 1,
        qual_restrict = 1 << 2,
    };

    enum type_kind {
        tk_void,
        tk_integer,
        tk_real_floating,
        tk_complex_floating,
        tk_pointer,
        tk_function,
        tk_structure,
        tk_union,
        tk_atomic,
        tk_array,
        tk_enum,
    };

    enum integer_kind {
        ik_bool,
        ik_char,
        ik_int,
        ik_short,
        ik_long,
        ik_long_long,
    };

    enum floating_kind {
        fk_float,
        fk_double,
        fk_long_double,
    };

    class type {
    public:
        type(enum type_kind kind) : kind{kind} { }
        type(integer_kind int_kind, bool is_signed) :
        kind{tk_integer}, int_kind{int_kind}, is_signed{is_signed} { }

        bool is_const() const { return quals & qual_const; }
        bool is_volatile() const { return quals & qual_volatile; }
        bool is_restrict() const { return quals & qual_restrict; }

        bool is_basic_type() const;
        bool is_arithmetic_type() const;
        bool is_real_type() const;
        bool is_character_type() const;
        bool is_integer_type() const;
        bool is_real_floating_type() const;
        bool is_pointer_type() const;
        bool is_scalar_type() const;
        bool is_aggregate_type() const;
        bool is_any_floating_type() const;

        bool is_specific_integer_kind(integer_kind) const;
        bool is_bool_type() const;
        bool is_object_type() const;
        bool is_complete_object_type() const;
        bool is_incomplete_object_type() const;

        enum type_kind type_kind() const { return kind; }
    private:
        friend class type_manager;

        qualifier quals;
        enum type_kind kind;

        union {
            struct { // tk_integer
                integer_kind int_kind;
                bool is_signed;
            };
            floating_kind float_kind; // tk_*_floating
            const type* pointee_type; // tk_pointer
            struct { // tk_function
                // TODO
            };
            struct {  // tk_array
                // TODO
            };
        };
    };

    class type_manager {
    public:
        type_manager();

        const type* build_pointer_to(const type* ty);
        const type* get_integer_type(integer_kind kind, bool is_signed) const;
        const type* get_real_floating_type(floating_kind kind) const;
        const type* get_complex_floating_type(floating_kind kind) const;
        const type* get_void_type() const;
    private:
        void initialize_integer_types();
        void initialize_floating_types();
        void register_integer_type(integer_kind kind, bool is_signed);
        void register_floating_type(floating_kind kind, bool is_real);

        using type_uptr = std::unique_ptr<type>;

        type_uptr void_type;
        std::unordered_map<integer_kind, type_uptr> signed_integer_types;
        std::unordered_map<integer_kind, type_uptr> unsigned_integer_types;
        std::unordered_map<floating_kind, type_uptr> real_floating_types;
        std::unordered_map<floating_kind, type_uptr> complex_floating_types;
        std::unordered_map<const type*, type_uptr> pointer_types;
    };
}

#endif
