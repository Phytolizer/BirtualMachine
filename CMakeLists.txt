cmake_minimum_required(VERSION 3.16...3.23)

project(
    "Birtual Machine"
    LANGUAGES CXX
    VERSION "0.1.0"
    DESCRIPTION "A [BV]irtual Machine"
    HOMEPAGE_URL "https://github.com/Phytolizer/BirtualMachine"
)

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)

find_package(fmt)
if(NOT fmt_FOUND)
    FetchContent_Declare(
        fmt
        GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
        GIT_TAG "8.1.1"
    )
    FetchContent_MakeAvailable(fmt)
endif()

FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY "https://github.com/Neargye/magic_enum.git"
    GIT_TAG "v0.7.3"
)
FetchContent_MakeAvailable(magic_enum)

add_executable(bm bm.cpp)
target_link_libraries(bm PRIVATE fmt::fmt magic_enum::magic_enum)
target_compile_features(bm PRIVATE cxx_std_20)
