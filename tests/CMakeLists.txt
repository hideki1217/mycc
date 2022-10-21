
add_executable(test_util
    test_util.c
)
target_link_libraries(test_util
    util_lib    
)
target_include_directories(test_util PRIVATE ../)

add_executable(test_tokenize
    ../tokenize.c
    ../error.c
    test_tokenize.c
)
target_link_libraries(test_tokenize
    util_lib    
)
target_include_directories(test_tokenize PRIVATE ../)


add_test(NAME test_util COMMAND test_util)
add_test(NAME test_tokenize COMMAND test_tokenize)
