#ifndef SPCC_DIAGNOSTIC_HH
#define SPCC_DIAGNOSTIC_HH

namespace diagnostic {
    enum class category {
        error,
        warning,
        auxiliary,
        undefined_behavior,
    };
}

#endif
