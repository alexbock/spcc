#include "type.hh"

#include <cassert>

namespace sem {
    bool type::is_basic_type() const {
        return is<integer_type>(this) || is<real_floating_type>(this);
    }

    bool type::is_arithmetic_type() const {
        return is<integer_type>(this) || is_any_floating_type();
    }

    bool type::is_real_type() const {
        return is<integer_type>(this) || is<real_floating_type>(this);
    }

    bool type::is_character_type() const {
        return is_specific_integer_kind(ik_char);
    }

    bool type::is_scalar_type() const {
        return is_arithmetic_type() || is<pointer_type>(this);
    }

    bool type::is_aggregate_type() const {
        return is<array_type>(this) || is<structure_type>(this);
    }

    bool type::is_any_floating_type() const {
        return is<real_floating_type>(this) || is<complex_floating_type>(this);
    }

    bool type::is_specific_integer_kind(integer_kind ik) const {
        if (auto x = virtual_cast<integer_type>(this)) {
            return x->int_kind() == ik;
        } else return false;
    }

    bool type::is_bool_type() const {
        return is_specific_integer_kind(ik_bool);
    }
}
