cmake_minimum_required(VERSION 3.22.1)

project(mycc)

enable_testing()

add_library(mycc_lib STATIC
    mycc.h
    define.h
    error.c
    tokenize.c
    util.c
)

add_executable(mycc
    main.c
)
target_link_libraries(mycc mycc_lib)

add_subdirectory(tests)
