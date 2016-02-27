#ifndef SPCC_PP_HH
#define SPCC_PP_HH

#include "buffer.hh"

#include <memory>

namespace pp {
    std::unique_ptr<buffer> perform_phase_one(std::unique_ptr<buffer> in);
    std::unique_ptr<buffer> perform_phase_two(std::unique_ptr<buffer> in);
}

#endif
