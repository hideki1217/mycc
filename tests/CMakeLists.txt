
add_executable(util_test
    util_test.c
)
target_link_libraries(util_test
    util_lib    
)
target_include_directories(util_test PRIVATE ../)


add_test(NAME util_test COMMAND util_test)
