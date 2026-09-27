cmake_minimum_required(VERSION 3.20)

project(geometry-kernel
    VERSION 0.1.0
    LANGUAGES CXX
)

add_library(geometry_kernel INTERFACE)
add_library(geometry::geometry_kernel ALIAS geometry_kernel)

target_compile_features(geometry_kernel INTERFACE cxx_std_20)
target_include_directories(geometry_kernel INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(CTest)

add_executable(matrix_example examples/vector_example.cpp)
target_link_libraries(matrix_example PRIVATE geometry::geometry_kernel)

if(BUILD_TESTING)
    add_executable(geometry_kernel_tests tests/test_vectors.cpp)
    target_link_libraries(geometry_kernel_tests PRIVATE geometry::geometry_kernel)
    add_test(NAME geometry_kernel_tests COMMAND geometry_kernel_tests)

    add_executable(geometry_kernel_matrix_tests tests/test_matrices.cpp)
    target_link_libraries(geometry_kernel_matrix_tests PRIVATE geometry::geometry_kernel)
    add_test(NAME geometry_kernel_matrix_tests COMMAND geometry_kernel_matrix_tests)
endif()