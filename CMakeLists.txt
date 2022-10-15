cmake_minimum_required(VERSION 3.22.1)

project(mycc)

enable_testing()

add_library(util_lib STATIC util.c)

add_executable(mycc
    mycc.h
    define.h
    tokenize.c
)
target_link_libraries(mycc util_lib)

add_subdirectory(tests)
