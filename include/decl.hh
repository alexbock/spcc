#ifndef SPCC_DECL_HH
#define SPCC_DECL_HH

#include "string_view.hh"
#include "token.hh"
#include "buffer.hh"

#include <vector>
#include <memory>
#include <cstddef>

namespace sem {
    class type;

    class decl {
    public:
        string_view name() const { return identifier.spelling; }
        location loc() const { return identifier.range.first; }
        void dump() const;
        virtual ~decl() = 0;
    protected:
        decl(token identifier) : identifier{identifier} { }
    private:
        token identifier;
    };

    class var_decl : public decl {
    public:
    private:
    };

    class field_decl : public var_decl {
    public:
    private:
    };

    class struct_decl : public decl {
    public:
    private:
    };

    class union_decl : public decl {
    public:
    private:
    };

    class typedef_decl : public decl {
    public:
        const type* underlying_type() const { return underlying; }
    private:
        const type* underlying;
    };
}

#endif
