cmake_minimum_required (VERSION 3.1)
project(LIPSedge_SDK_Samples LANGUAGES CXX C)

## Setup Building Environment
include(CMake/SetupBuildEnvironment.cmake)
include(CMake/SetupGLFW.cmake)
build_glfw(${CMAKE_CURRENT_SOURCE_DIR}/third-party/glfw-3.3)
set(ENVIRONMENT_INITIALIZED 1)

# Add our samples
set(DIR_LISTS "")

file(GLOB LIPS_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/LIPS*)
foreach(DIR ${LIPS_DIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DIR})
        list(APPEND DIR_LISTS ${DIR})
    endif()
endforeach()

file(GLOB NI2_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Ni2*)
foreach(DIR ${NI2_DIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DIR})
        list(APPEND DIR_LISTS ${DIR})
    endif()
endforeach()

foreach(DIR ${DIR_LISTS})
    add_subdirectory(${DIR})
endforeach()
