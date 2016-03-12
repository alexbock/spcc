#ifndef SPCC_TYPE_HH
#define SPCC_TYPE_HH

#include "util.hh"

namespace sem {
    enum qualifier {
        qual_const    = 1 << 0,
        qual_volatile = 1 << 1,
        qual_restrict = 1 << 2,
    };

    enum type_kind {
        tk_object,
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
        bool is_const() const { return quals & qual_const; }
        bool is_volatile() const { return quals & qual_volatile; }
        bool is_restrict() const { return quals & qual_restrict; }

        bool is_basic_type() const;
        bool is_arithmetic_type() const;
        bool is_real_type() const;
        bool is_character_type() const;
        bool is_scalar_type() const;
        bool is_aggregate_type() const;
        bool is_any_floating_type() const;

        bool is_specific_integer_kind(integer_kind) const;
        bool is_bool_type() const;

        enum type_kind type_kind() const { return kind; }
    private:
        qualifier quals;
        enum type_kind kind;
    };

    class object_type : public type {
    public:
        static bool is_type_of(const type* ty) {
            return ty->type_kind() != tk_function;
        }

        virtual bool is_complete() const = 0;
    private:
    };

    class void_type : public object_type {
    public:
        static bool is_type_of(const type* ty) {
            return ty->type_kind() == tk_void;
        }

        bool is_complete() const override { return false; }
    private:
    };

    class integer_type : public object_type {
    public:
        static bool is_type_of(const type* ty) {
            return ty->type_kind() == tk_integer;
        }

        bool is_complete() const override { return true; }
        integer_kind int_kind() const { return kind; }
        bool is_signed() const { return !is_unsigned; }
    private:
        integer_kind kind;
        bool is_unsigned;
    };

    class real_floating_type : public object_type {
    public:
        static bool is_type_of(const type* ty) {
            return ty->type_kind() == tk_real_floating;
        }

        floating_kind float_kind() const { return kind; }
        bool is_complete() const override { return true; }
    private:
        floating_kind kind;
    };

    class complex_floating_type : public object_type {
    public:
        static bool is_type_of(const type* ty) {
            return ty->type_kind() == tk_complex_floating;
        }

        floating_kind float_kind() const { return kind; }
        bool is_complete() const override { return true; }
    private:
        floating_kind kind;
    };

    class pointer_type : public object_type {
    public:
        static bool is_type_of(const type* ty) {
            return ty->type_kind() == tk_pointer;
        }

        bool is_complete() const override { return true; }
    private:
        type* pointee;
    };

    class array_type : public object_type {
    public:
        static bool is_type_of(const type* ty) {
            return ty->type_kind() == tk_array;
        }

        bool is_complete() const override;
    private:
    };

    class atomic_type : public object_type {
    public:
        static bool is_type_of(const type* ty) {
            return ty->type_kind() == tk_atomic;
        }

        bool is_complete() const override { return true; }
    private:
    };

    class structure_type : public object_type {
    public:
        static bool is_type_of(const type* ty) {
            return ty->type_kind() == tk_structure;
        }

        bool is_complete() const override { return true; }
    private:
    };

    class union_type : public object_type {
    public:
        static bool is_type_of(const type* ty) {
            return ty->type_kind() == tk_union;
        }

        bool is_complete() const override { return true; }
    private:
    };

    class function_type : public type {
    public:
        static bool is_type_of(const type* ty) {
            return ty->type_kind() == tk_function;
        }
    private:
    };
}

#endif
