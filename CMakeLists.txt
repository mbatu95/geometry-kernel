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

add_executable(vector_example examples/vector_example.cpp)
target_link_libraries(vector_example PRIVATE geometry::geometry_kernel)

add_executable(matrix_example examples/matrix_example.cpp)
target_link_libraries(matrix_example PRIVATE geometry::geometry_kernel)

add_executable(transform_example examples/transform_example.cpp)
target_link_libraries(transform_example PRIVATE geometry::geometry_kernel)

add_executable(ray_plane_example examples/ray_plane_example.cpp)
target_link_libraries(ray_plane_example PRIVATE geometry::geometry_kernel)

add_executable(ray_triangle_example examples/ray_triangle_example.cpp)
target_link_libraries(ray_triangle_example PRIVATE geometry::geometry_kernel)

if(BUILD_TESTING)
    add_executable(geometry_kernel_tests tests/test_vectors.cpp)
    target_link_libraries(geometry_kernel_tests PRIVATE geometry::geometry_kernel)
    add_test(NAME geometry_kernel_tests COMMAND geometry_kernel_tests)

    add_executable(geometry_kernel_matrix_tests tests/test_matrices.cpp)
    target_link_libraries(geometry_kernel_matrix_tests PRIVATE geometry::geometry_kernel)
    add_test(NAME geometry_kernel_matrix_tests COMMAND geometry_kernel_matrix_tests)

    add_executable(geometry_kernel_transform_tests tests/test_transforms.cpp)
    target_link_libraries(geometry_kernel_transform_tests PRIVATE geometry::geometry_kernel)
    add_test(NAME geometry_kernel_transform_tests COMMAND geometry_kernel_transform_tests)

    add_executable(geometry_kernel_ray_plane_tests tests/test_ray_plane.cpp)
    target_link_libraries(geometry_kernel_ray_plane_tests PRIVATE geometry::geometry_kernel)
    add_test(NAME geometry_kernel_ray_plane_tests COMMAND geometry_kernel_ray_plane_tests)

    add_executable(geometry_kernel_triangle_tests tests/test_triangle.cpp)
    target_link_libraries(geometry_kernel_triangle_tests PRIVATE geometry::geometry_kernel)
    add_test(NAME geometry_kernel_triangle_tests COMMAND geometry_kernel_triangle_tests)
endif()