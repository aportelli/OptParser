cmake_minimum_required(VERSION 3.0.0)
project(OptParser VERSION 1.0.0)

include(CTest)
enable_testing()

add_library(${PROJECT_NAME} INTERFACE)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_11)

add_subdirectory(tests)
