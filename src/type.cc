#include "type.hh"

#include <cassert>
#include <utility>

namespace sem {
    bool type::is_basic_type() const {
        switch (type_kind()) {
            case tk_integer:
            case tk_real_floating:
            case tk_complex_floating:
                return true;
            default:
                return false;
        }
    }

    bool type::is_arithmetic_type() const {
        return is_integer_type() || is_any_floating_type();
    }

    bool type::is_real_type() const {
        return is_integer_type() || is_real_floating_type();
    }

    bool type::is_character_type() const {
        return is_specific_integer_kind(ik_char);
    }

    bool type::is_integer_type() const {
        switch (type_kind()) {
            case tk_integer:
            case tk_enum:
                return true;
            default:
                return false;
        }
    }

    bool type::is_real_floating_type() const {
        return type_kind() == tk_real_floating;
    }

    bool type::is_pointer_type() const {
        return type_kind() == tk_pointer;
    }

    bool type::is_scalar_type() const {
        return is_arithmetic_type() || is_pointer_type();
    }

    bool type::is_aggregate_type() const {
        switch (type_kind()) {
            case tk_array:
            case tk_structure:
                return true;
            default:
                return false;
        }
    }

    bool type::is_any_floating_type() const {
        switch (type_kind()) {
            case tk_real_floating:
            case tk_complex_floating:
                return true;
            default:
                return false;
        }
    }

    bool type::is_specific_integer_kind(integer_kind ik) const {
        if (type_kind() != tk_integer) return false;
        return int_kind == ik;
    }

    bool type::is_bool_type() const {
        return is_specific_integer_kind(ik_bool);
    }

    bool type::is_object_type() const {
        return type_kind() != tk_function;
    }

    bool type::is_complete_object_type() const {
        switch (type_kind()) {
            case tk_function:
            case tk_void:
                return false;
            case tk_structure:
            case tk_union:
            case tk_enum:
            case tk_array:
                assert(false); // TODO
            default:
                return true;
        }
    }

    bool type::is_incomplete_object_type() const {
        return is_object_type() && !is_complete_object_type();
    }

    using tm = type_manager;

    type_manager::type_manager() {
        // create standard types
        void_type = std::make_unique<type>(tk_void);
        initialize_integer_types();
        initialize_floating_types();
    }

    const type* tm::build_pointer_to(const type* ty) {
        auto it = pointer_types.find(ty);
        if (it != pointer_types.end()) return it->second.get();
        auto ptr_ty = std::make_unique<type>(tk_pointer);
        ptr_ty->pointee_type = ty;
        pointer_types[ty] = std::move(ptr_ty);
        return pointer_types[ty].get();
    }

    void tm::initialize_integer_types() {
        register_integer_type(integer_kind::ik_bool, false);
        register_integer_type(integer_kind::ik_char, false);
        register_integer_type(integer_kind::ik_int, false);
        register_integer_type(integer_kind::ik_long, false);
        register_integer_type(integer_kind::ik_long_long, false);
        register_integer_type(integer_kind::ik_short, false);

        register_integer_type(integer_kind::ik_char, true);
        register_integer_type(integer_kind::ik_int, true);
        register_integer_type(integer_kind::ik_long, true);
        register_integer_type(integer_kind::ik_long_long, true);
        register_integer_type(integer_kind::ik_short, true);
    }

    void tm::initialize_floating_types() {
        register_floating_type(fk_float, true);
        register_floating_type(fk_double, true);
        register_floating_type(fk_long_double, true);

        register_floating_type(fk_float, false);
        register_floating_type(fk_double, false);
        register_floating_type(fk_long_double, false);
    }

    void tm::register_integer_type(integer_kind kind,
                                   bool is_signed) {
        auto& map = is_signed ? signed_integer_types : unsigned_integer_types;
        map[kind] = std::make_unique<type>(kind, is_signed);
    }

    void tm::register_floating_type(floating_kind kind,
                                    bool is_real) {
        auto& map = is_real ? real_floating_types : complex_floating_types;
        auto type_kind = is_real ? tk_real_floating : tk_complex_floating;
        auto ty = std::make_unique<type>(type_kind);
        ty->float_kind = kind;
        map[kind] = std::move(ty);
    }

    const type* tm::get_integer_type(integer_kind kind,
                                     bool is_signed) const {
        assert(!(kind == ik_bool && is_signed));
        auto& map = is_signed ? signed_integer_types : unsigned_integer_types;
        return map.find(kind)->second.get();
    }

    const type* tm::get_real_floating_type(floating_kind kind) const {
        return real_floating_types.find(kind)->second.get();
    }

    const type* tm::get_complex_floating_type(floating_kind kind) const {
        return complex_floating_types.find(kind)->second.get();
    }

    const type* tm::get_void_type() const {
        return void_type.get();
    }
}
