cmake_minimum_required(VERSION 3.10)

project(ProjectOpenGL)

set(CMAKE_CXX_STANDARD 17)

# Ensure vcpkg paths are correctly set
set(CMAKE_TOOLCHAIN_FILE "C:/Users/andra/source/repos/ProjectOpenGL/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Set CMAKE_PREFIX_PATH explicitly
set(CMAKE_PREFIX_PATH "C:/Users/andra/source/repos/ProjectOpenGL/vcpkg/installed/x64-windows")

# Find OpenGL and required libraries
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

# Include necessary directories
include_directories("C:/Users/andra/Documents/an3/gp/glm-1.0.1")

# Define the executable
add_executable(ProjectOpenGL 
    ProjectOpenGL/main.cpp
    ProjectOpenGL/Shader.cpp
    ProjectOpenGL/Model3D.cpp
    ProjectOpenGL/Camera.cpp
    ProjectOpenGL/SkyBox.cpp
    ProjectOpenGL/Rain.cpp
    ProjectOpenGL/stb_image.cpp
    ProjectOpenGL/tiny_obj_loader.cpp
)

# **Link Libraries Correctly**
target_link_libraries(ProjectOpenGL
    PRIVATE GLEW::GLEW  
    PRIVATE OpenGL::GL 
    PRIVATE glfw
)

# Define GLEW_STATIC
add_compile_definitions(GLEW_STATIC)
target_link_libraries(ProjectOpenGL PRIVATE GLEW::GLEW OpenGL::GL glfw)
