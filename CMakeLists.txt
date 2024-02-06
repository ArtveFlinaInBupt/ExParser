cmake_minimum_required(VERSION 3.26)
project(ExParser)

set(CMAKE_CXX_STANDARD 20)

set(SRC_DIR src)

include_directories(${SRC_DIR})

add_compile_options("-W")
add_compile_options("-Wall")
add_compile_options("-Wextra")
add_compile_options("-Wpedantic")

add_executable(ExParser
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/parser/grammar.cpp
    ${SRC_DIR}/parser/parser.cpp
    ${SRC_DIR}/simple_lexer/lexer.cpp
)
