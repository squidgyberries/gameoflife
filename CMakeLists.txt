cmake_minimum_required(VERSION 3.20)
project(gameoflife VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)

set(OpenGL_GL_PREFERENCE GLVND)
add_subdirectory(raylib)

add_executable(${PROJECT_NAME} src/main.c)

target_link_libraries(${PROJECT_NAME} raylib)