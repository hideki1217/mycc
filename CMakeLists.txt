cmake_minimum_required(VERSION 3.22.1)

project(mycc)

enable_testing()

add_subdirectory(collection)
add_subdirectory(tests)

add_library(mycc_lib STATIC
    mycc.h
    define.h
    error.c
    tokenize.c
    util.c
    path.c
    pp.c
)
target_link_libraries(mycc_lib collection_lib)
target_include_directories(mycc_lib PRIVATE collection/)

add_executable(mycc
    main.c
)
target_link_libraries(mycc mycc_lib)

