add_executable(print-opt print-opt.cpp)
target_include_directories(print-opt PRIVATE "${PROJECT_SOURCE_DIR}")

add_test(NAME print-opt COMMAND print-opt)