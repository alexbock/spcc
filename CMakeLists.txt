cmake_minimum_required(VERSION 3.1)
project(spcc)
include_directories(
    include
)
set(SOURCE

)
set(INCLUDE

)
add_executable(spcc
    ${SOURCE}
    ${INCLUDE}
)
set_property(TARGET spcc PROPERTY CXX_STANDARD 14)
