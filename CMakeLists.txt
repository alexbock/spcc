cmake_minimum_required(VERSION 3.1)
project(spcc)
include_directories(
    include
)
set(SOURCE
    src/buffer.cc
    src/main.cc
    src/options.cc
    src/system.cc
    src/utf8.cc
)
set(INCLUDE
    include/buffer.hh
    include/diagnostic.hh
    include/optional.hh
    include/options.hh
    include/string_view.hh
    include/system.hh
    include/utf8.hh
)
add_executable(spcc
    ${SOURCE}
    ${INCLUDE}
)
set_property(TARGET spcc PROPERTY CXX_STANDARD 14)
