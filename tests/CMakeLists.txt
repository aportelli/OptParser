add_executable(print-opt print-opt.cpp)
target_link_libraries(print-opt OptParser)

add_test(NAME print-opt COMMAND print-opt)
