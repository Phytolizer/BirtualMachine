cmake_minimum_required(VERSION 3.16...3.23)

project(
    "Birtual Machine"
    LANGUAGES CXX
    VERSION "0.1.0"
    DESCRIPTION "A [BV]irtual Machine"
    HOMEPAGE_URL "https://github.com/Phytolizer/BirtualMachine"
)

set(CMAKE_CXX_STANDARD 20)

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME "fmt"
    GITHUB_REPOSITORY "fmtlib/fmt"
    GIT_TAG "8.1.1"
)

CPMAddPackage(
    NAME "magic_enum"
    GITHUB_REPOSITORY "Neargye/magic_enum"
    GIT_TAG "v0.7.3"
)

add_executable(bm bm.cpp)
target_link_libraries(bm PRIVATE fmt::fmt magic_enum::magic_enum)
target_compile_features(bm PRIVATE cxx_std_20)
