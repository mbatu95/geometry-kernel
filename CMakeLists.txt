cmake_minimum_required(VERSION 3.20)

project(geometry-kernel
    VERSION 0.1.0
    LANGUAGES CXX
)

option(GEOMETRY_KERNEL_BUILD_VIEWER "Build the OpenGL geometry viewer" ON)

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

add_executable(ray_aabb_example examples/ray_aabb_example.cpp)
target_link_libraries(ray_aabb_example PRIVATE geometry::geometry_kernel)

add_executable(mesh_example examples/mesh_example.cpp)
target_link_libraries(mesh_example PRIVATE geometry::geometry_kernel)

add_executable(primitives_example examples/primitives_example.cpp)
target_link_libraries(primitives_example PRIVATE geometry::geometry_kernel)

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

    add_executable(geometry_kernel_aabb_tests tests/test_aabb.cpp)
    target_link_libraries(geometry_kernel_aabb_tests PRIVATE geometry::geometry_kernel)
    add_test(NAME geometry_kernel_aabb_tests COMMAND geometry_kernel_aabb_tests)

    add_executable(geometry_kernel_mesh_tests tests/test_mesh.cpp)
    target_link_libraries(geometry_kernel_mesh_tests PRIVATE geometry::geometry_kernel)
    add_test(NAME geometry_kernel_mesh_tests COMMAND geometry_kernel_mesh_tests)

    add_executable(geometry_kernel_primitive_tests tests/test_primitives.cpp)
    target_link_libraries(geometry_kernel_primitive_tests PRIVATE geometry::geometry_kernel)
    add_test(NAME geometry_kernel_primitive_tests COMMAND geometry_kernel_primitive_tests)
endif()

if(GEOMETRY_KERNEL_BUILD_VIEWER)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 3.3 CONFIG REQUIRED)

    if(TARGET glfw)
        set(GEOMETRY_KERNEL_GLFW_TARGET glfw)
    elseif(TARGET glfw3::glfw)
        set(GEOMETRY_KERNEL_GLFW_TARGET glfw3::glfw)
    else()
        message(FATAL_ERROR "glfw3 was found but did not define a supported CMake target")
    endif()

    add_executable(geometry_viewer
        viewer/main.cpp
        viewer/camera.cpp
        viewer/renderer.cpp
        viewer/shader.cpp
    )
    target_include_directories(geometry_viewer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(geometry_viewer PRIVATE
        geometry::geometry_kernel
        ${GEOMETRY_KERNEL_GLFW_TARGET}
        OpenGL::GL
    )
    target_compile_definitions(geometry_viewer PRIVATE
        GEOMETRY_VIEWER_SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/viewer/shaders"
    )
    if(APPLE)
        target_compile_definitions(geometry_viewer PRIVATE GL_SILENCE_DEPRECATION)
    endif()

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/viewer/shaders")
    configure_file(viewer/shaders/basic.vert
        "${CMAKE_CURRENT_BINARY_DIR}/viewer/shaders/basic.vert" COPYONLY)
    configure_file(viewer/shaders/basic.frag
        "${CMAKE_CURRENT_BINARY_DIR}/viewer/shaders/basic.frag" COPYONLY)
endif()