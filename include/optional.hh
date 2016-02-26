#ifndef SPCC_OPTIONAL_HH
#define SPCC_OPTIONAL_HH

#include <type_traits>
#include <utility>
#include <cassert>
#include <new>

namespace meta {
    template<typename T>
    class optional {
    public:
        optional() : valid{false} { }
        optional(const T& t) {
            operator=(t);
        }
        optional(T&& t) {
            operator=(std::move(t));
        }
        optional(const optional& other) {
            if (other.valid) operator=(other.value());
            else valid = false;
        }
        optional(optional&& other) {
            if (other.valid) {
                other.valid = false;
                operator=(std::move(other.value()));
            } else valid = false;
        }
        optional& operator=(const optional& other) {
            if (!other.valid) valid = false;
            else operator=(other.value());
            return *this;
        }
        optional& operator=(optional&& other) {
            if (!other.valid) valid = false;
            else {
                other.valid = false;
                operator=(std::move(other.value()));
            }
            return *this;
        }
        optional& operator=(const T& t) {
            if (valid) {
                valid = false;
                reinterpret_cast<T&>(data) = t;
                valid = true;
            } else {
                new (&data) T(t);
                valid = true;
            }
            return *this;
        }
        optional& operator=(T&& t) {
            if (valid) {
                valid = false;
                reinterpret_cast<T&>(data) = std::move(t);
                valid = true;
            } else {
                new (&data) T(std::move(t));
                valid = true;
            }
            return *this;
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
                reinterpret_cast<T&>(data).~T();
            }
        }

        std::aligned_union_t<0, T> data;
        bool valid;
    };
}

using meta::optional;

#endif
