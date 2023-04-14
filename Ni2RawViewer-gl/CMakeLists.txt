### CMakeLists for Ni2RawViewer-gl ###

cmake_minimum_required (VERSION 3.1)

set(TARGET_SAMPLE_NAME Ni2RawViewer-gl)

project(${TARGET_SAMPLE_NAME})

## Setup Building Environment
if(NOT ENVIRONMENT_INITIALIZED)
    include(${PROJECT_SOURCE_DIR}/../CMake/SetupBuildEnvironment.cmake)
    include(${PROJECT_SOURCE_DIR}/../CMake/SetupGLFW.cmake)
    build_glfw(${CMAKE_CURRENT_SOURCE_DIR}/../third-party/glfw-3.3)
    set(ENVIRONMENT_INITIALIZED 1)
endif()

## Build Taget Executable
add_executable(${TARGET_SAMPLE_NAME}
    Source/main.cpp
    ${GLFW_DEPS_INC}/glad/gl.h
    ${GLFW_DEPS_INC}/glad_gl.c
)

add_dependencies(${TARGET_SAMPLE_NAME} glfw)

target_include_directories(${TARGET_SAMPLE_NAME} PRIVATE
    ${GLFW_INC}
    ${GLFW_DEPS_INC}
)

target_link_libraries(${TARGET_SAMPLE_NAME}
    ${GLFW_LIB}
)

set_property(TARGET ${TARGET_SAMPLE_NAME} PROPERTY CXX_STANDARD 11)

set_target_properties(${TARGET_SAMPLE_NAME} PROPERTIES FOLDER "LIPSedgeSDKSamples")

## Installation
install(TARGETS ${TARGET_SAMPLE_NAME} DESTINATION .)