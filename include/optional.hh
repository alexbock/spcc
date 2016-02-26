#include <type_traits>
#include <utility>
#include <cassert>
#include <new>

namespace meta {
    template<typename T>
    class optional {
    public:
        optional() : valid{false} { }
        optional(T&& t) {
            *this = std::forward<T>(t);
        }
        optional(const optional& other) {
            if (other.valid) *this = other.value();
            else valid = false;
        }
        optional(optional&& other) {
            if (other.valid) {
                other.valid = false;
                *this = std::move(other.value());
            } else valid = false;
        }
        optional& operator=(const optional& other) {
            if (!other.valid) valid = false;
            else *this = other.value();
        }
        optional& operator=(optional&& other) {
            if (!other.valid) valid = false;
            else {
                other.valid = false;
                *this = std::move(other.value());
            }
        }
        optional& operator=(T&& t) {
            if (valid) {
                valid = false;
                value() = std::forward<T>(t);
                valid = true;
            } else {
                new (&data) T(std::forward<T>(t));
                valid = true;
            }
        }
        ~optional() {
            maybe_destroy();
        }

        T& value() {
            assert(valid);
            return reinterpret_cast<T&>(data);
        }
        const T& value() const {
            assert(valid);
            return reinterpret_cast<const T&>(data);
        }
        operator bool() const { return valid; }
        T* operator->() { return &value(); }
        const T* operator->() const { return &value(); }
        T& operator*() { return value(); }
        const T& operator*() const { return value(); }
    private:
        void maybe_destroy() {
            if (valid) {
                valid = false;
                value().~T();
            }
        }

        std::aligned_union_t<0, T> data;
        bool valid;
    };
}

using meta::optional;
