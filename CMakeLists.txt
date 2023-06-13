cmake_minimum_required(VERSION 3.0.0)
project(opengl VERSION 0.1.0)

set(GLFW3_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/glfw-3.3.8/include)
set(GLW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/glm)
set(GLFW_LIB ${CMAKE_SOURCE_DIR}/glfw3.lib)
set(GLAD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/glad/include)
set(ASSIMP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/assimp-5.2.5/include)
set(ASSIMP_LIB ${CMAKE_SOURCE_DIR}/assimp-vc142-mt.lib)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})

include_directories(
    ${GLFW3_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
    ${GLW_INCLUDE_DIR}
    ${ASSIMP_INCLUDE_DIR}
)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADERS ${CMAKE_SOURCE_DIR}/src/*.h)

add_executable(
    ${PROJECT_NAME}
    ${HEADERS}
    ${SOURCES}
    ${GLAD_INCLUDE_DIR}/../src/glad.c
)

target_link_libraries(
    ${PROJECT_NAME}
    ${GLFW_LIB}
    ${ASSIMP_LIB}
)